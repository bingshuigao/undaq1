#include "ebd_recv.h"
#include "my_tcp_clt.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <unistd.h>

ebd_recv::ebd_recv()
{
	sock_buf = NULL;
	sock = -1;
	thread_id = 1;
	init_fun.push_back(&ebd_recv_init);
}

ebd_recv::~ebd_recv()
{
	if (sock_buf)
		delete sock_buf;
}

int ebd_recv::ebd_recv_init(my_thread* This, initzer* the_initzer)
{
	ebd_recv* ptr = reinterpret_cast<ebd_recv*>(This);

	ptr->port = the_initzer->get_fe_sender_port();
	ptr->svr_addr = the_initzer->get_ebd_recv_svr_addr();
	ptr->recv_buf_sz = the_initzer->get_fe_sender_buf_sz();
	ptr->sock_buf = new unsigned char[ptr->recv_buf_sz];
	ptr->t_us = the_initzer->get_ebd_recv_t_us();

	return 0;
}

int ebd_recv::handle_msg(uint32_t* msg_body)
{
	/* The message type of the current thread are defined as following 
	 * 1 --> run status transition
	 * 2 --> To be defined yet 
	 * */
	uint32_t msg_type = msg_body[0] & 0xFFFFFF;
	switch (msg_type) {
	case 1:
		/* run status transition
		 * */
		return switch_run(msg_body[1]);
	default:
		return -E_MSG_TYPE;
	}

}

int ebd_recv::start()
{
	acq_stat = DAQ_RUN;
	
	/* establish the connection between the current thread and the frontend
	 * sender if not yet. */
	if (sock == -1) {
		int ret;
		int sz = MAX_SLOT_MAP;
		char* p_slot_map = slot_map;
		sock = my_tcp_clt::connect(port, svr_addr.c_str());
		if (sock == -1)
			return -E_SYSCALL;
		/* receive the slot map*/
		ret = recv(sock, slot_map, sz, MSG_WAITALL);
		if (ret != sz)
			return -E_SYSCALL;
		/* tell the ebd_sort the address of the slot map */
		return send_msg(EBD_SORT, 2, &p_slot_map, sizeof(p_slot_map));
	}
	return 0;
}

int ebd_recv::stop()
{
	acq_stat = DAQ_STOP;

	/* proporgate the stop message to the next thread */
	return send_msg(2, 1, &acq_stat, 4);
}

int ebd_recv::quit()
{
	acq_stat = DAQ_EXIT;
	close(sock);
	return 0;
}

int ebd_recv::main_proc()
{
	int sz, ret;
	if (has_sock_data()) {
		ret = recv(sock, &sz, 4, MSG_WAITALL);
		if (ret != 4)
			return -E_SYSCALL;
		if (sz == 0) {
			/* the frontend sends a 'stop' signal, we should stop */
			int status = DAQ_STOP;
			return send_msg(1, 1, &status, 4);
		}

		/* we expect a following data packets with total length of sz */
		return read_sock_data(sz);
	}

	/* no data available from socket */
	return 0;
}

int ebd_recv::read_sock_data(int sz)
{
	int ret;
	
	/* Because the recv_buf_sz equals to the frontend sender's sock_buf_sz,
	 * so we always have sz <= recv_buf_sz. */
	ret = recv(sock, sock_buf, sz, MSG_WAITALL);
	if (ret != sz)
		return -E_SYSCALL;
	/* now save it into the ring buffer */
	if (rb_fe->wait_buf_free(sz))
		return -E_RING_BUF_DATA;
	if (rb_fe->get_lock())
		return -E_SYSCALL;
	rb_fe->write1(sock_buf, sz);
	if (rb_fe->rel_lock())
		return -E_SYSCALL;
	return 0;
}

bool ebd_recv::has_sock_data()
{
	int ret;
        fd_set readfds;
        struct timeval timeout;

	timeout.tv_sec=0;
        timeout.tv_usec=t_us;
        FD_ZERO(&readfds); 
        FD_SET(sock, &readfds);
        ret = select(sock+1, &readfds, NULL, NULL, &timeout);
        if (ret == -1) 
	    return false;
        else 
            return ret;
}
