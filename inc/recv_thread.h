#ifndef RECV_THREAD_HHH
#define RECV_THREAD_HHH

/* This is the base class for the receiver threads. In this thread, it receives
 * data from server and saves it into the ring buffer.
 *
 * The received data from server is saved directly into the ring buffer without
 * re-orgnizing, so the data format in the ring buffer is exactly the same as
 * that in the data ring buffer in the server.
 * 
 * By B.Gao Jan. 2019 */

#include <string>
#include "err_code.h"
#include "ring_buf.h"

class recv_thread
{
public:
	recv_thread();
	~recv_thread();

protected:

	/* read the socket data (the raw data from the server) and save to the
	 * ring buffer.
	 * @param sz The size (bytes) of incoming data. 
	 * @param rb The ring buffer where the received data is saved.
	 * @return return 0 if succeed, otherwise return error code */
	int read_sock_data(ring_buf* rb, int sz);

	/* determine if data available from the socket, return true if has
	 * data, otherwise return false */
	bool has_sock_data();

	/* this is called every time when the DAQ starts/stops/quits.
	 * @return return 0 if succeed, otherwise return error code 
	 * */
	int recv_start();
	int recv_stop();
	int recv_quit();

protected:
	/* server port and address */
	int port;
	std::string svr_addr;

	/* communication sock */
	int sock;


	/* The buffer for receiving data from socket */
	unsigned char* sock_buf;

	/* The buffer size of sock_buf (should be the same as the sender buffer
	 * size of server) */
	int recv_buf_sz;
	
	/* The time interval (us) which the thread spend checking for incoming
	 * data packet from the socket (via select syscall) */
	int t_us;
};



#endif 
