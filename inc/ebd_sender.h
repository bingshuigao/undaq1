#ifndef EBD_SENDER_HHH
#define EBD_SENDER_HHHy

/* this is the sender thread of the event builder (send built events to logger
 * and analyzer) 
 *
 * By B.Gao Jan. 2019
 *
 * */

#include "ebd_thread.h"
#include "my_tcp_svr.h"

class ebd_sender : public ebd_thread
{
public:
	ebd_sender();
	~ebd_sender();
	
private:
	static int ebd_sender_init(my_thread* This, initzer* the_initzer);
	
	/* implementation of virtual functions declared in base classes. See
	 * comments in base classes for explainations */
	int handle_msg(uint32_t* msg_body);
	int start();
	int stop();
	int quit();
	int main_proc();
	
	/* flush the data ring buffer. (send them all out to the client)
	 * Return 0 if succeed, otherwise return error code.
	 * */
	int flush_buf();
	
	/* send a chunk of data out to the socket client.
	 * Return 0 if succeed, otherwise return error code.
	 * BE CAREFUL!!!! The lock should be hold when calling this method. The
	 * lock will be released before returning from this method (don't
	 * double release).
	 * @param the_rb the ring buffer whose data is to be sent .
	 * */
	int send_data(ring_buf* rb);

	/* reconnect the analyzer (do not block). If no connect request is
	 * found, send another message to this thread (type 2). 
	 * Return 0 if succeed, otherwise return error code.
	 * */
	int re_conn_ana();

private:
	/* listening socket*/
	my_tcp_svr* svr;
	/* conmmunication socket */
	int sock_log;
	int sock_ana;

	unsigned char* sock_buf;
	int sock_buf_sz;

};


#endif 
