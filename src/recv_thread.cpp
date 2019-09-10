#include "recv_thread.h"
#include "my_tcp_clt.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <unistd.h>
#include <iostream>
#include <errno.h>

recv_thread::recv_thread()
{
	sock_buf = NULL;
	sock = -1;
}

recv_thread::~recv_thread()
{
	if (sock_buf)
		delete sock_buf;
}



int recv_thread::recv_start()
{
	/* establish the connection between the current thread and the server
	 * sender if not yet. */
	std::string err;
	if (sock == -1) {
		/* debug ...*/
		std::cout<<"connecting: port: "<<port<<" ip: "<<svr_addr.c_str()<<std::endl;
		/* *********/
		sock = my_tcp_clt::connect(port, svr_addr.c_str(), &err);
		if (sock == -1) {
			std::cout<<err<<std::endl;
			return -E_SYSCALL;
		}
		/* debug ...*/
		std::cout<<"connected"<<std::endl;
		/* *********/
	}
	return 0;
}

int recv_thread::recv_stop()
{
	return 0;
}

int recv_thread::recv_quit()
{
	close(sock);
	return 0;
}


int recv_thread::read_sock_data(ring_buf* recv_rb, int sz)
{
	int ret;
	
	/* Because the recv_buf_sz equals to the sender's sock_buf_sz,
	 * so we always have sz <= recv_buf_sz. */
	ret = recv(sock, sock_buf, sz, MSG_WAITALL);
	if (ret != sz)
		return -E_SYSCALL;
	/* now save it into the ring buffer */
	if (recv_rb->wait_buf_free(sz))
		return -E_RING_BUF_DATA;
	if (recv_rb->get_lock())
		return -E_SYSCALL;
	recv_rb->write1(sock_buf, sz);
	if (recv_rb->rel_lock())
		return -E_SYSCALL;
	return 0;
}

bool recv_thread::has_sock_data()
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
