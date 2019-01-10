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
#include "recv_thread.h"
#include <string>

class ebd_recv : public ebd_thread, public recv_thread
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


private:
	char slot_map[MAX_SLOT_MAP];
};



#endif 
