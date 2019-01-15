#ifndef ANA_THREAD_HHH
#define ANA_THREAD_HHH

/* This is the base class of the threads of analyzer. For details, refer
 * to the subclasses.
 * 
 *  The thread ids are defined as following:
 *  1--> The receiver thread
 *  2--> The analysis thread
 *  3--> server thread of roody
 *  4--> The controller thread
 *
 * By B.Gao Jan. 2019 */

#include "daq_thread.h"

class ana_thread : public daq_thread
{
public:
	ana_thread();
	~ana_thread();
private:
	static int ana_thread_init(my_thread* This, initzer* the_initzer);

protected:
	/* This is for data sharing of built event */
	ring_buf* rb_evt;

};



#endif
