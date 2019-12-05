#include "fe_ctl.h"
#include "my_tcp_clt.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>

fe_ctl::fe_ctl()
{
	thread_id = 4;
	init_fun.push_back(&fe_ctl_init);
}

fe_ctl::~fe_ctl()
{
}

int fe_ctl::handle_msg(uint32_t* msg_body)
{
         /* The message type of the current thread are defined as following:
          * msg_type == 1 --> run status transition.                        
          * msg_type == 2 --> to be defined yet 
	  * msg_type == 3 --> A warning message
	  * msg_type == 4 --> reply to the query of statistics (from rd_trig
	  * thread)
	  * */                          

	uint32_t msg_type = msg_body[0] & 0xFFFFFF;
	unsigned char gui_msg[128];
	uint32_t* p_int = reinterpret_cast<uint32_t*>(gui_msg);

	switch (msg_type) {
	case 1:
		/* run status transition */
		return switch_run(msg_body[1]);
	case 3:
		/* this is a warning to be sent to controler */
		std::cout<<"warning received!"<<std::endl;
		return 0;
	case 4:
		/* reply from rd_trig thread on the statistics */
		p_int[0] = 3; /* gui message type */
		p_int[1] = msg_body[1];
		p_int[2] = msg_body[2];
		p_int[3] = msg_body[3];
		p_int[4] = msg_body[4];
		std::cout<<"sending rates..."<<std::endl;
		p_int[31] = 0;
		return do_send(sock, gui_msg, 128, 0);
	default:
		return -E_MSG_TYPE;
	}
}

int fe_ctl::start()
{
	int ret = ctl_start();
	RET_IF_NONZERO(ret);
	
	/* proporgate the start message to next thread (thread 3)*/
	return send_msg(3, 1, &real_stat, 4);
}

int fe_ctl::stop()
{
	return ctl_stop();
}

int fe_ctl::quit()
{
	acq_stat = DAQ_EXIT;
	return ctl_quit();
}

int fe_ctl::fe_ctl_init(my_thread* ptr, initzer* the_initzer)
{
	fe_ctl* This = static_cast<fe_ctl*>(ptr);
	int port, sock_buf_sz, t_us;
	std::string svr_addr;

	This->port = the_initzer->get_ctl_port();
	This->sock_buf_sz = the_initzer->get_fe_ctl_buf_sz();
	This->sock_buf = new unsigned char[This->sock_buf_sz];
	This->svr_addr = the_initzer->get_fe_ctl_svr_addr();
	This->t_us = the_initzer->get_fe_ctl_t_us();

	/* Now we need to connect to the GUI */
	This->sock = my_tcp_clt::connect(This->port, This->svr_addr.c_str());

	if (This->sock == -1)
		return -E_SYSCALL;

	/* the control thread needs always be in running status unless when to
	 * quit. */
	This->acq_stat = 1;

	return 0;
}


int fe_ctl::handle_GUI_msg(unsigned char* msg)
{
	uint32_t* p_msg = reinterpret_cast<uint32_t*>(msg);
	uint32_t msg_type = p_msg[0];
	int ret, stat;
	unsigned char msg_send[128];
	uint32_t *p;
#ifdef DEBUG___
//	printf("received msg type: %d\n", msg_type);
#endif

	switch (msg_type) {
	case 0:
		/* a run status transition is requested. */
		stat = p_msg[1];
		if (stat == 0) {
			/* to stop a run, thread 1 initiate the chain */
			/* debug ... */
			std::cout<<"stop command received"<<std::endl;
			return send_msg(1, 1, &stat, 4);
		}
		else if (stat == 1) {
			/* to start a run, thread 4 initiate the chain */
			/* debug ... */
			std::cout<<"start command received"<<std::endl;
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
		sprintf((char*)msg_send+4, "frontend");
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
		/* query statistics, the respond has message type of 3 */
		ret = send_msg(1, 4, NULL, 0);
		RET_IF_NONZERO(ret);
		break;
	case 4:
		/* query the status of the ring buffer (rb_data) */
		p = reinterpret_cast<uint32_t*>(msg_send);
		p[0] = 4;
		p[1] = rb_data->get_sz(); /* total sz */ 
		p[2] = rb_data->get_used(); /* used sz */
		if (p[2] == -1)
			return -E_SYSCALL;
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
