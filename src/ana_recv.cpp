#include "ana_recv.h"
#include "my_tcp_clt.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <unistd.h>

ana_recv::ana_recv()
{
	thread_id = 1;
	init_fun.push_back(&ana_recv_init);
}

ana_recv::~ana_recv()
{
}

int ana_recv::ana_recv_init(my_thread* This, initzer* the_initzer)
{
	ana_recv* ptr = reinterpret_cast<ana_recv*>(This);

	ptr->port = the_initzer->get_ebd_sender_port();
	ptr->svr_addr = the_initzer->get_ana_recv_svr_addr();
	ptr->recv_buf_sz = the_initzer->get_ebd_sender_buf_sz();
	ptr->sock_buf = new unsigned char[ptr->recv_buf_sz];
	ptr->t_us = the_initzer->get_ana_recv_t_us();

	return 0;
}

int ana_recv::handle_msg(uint32_t* msg_body)
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

int ana_recv::start()
{
	acq_stat = DAQ_RUN;
	int ret = recv_start();

	RET_IF_NONZERO(ret);
	return 0;
}

int ana_recv::stop()
{
	int ret = recv_stop();
	RET_IF_NONZERO(ret);

	acq_stat = DAQ_STOP;
	/* proporgate the stop message to the next thread */
	return send_msg(2, 1, &acq_stat, 4);
}

int ana_recv::quit()
{
	int ret = recv_quit();
	RET_IF_NONZERO(ret);

	acq_stat = DAQ_EXIT;
	return 0;
}

int ana_recv::main_proc()
{
	int sz, ret;
	unsigned char type;
	ring_buf* the_rb;
	if (has_sock_data()) {
		ret = recv(sock, &sz, 4, MSG_WAITALL);
		if (ret != 4)
			return -E_SYSCALL;
		if (sz == 0) {
			/* the event builder sends a 'stop' signal, we should
			 * stop */
			int status = DAQ_STOP;
			return send_msg(1, 1, &status, 4);
		}

		/* check the data type (trigger or scaler) */
		ret = recv(sock, &type, 1, MSG_WAITALL);
		if (ret != 1)
			return -E_SYSCALL;
		if (type == 1)
			the_rb = rb_scal;
		else if (type == 2)
			the_rb = rb_evt;
		else
			return -E_EVT_TYPE;


		/* receive the data */
		return read_sock_data(the_rb, sz);
	}

	/* no data available from socket, take a short break and return */
	usleep(120);
	return 0;
}
