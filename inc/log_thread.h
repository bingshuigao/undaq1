#ifndef LOG_THREAD_HHH
#define LOG_THREAD_HHH

/* This is the base class of the threads of logger. For details, refer
 * to the subclasses.
 * 
 *  The thread ids are defined as following:
 *  1--> The receiver thread
 *  2--> The saving thread
 *  3--> The controller thread
 *
 * By B.Gao Jan. 2019 */

#include "daq_thread.h"

class log_thread : public daq_thread
{
public:
	log_thread();
	~log_thread();
private:
	static int log_thread_init(my_thread* This, initzer* the_initzer);

protected:
	/* This is for data sharing of scaler */
	ring_buf* rb_trig;

	/* This is for data sharing of scaler */
	ring_buf* rb_scal;

};



#endif
