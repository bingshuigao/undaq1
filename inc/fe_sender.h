#ifndef FE_SENDER_HHH
#define FE_SENDER_HHH

/* This class manages the frontend ring buffer. It basically reads data from
 * the ring buffer and then sends them out to the event builder. Here this
 * class is the tcp server. 
 *
 * By B.Gao Nov. 2018 */

#include "my_tcp_svr.h"
#include <sys/types.h>
#include <sys/socket.h>
#include "fe_thread.h"
#include "my_tcp_svr.h"


class fe_sender : public fe_thread
{
public:
	fe_sender();
	~fe_sender();

protected:
	/* handle the message. The pointer points to the message body (not
	 * including the message header).
	 * return 0 if succeed, otherwise return error code */
	virtual int handle_msg(uint32_t* msg_body);

	/* Start/stop/quit the run.
	 * Return 0 if succeed. otherwise return error code. */
	virtual int start();
	virtual int stop();
	virtual int quit();


	/* The main procudure in the main_loop. This procudure is executed only
	 * if the daq is in running status. 
	 * Return 0 if succeed, otherwise return error code. */
	virtual int main_proc();

	/* flush the data ring buffer. (send them all out to the client)
	 * Return 0 if succeed, otherwise return error code.
	 * */
	int flush_buf();

	/* send a chunk of data out to the socket client.
	 * Return 0 if succeed, otherwise return error code.
	 * BE CAREFUL!!!! The lock should be hold when calling this method. The
	 * lock will be released before returning from this method (don't
	 * double release).
	 * */
	int send_data();

	/* send the slot map (obtained from initzer) to the event builder. We
	 * need to do this because only frontend has access to the slot map,
	 * event builder does not know the map. 
	 * Return 0 if succeed, otherwise return error code.
	 * */
	int send_slot_map();

	/* send the mod_rb_par to the event builder. The event builder will
	 * need these data to create the dedicated ring buffers for each
	 * module.
	 * Return 0 if succeed, otherwise return error code.
	 * */
	int send_mod_rb_par();

private:
	static int fe_sender_init(my_thread* This, initzer* the_initzer);
	
	/* listening socket*/
	my_tcp_svr* svr;
	/* conmmunication socket */
	int sock;

	/* The interval (in us) the thread should spend checking the data
	 * buffer. This basically sets the response time (to messages like
	 * start/stop) of the thread. */
	int itv;

	my_timer timer_itv;
	unsigned char* sock_buf;
	int sock_buf_sz;
	char* slot_map;


};






#endif
