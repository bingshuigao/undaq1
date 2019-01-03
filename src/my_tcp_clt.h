#ifndef MY_TCP_CLT_HHH
#define MY_TCP_CLT_HHH

/* This is a simple tcp client class. The typical use is (not including error
 * checkings) :
 *
 *         int port = 1234;
 *         int sock;
 *         
 *         sock = my_tcp_clt::connect(port, "127.0.0.1");
 *         receiv_and_send_msg_using_the_new_socket(sock);
 *         (when done) close(sock);
 *
 *
 *
 * By B.Gao Nov. 2018 */

#include <string>

class my_tcp_clt
{
public:
	my_tcp_clt() {};
	~my_tcp_clt() {};

	/* connect to a server.
	 * @param port. port number of the connection.
	 * @param host_name. host name of the server. (can be the format of
	 * 127.0.0.1 or 'localhost')
	 * return  a nonnegative integer that is a descriptor of the socket for
	 * the established connection.  On error, -1 is returned,*/
	static int connect(int port, const char* host_name, 
			std::string* err=NULL); 
};

#endif
