#include "ebd_ctl.h"
#include "my_tcp_clt.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>

ebd_ctl::ebd_ctl()
{
	thread_id = 5;
	init_fun.push_back(&ebd_ctl_init);
}

ebd_ctl::~ebd_ctl()
{
}

int ebd_ctl::handle_msg(uint32_t* msg_body)
{
         /* The message type of the current thread are defined as following:
          * msg_type == 1 --> run status transition.                        
          * msg_type == 2 --> to be defined yet */                          

	uint32_t msg_type = msg_body[0] & 0xFFFFFF;

	switch (msg_type) {
	case 1:
		/* run status transition */
		return switch_run(msg_body[1]);
	default:
		return -E_MSG_TYPE;
	}
}

int ebd_ctl::start()
{
	int ret = ctl_start();
	RET_IF_NONZERO(ret);

	/* proporgate the start message to next thread (thread 3)*/
	return send_msg(4, 1, &real_stat, 4);
}

int ebd_ctl::stop()
{
	return ctl_stop();
}

int ebd_ctl::quit()
{
	acq_stat = DAQ_EXIT;
	return ctl_quit();
}

int ebd_ctl::ebd_ctl_init(my_thread* ptr, initzer* the_initzer)
{
	ebd_ctl* This = static_cast<ebd_ctl*>(ptr);
	int port, sock_buf_sz, t_us;
	std::string svr_addr;

	This->port = the_initzer->get_ctl_port();
	This->sock_buf_sz = the_initzer->get_ebd_ctl_buf_sz();
	This->sock_buf = new unsigned char[This->sock_buf_sz];
	This->svr_addr = the_initzer->get_ebd_ctl_svr_addr();
	This->t_us = the_initzer->get_ebd_ctl_t_us();

	/* the control thread needs always be in running status unless when to
	 * quit. */
	This->acq_stat = 1;

	return 0;
}


int ebd_ctl::handle_GUI_msg(unsigned char* msg)
{
	uint32_t* p_msg = reinterpret_cast<uint32_t*>(msg);
	uint32_t msg_type = p_msg[0];
	int ret;

	switch (msg_type) {
	int stat;
	case 0:
		/* a run status transition is requested. */
		stat = p_msg[1];
		if (stat == DAQ_STOP) {
			/* we ignore the stop run request, because we receive
			 * the message from frontend (by sending a zero length
			 * event) */
			return 0;
		}
		else if (stat == 1) {
			/* to start a run, thread 5 initiate the chain */
			return send_msg(5, 1, &stat, 4);
		}
		else if (stat == 2) {
			/* to quit the daq, all the thread exit in parallel. */
			ret = send_msg(1, 1, &stat, 4);
			RET_IF_NONZERO(ret);
			ret = send_msg(2, 1, &stat, 4);
			RET_IF_NONZERO(ret);
			ret = send_msg(3, 1, &stat, 4);
			RET_IF_NONZERO(ret);
			ret = send_msg(4, 1, &stat, 4);
			RET_IF_NONZERO(ret);
			ret = send_msg(5, 1, &stat, 4);
			RET_IF_NONZERO(ret);
			return 0;
		}
		break;
	default:
		/* unknown message type */
		return -E_MSG_TYPE;
	}

	return 0;
}
