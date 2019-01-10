#ifndef LOG_RECV_HHH
#define LOG_RECV_HHH

/* This is the receiver thread of the logger . In this thread, it
 * receives data from event builder and saves it into the ring buffer
 *
 * By B.Gao Jan. 2019 */

#include "recv_thread.h"
#include "log_thread.h"
#include <string>

class log_recv : public log_thread, public recv_thread
{
public:
	log_recv();
	~log_recv();

private:
	static int log_recv_init(my_thread* This, initzer* the_initzer); 

	/* implementation of virtual functions declared in base classes. See
	 * comments in base classes for explainations */
	int handle_msg(uint32_t* msg_body);
	int start();
	int stop();
	int quit();
	int main_proc();


private:
};



#endif 
