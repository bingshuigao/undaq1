#include "fe_ctl.h"
#include "my_tcp_clt.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>

fe_ctl::fe_ctl()
{
	thread_id = 4;
	sock = -1;
	sock_buf = NULL;
	real_stat = 0;
	init_fun.push_back(&fe_ctl_init);
}

fe_ctl::~fe_ctl()
{
	if (sock_buf)
		delete sock_buf;
}

int fe_ctl::handle_msg(uint32_t* msg_body)
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

int fe_ctl::start()
{
	real_stat = 1;

	/* establish the connection between the thread (client) and the gui
	 * controller (server). */
	if (sock == -1) {
		sock = my_tcp_clt::connect(port, svr_addr.c_str());
		if (sock == -1)
			return -E_SYSCALL;
	}

	/* proporgate the start message to next thread (thread 3)*/
	return send_msg(3, 1, &real_stat, 4);
}

int fe_ctl::stop()
{
	real_stat = 0;
	return 0;
}

int fe_ctl::quit()
{
	acq_stat = 2;
	real_stat = 2;
	close(sock);
	sock = -1;
	return 0;
}

int fe_ctl::fe_ctl_init(my_thread* ptr, initzer* the_initzer)
{
	fe_ctl* This = static_cast<fe_ctl*>(ptr);
	int port, sock_buf_sz, t_us;
	std::string svr_addr;

	This->port = the_initzer->get_ctl_port_fe();
	This->sock_buf_sz = the_initzer->get_fe_ctl_buf_sz();
	This->sock_buf = new unsigned char[This->sock_buf_sz];
	This->svr_addr = the_initzer->get_fe_ctl_svr_addr();
	This->t_us = the_initzer->get_fe_ctl_t_us();

	/* the control thread needs always be in running status unless when to
	 * quit. */
	This->acq_stat = 1;


}
int fe_ctl::main_proc()
{
	/* The main purpose of this function is to communicate with the GUI
	 * controler. The way how they communicate is that the current thread
	 * waits for incoming packets from the GUI, decode the packet and
	 * respond accudingly. Because the response sometimes requires
	 * interprocess communication (via rb_msg), so the response sometimes
	 * is done not here, but in the message handler function handle_msg. In
	 * that case, here we simply mark that the current thread needs to
	 * respond to the GUI. The incoming packet from the GUI is in unit of
	 * 128-byte long packets. The definition of the packets is flexible,
	 * but the last byte is fixed, it indicate whether more data are
	 * expected from the following packets (that means the current packet
	 * is only part of the message). We define the packet as following:
	 *
	 *      _________________________________________
	 *      |  message type (4-byte int)            |
	 *      |_______________________________________|
	 *      |  complementary information (123 byte) |
	 *      |_______________________________________|
	 *      | end of message mark (1 byte)          |
	 *      |_______________________________________|
	 *
	 * The message type is defined as following:
	 *     0-->run transition required. The new run status is contained in
	 *     the following 4-byte word.
	 *     1--> To be defined....
	 *
	 * The complementary information depends on the message type
	 * The end of message mark: 
	 *     0--> The message is complete.
	 *     1--> The message is incomplete, more information will be found
	 *     in the following packets. The following packets has no message
	 *     type, but still has the end of message mark.
	 *
	 *
	 *      */

	int ret;
	unsigned char msg[128];

	if (has_GUI_msg()) {
		ret = recv(sock, msg, 128, MSG_WAITALL);
		if (ret != 128)
			return -E_SYSCALL;
		return handle_GUI_msg(msg);
	}

	return 0;
}

bool fe_ctl::has_GUI_msg(int* err)
{
        int ret;
        fd_set readfds;
        struct timeval timeout;

	timeout.tv_sec=0;
        timeout.tv_usec=t_us;
        FD_ZERO(&readfds); 
        FD_SET(sock, &readfds);
        ret = select(sock+1, &readfds, NULL, NULL, &timeout);
        if (ret == -1) {
            if (err)
		    *err = -E_SYSCALL;
	    return false;
        }
        else 
            return ret;
}


int fe_ctl::handle_GUI_msg(unsigned char* msg)
{
	uint32_t* p_msg = reinterpret_cast<uint32_t*>(msg);
	uint32_t msg_type = p_msg[0];
	int ret;

	switch (msg_type) {
	int stat;
	case 0:
		/* a run status transition is requested. */
		stat = p_msg[1];
		if (stat == 0) {
			/* to stop a run, thread 1 initiate the chain */
			return send_msg(1, 1, &stat, 4);
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
	default:
		/* unknown message type */
		return -E_MSG_TYPE;
	}

	return 0;
}
