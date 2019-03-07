#include "ebd_recv.h"
#include "my_tcp_clt.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <unistd.h>

ebd_recv::ebd_recv()
{
	thread_id = 1;
	init_fun.push_back(&ebd_recv_init);
}

ebd_recv::~ebd_recv()
{
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
	int flag = sock;
	int ret = recv_start();

	RET_IF_NONZERO(ret);
	
	/* receive the slot map and ring buffer data if not yet.*/
	if (flag == -1) {
		int sz = MAX_SLOT_MAP;
		char* p_slot_map = slot_map;
	
		ret = init_rb_data();
		RET_IF_NONZERO(ret);
		
		ret = recv(sock, slot_map, sz, MSG_WAITALL);
		if (ret != sz)
			return -E_SYSCALL;
		/* tell the ebd_sort the address of the slot map */
		return send_msg(EBD_SORT, 2, &p_slot_map, sizeof(p_slot_map));
		
	}
	return 0;
}

int ebd_recv::init_rb_data()
{
	int ret, n, i, slot, crate;
	ring_buf* p_rb;

	ret = recv(sock, &n, 4, MSG_WAITALL);
	RET_IF_NONZERO(ret);
	for (i = 0; i < n; i++) {
		ret = recv(sock, &slot, 4, MSG_WAITALL);
		RET_IF_NONZERO(ret);
		ret = recv(sock, &crate, 4, MSG_WAITALL);
		RET_IF_NONZERO(ret);
		p_rb = new ring_buf;
		if (p_rb->init(rb_data_sz)) {
			delete p_rb;
			p_rb = NULL;
		}
		if (p_rb) {
			 char* p_data = p_rb->get_usr_data();
			 p_data[0] = crate;
			 p_data[1] = slot;
			 p_data[2] = 0; /* a marker used by ebd_merge
                                               (can build or not) */
		}
		rb_data.push_back(p_rb);
	}
	return 0;
}

int ebd_recv::stop()
{
	int ret = recv_stop();
	RET_IF_NONZERO(ret);

	acq_stat = DAQ_STOP;
	/* proporgate the stop message to the next thread */
	return send_msg(2, 1, &acq_stat, 4);
}

int ebd_recv::quit()
{
	int ret = recv_quit();
	RET_IF_NONZERO(ret);

	acq_stat = DAQ_EXIT;
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
		return read_sock_data(rb_fe, sz);
	}

	/* no data available from socket */
	return 0;
}
