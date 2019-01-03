#ifndef MY_TCP_SVR_HHH
#define MY_TCP_SVR_HHH

/* This is a simple tcp server class. The typical use is (errors are not
 * checked):
 *
 *         int port = 1234;
 *         int sock;
 *         
 *         my_tcp_svr svr;
 *         svr.init(port);
 *         sock = svr.accept();
 *         svr.destroy();
 *         receiv_and_send_msg_using_the_new_socket(sock);
 *         (when done) close(sock);
 *
 *
 *
 * By B.Gao Nov. 2018 */

#include <sys/types.h>     
#include <sys/socket.h>
#include <string>
#include <unistd.h>

class my_tcp_svr
{
public:
	my_tcp_svr() {};
	~my_tcp_svr() {};

	/* Initialize the server.
	 * @param port. The port number of listening 
	 * @param backlog. The max length of the queue of the pending
	 * connection (see man 2 listen).
	 * @param err. (If not NULL) Contains the error message if any.
	 * @return return 0 if succeed, otherwise return -1 */
	int init(int port, int backlog=128, std::string* err=NULL);

	/* accept incomming connections.
	 * return  a nonnegative integer that is a descriptor for the accepted
	 * socket.  On error, -1 is returned,*/
	int accept(struct sockaddr *addr = NULL, socklen_t *addrlen = NULL) 
	{return ::accept(sock, addr, addrlen);}

	void destroy() {close(sock);}

	int get_sock() {return sock;}

private:
	/* the underlying listening socket file descriptor */
	int sock;
};



#endif
