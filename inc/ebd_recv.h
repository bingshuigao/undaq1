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
	/* total is the total number of receiver threads, n is the n'th
	 * receiver thread, starting from zero (there can be more than one
	 * receiver thread since more than one frondend is allowed) */
	ebd_recv(int total, int n);
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

	/* init the individual ring buffers for each vme module. We need to
	 * receive the parameters of these ring buffers from frontend, then
	 * crate these ring buffers. 
	 * return 0 if succeed, otherwise return error code.
	 * */
	int init_rb_data();

	/* Since there could be more than one receiver thread, and they are
	 * chained when start/stop runs, this function make sense. return true
	 * if it is the last thread in the chain, otherwise return false. */
	bool is_last_thread();
	/* return the thread id of the next receiver thread in the chain. */
	int next_thread();

private:
	char slot_map[MAX_SLOT_MAP];
	uint64_t clk_map[MAX_CLK_MAP];

	/* total number of receiver threads */
	int total_thread;

	/* number of received stop signals. Since there can be more than one
	 * receiver thread, there should be a synchronization mechanism when
	 * star/stop runs. The way to synchronize here is to star/stop the
	 * threads sequentially: when starting a run, the start request chain
	 * is propagated in the order: 1->11->21->31->... However, it's more
	 * tricky when stopping runs. This is because the stop request is not
	 * directly from the controller, but from the frontend (by sending a
	 * zero-length event). So in order to stop a receiver thread, two stop
	 * requests should have been received (except the first receiver
	 * thread): one is from the frontend, the other is from the receiver
	 * thread chains (in the order 1->11->21->31->...). */
	int n_stops;
};



#endif 
