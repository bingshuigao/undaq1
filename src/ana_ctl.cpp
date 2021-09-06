#include "ana_ctl.h"
#include "my_tcp_clt.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <string.h>

ana_ctl::ana_ctl()
{
	thread_id = 4;
	init_fun.push_back(&ana_ctl_init);
}

ana_ctl::~ana_ctl()
{
}

int ana_ctl::handle_msg(uint32_t* msg_body)
{
         /* The message type of the current thread are defined as following:
          * msg_type == 1 --> run status transition.                        
	  * msg_type == 2 --> scaler counters from ana_main
	  * msg_type == MSG_TEXT (100) --> text message (to gui)
	  * */                          

	uint32_t msg_type = msg_body[0] & 0xFFFFFF;
	unsigned char gui_msg[128];
	uint32_t* p_int = reinterpret_cast<uint32_t*>(gui_msg);

	switch (msg_type) {
	case 1:
		/* run status transition */
		return switch_run(msg_body[1]);
	case 2:
		/* scaler counters */
		p_int[0] = 6; /* gui message type */
		memcpy(p_int+1, msg_body+1, 30*4);
		p_int[31] = 0;
		return do_send(sock, gui_msg, 128, 0);
	case MSG_TEXT:
		p_int[0] = 5; /* gui message type */
		p_int[1] = msg_body[1];
		sprintf((char*)(gui_msg+8), "%s", (char*)(msg_body+2));
		p_int[31] = 0;
		return do_send(sock, gui_msg, 128, 0);
	default:
		return -E_MSG_TYPE;
	}
}

int ana_ctl::start()
{
	int ret = ctl_start();
	RET_IF_NONZERO(ret);

	/* proporgate the start message to next thread (thread 3)*/
	return send_msg(3, 1, &real_stat, 4);
}

int ana_ctl::stop()
{
	return ctl_stop();
}

int ana_ctl::quit()
{
	acq_stat = DAQ_EXIT;
	return ctl_quit();
}

int ana_ctl::ana_ctl_init(my_thread* ptr, initzer* the_initzer)
{
	ana_ctl* This = static_cast<ana_ctl*>(ptr);
	int port, sock_buf_sz, t_us;
	std::string svr_addr;

	This->port = the_initzer->get_ctl_port();
	This->sock_buf_sz = the_initzer->get_ana_ctl_buf_sz();
	This->sock_buf = new unsigned char[This->sock_buf_sz];
	This->svr_addr = the_initzer->get_ana_ctl_svr_addr();
	This->t_us = the_initzer->get_ana_ctl_t_us();

	/* the control thread needs always be in running status unless when to
	 * quit. */
	This->acq_stat = 1;

	/* Now we should connect the GUI */
	This->sock = my_tcp_clt::connect(This->port, This->svr_addr.c_str());
	if (This->sock == -1)
		return -E_SYSCALL;

	This->is_ctl_thread = true;
	return 0;
}


int ana_ctl::handle_GUI_msg(unsigned char* msg)
{
	uint32_t* p_msg = reinterpret_cast<uint32_t*>(msg);
	uint32_t msg_type = p_msg[0];
	int ret;
	int stat;
	unsigned char msg_send[128];
	uint32_t* p;
#ifdef DEBUG___
//	printf("received msg type: %d\n", msg_type);
#endif

	switch (msg_type) {
	case 0:
		/* a run status transition is requested. */
		stat = p_msg[1];
		if (stat == DAQ_STOP) {
			/* we ignore the stop run request, because we receive
			 * the message from event builder (by sending a zero
			 * length event) */
			return 0;
		}
		else if (stat == 1) {
			/* to start a run, thread 4 initiate the chain */
			return send_msg(4, 1, &stat, 4);
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
			return 0;
		}
		break;
	case 1:
		/* query name */
		p = reinterpret_cast<uint32_t*>(msg_send);
		p[0] = 1;
		p[31] = 0;
		sprintf((char*)msg_send+4, "analyzer");
		ret = do_send(sock, msg_send, 128, 0);
		RET_IF_NONZERO(ret);
		break;
	case 2:
		/* query status  */
		p = reinterpret_cast<uint32_t*>(msg_send);
		p[0] = 2;
		p[1] = real_stat;
		p[31] = 0;
		ret = do_send(sock, msg_send, 128, 0);
		RET_IF_NONZERO(ret);
		break;
	case 4:
		/* query ring buffer status */
		p = reinterpret_cast<uint32_t*>(msg_send);
		p[0] = 4; /* message type */
		p[1] = rb_scal->get_sz();
		p[2] = rb_scal->get_used();
		p[3] = rb_evt->get_sz();
		p[4] = rb_evt->get_used();
		p[31] = 0;
		ret = do_send(sock, msg_send, 128, 0);
		RET_IF_NONZERO(ret);
		break;
	default:
		/* unknown message type */
		return -E_MSG_TYPE;
	}

	return 0;
}
