#include "ebd_ctl.h"
#include "my_tcp_clt.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <string.h>

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
          * msg_type == 2 --> n_mod
	  * msg_type == 3 --> broken pipe
	  * msg_type == 4 --> evt counts (from ebd sort)
	  * msg_type == MSG_TEXT (100) --> text message (to gui)
	  * */                          

	uint32_t msg_type = msg_body[0] & 0xFFFFFF;
	uint32_t n_mod;
	unsigned char msg_send[1280];
	uint32_t* p;
	uint32_t* p_int = reinterpret_cast<uint32_t*>(msg_send);

	switch (msg_type) {
	case 1:
		/* run status transition */
		return switch_run(msg_body[1]);
	case 2:
		/* The number of vme modules is returned in the message */
		n_mod = msg_body[1];
		p = reinterpret_cast<uint32_t*>(msg_send);
		p[0] = 3;
		p[1] = n_mod;
		p[31] = 0;
		return do_send(sock, msg_send, 128, 0);
	case MSG_TEXT:
		p_int[0] = 5; /* gui message type */
		p_int[1] = msg_body[1];
		sprintf((char*)(msg_send+8), "%s", (char*)(msg_body+2));
		p_int[31] = 0;
		return do_send(sock, msg_send, 128, 0);
	case 3:
		/* broken pipe */
		p_int[0] = 6; /* gui message type */
		p_int[31] = 0; 
		return do_send(sock, msg_send, 128, 0);
	case 4:
		/* evt cnts */
		p_int[0] = 7; /* gui message type */
		memcpy(msg_send+4, msg_body+1, 153*4);
		return do_send_msg_all(sock, msg_send, 154*4, 0);
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

	/* Now we should connect the GUI */
	This->sock = my_tcp_clt::connect(This->port, This->svr_addr.c_str());
	if (This->sock == -1)
		return -E_SYSCALL;
	This->is_ctl_thread = true;

	return 0;
}


unsigned char msg_send[1024*1024];
int ebd_ctl::handle_GUI_msg(unsigned char* msg)
{
	uint32_t* p_msg = reinterpret_cast<uint32_t*>(msg);
	uint32_t msg_type = p_msg[0];
	int ret;
	int stat;
	uint32_t *p, *p0;
#ifdef DEBUG___
//	printf("received msg type: %d\n", msg_type);
#endif

	switch (msg_type) {
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
	case 1:
		/* query name */
		p = reinterpret_cast<uint32_t*>(msg_send);
		p[0] = 1;
		p[31] = 0;
		sprintf((char*)msg_send+4, "event builder");
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
	case 3:
		/* query number of vme modules (in the ebd_sort thread)*/
		ret = send_msg(2, 5, NULL, 0);
		RET_IF_NONZERO(ret);
		break;
	case 4:
		/* query ring buffers status. unlike case 3, we can directly
		 * query the ring buffer in this thread, because by the time we
		 * receive this query, the rb_map (ebd_sort thread) must have
		 * been initialized  */
		p = reinterpret_cast<uint32_t*>(msg_send);
		p0 = p;
		/* the message header */
		*p = 4; /* message type is 4 */
		p++;

		/* the receive buffers */
		for (auto it = rb_fe.begin(); it != rb_fe.end(); it++) {
			*p = (*it)->get_sz(); p++;
			*p = (*it)->get_used(); p++;
		}
		/* the individual vme module buffers */
		for (auto it = rb_data.begin(); it != rb_data.end(); it++) {
			/* slot and crate number (the lower bytes)  */
			*p = ((uint32_t*)(*it)->get_usr_data())[0]; p++; 
			/* total and used size */
			*p = (*it)->get_sz(); p++;
			*p = (*it)->get_used(); p++;
		}
		/* the scaler buffer */
		*p = rb_scal->get_sz(); p++;
		*p = rb_scal->get_used(); p++;
		/* the built-event buffer */
		*p = rb_evt->get_sz(); p++;
		*p = rb_evt->get_used(); p++;
		
		if (p-p0 <= 31) {
			*p = 0;
			ret = do_send(sock, msg_send, 128, 0);
		}
		else {
			ret = do_send_msg_all(sock, msg_send, p-p0, 0);
		}
		RET_IF_NONZERO(ret);
		break;
	case 5:
		/* query evt couts of each module */
		ret = send_msg(2, 6, NULL, 0);
		RET_IF_NONZERO(ret);
		break;
	default:
		/* unknown message type */
		return -E_MSG_TYPE;
	}

	return 0;
}
