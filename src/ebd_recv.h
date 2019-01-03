#ifndef EBD_RECV_HHH
#define EBD_RECV_HHH

/* This is the receiver thread of the event builder. In this thread, it
 * receives data from frontend and saves it into the ring buffer: rb_data 
 *
 * The received data from frontend is saved directly into the rb_data without
 * re-orgnizing, so the data format in the rb_data is exactly the same as that
 * in the data ring buffer in frontend.
 * 
 * By B.Gao Dec. 2018 */

#include "ebd_thread.h"
#include <string>

class ebd_recv : public ebd_thread
{
public:
	ebd_recv();
	~ebd_recv();

private:
	static int ebd_recv_init(my_thread* This, initzer* the_initzer); 

	/* implementation of virtual functions declared in base classes. See
	 * comments in base classes for explainations */
	int handle_msg(uint32_t* msg_body);
	int start();
	int stop();
	int quit();
	int main_proc();

	/* read the socket data (the raw data from frontend) and save to the
	 * ring buffer.
	 * @param sz The size (bytes) of incoming data. 
	 * @return return 0 if succeed, otherwise return error code */
	int read_sock_data(int sz);

	/* determine if data available from the socket, return true if has
	 * data, otherwise return false */
	bool has_sock_data();

private:
	/* server port and address */
	int port;
	std::string svr_addr;

	/* communication sock */
	int sock;


	/* The buffer for receiving data from socket */
	unsigned char* sock_buf;

	/* The buffer size of sock_buf (should be the same as the sender buffer
	 * size of frontend) */
	int recv_buf_sz;
	
	/* The time interval (us) which the thread spend checking for incoming
	 * data packet from the socket (via select syscall) */
	int t_us;

	char slot_map[MAX_SLOT_MAP];
};



#endif 
