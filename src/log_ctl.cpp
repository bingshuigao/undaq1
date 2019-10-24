#include "log_ctl.h"
#include "my_tcp_clt.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>

log_ctl::log_ctl()
{
	thread_id = 3;
	init_fun.push_back(&log_ctl_init);
}

log_ctl::~log_ctl()
{
}

int log_ctl::handle_msg(uint32_t* msg_body)
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

int log_ctl::start()
{
	uint32_t new_msg[3];
	int ret = ctl_start();
	RET_IF_NONZERO(ret);

	/* proporgate the start message to next thread (thread 2)*/
	new_msg[0] = real_stat; /* new status */
	new_msg[1] = run_num; /* run number */
	new_msg[2] = if_save; /* if save flag */
	/* debug ...*/
	std::cout<<"run number from ctrl: "<<run_num<<std::endl;
	/* ************/
	return send_msg(2, 1, new_msg, 12);
}

int log_ctl::stop()
{
	return ctl_stop();
}

int log_ctl::quit()
{
	acq_stat = DAQ_EXIT;
	return ctl_quit();
}

int log_ctl::log_ctl_init(my_thread* ptr, initzer* the_initzer)
{
	log_ctl* This = static_cast<log_ctl*>(ptr);
	int port, sock_buf_sz, t_us;
	std::string svr_addr;

	This->port = the_initzer->get_ctl_port();
	This->sock_buf_sz = the_initzer->get_log_ctl_buf_sz();
	This->sock_buf = new unsigned char[This->sock_buf_sz];
	This->svr_addr = the_initzer->get_log_ctl_svr_addr(); 
	This->t_us = the_initzer->get_log_ctl_t_us();

	/* the control thread needs always be in running status unless when to
	 * quit. */
	This->acq_stat = 1;

	/* Now we should connect the GUI */
	This->sock = my_tcp_clt::connect(This->port, This->svr_addr.c_str());
	if (This->sock == -1)
		return -E_SYSCALL;
	return 0;
}


int log_ctl::handle_GUI_msg(unsigned char* msg)
{
	uint32_t* p_msg = reinterpret_cast<uint32_t*>(msg);
	uint32_t msg_type = p_msg[0];
	int ret;
	int stat;
	unsigned char msg_send[128];
	int* p;

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
			/* to start a run, thread 3 initiate the chain,
			 * additional parameters are also needed to start a new
			 * run, namely the run number and if_save flag. They
			 * are in p_msg[2] and p_msg[3]. */
			run_num = p_msg[2];
			if_save = p_msg[3];
			/* debug ...*/
//			std::cout<<"run_num "<<run_num<<std::endl;
			/* ********...*/
			return send_msg(3, 1, &stat, 4);
		}
		else if (stat == 2) {
			/* to quit the daq, all the thread exit in parallel. */
			ret = send_msg(1, 1, &stat, 4);
			RET_IF_NONZERO(ret);
			ret = send_msg(2, 1, &stat, 4);
			RET_IF_NONZERO(ret);
			ret = send_msg(3, 1, &stat, 4);
			RET_IF_NONZERO(ret);
			return 0;
		}
		break;
	case 1:
		/* query name */
		p = reinterpret_cast<int32_t*>(msg_send);
		p[0] = 1;
		sprintf((char*)msg_send+4, "logger");
		ret = do_send(sock, msg_send, 128, 0);
		RET_IF_NONZERO(ret);
		break;
	case 2:
		/* query status  */
		p = reinterpret_cast<int32_t*>(msg_send);
		p[0] = 2;
		p[1] = real_stat;
		ret = do_send(sock, msg_send, 128, 0);
		RET_IF_NONZERO(ret);
		break;
	default:
		/* unknown message type */
		return -E_MSG_TYPE;
	}

	return 0;
}
