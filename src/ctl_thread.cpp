#include "ctl_thread.h"
#include "imp_daq.h"
#include "my_tcp_clt.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>

ctl_thread::ctl_thread()
{
	sock = -1;
	sock_buf = NULL;
	real_stat = 0;
}

ctl_thread::~ctl_thread()
{
	if (sock_buf)
		delete sock_buf;
}


int ctl_thread::ctl_start()
{
	real_stat = 1;

	/* establish the connection between the thread (client) and the gui
	 * controller (server). */
	if (sock == -1) {
		sock = my_tcp_clt::connect(port, svr_addr.c_str());
		if (sock == -1)
			return -E_SYSCALL;
	}
	return 0;
}

int ctl_thread::ctl_stop()
{
	real_stat = 0;
	return 0;
}

int ctl_thread::ctl_quit()
{
	real_stat = 2;
	close(sock);
	sock = -1;
	return 0;
}


int ctl_thread::ctl_main_proc()
{
	/* The main purpose of this function is to communicate with the GUI
	 * controler. The way how they communicate is that the current thread
	 * waits for incoming packets from the GUI, decode the packet and
	 * respond accudingly. Because the response sometimes requires
	 * interprocess communication (via rb_msg), so the response sometimes
	 * is done not here, but in the message handler function handle_msg (by
	 * sending a message into the rb_msg). In
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
	 *     1-->query name
	 *     2--> query status
	 *     3--> to be defined...
	 *     1000 --> This is the continue of the previous message.
	 *
	 * The complementary information depends on the message type
	 * The end of message mark: 
	 *     0--> The message is complete.
	 *     1--> The message is incomplete, more information will be found
	 *     in the following packets. The following packets has no message
	 *     type, but still has the end of message mark.
	 *
	 *
	 *  When sending messages to the GUI controller, similar formats are used:
	 *      _________________________________________
	 *      |  message type (4-byte int)            |
	 *      |_______________________________________|
	 *      |  complementary information (123 byte) |
	 *      |_______________________________________|
	 *      | end of message mark (1 byte)          |
	 *      |_______________________________________|
	 *      
	 *      The definations of message types are :
	 *      0--> A system message (warning or complaning...)
	 *      1--> Respond to a query name
	 *
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

bool ctl_thread::has_GUI_msg(int* err)
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


