#ifndef EBD_MERGE_HHH
#define EBD_MERGE_HHH

/* This is the merger thread of the event builder. In this thread, we read the
 * fragments of events from the individual ring buffers of each vme module and
 * merge these fragments into one event, based on their timestamps. 
 * The merging is tricky, see comments in ebd_thread.h.
 *
 * By B.Gao Dec.2018
 * 
 * */

#include "ebd_thread.h"


class ebd_merge : public ebd_thread
{
public:
	ebd_merge();
	~ebd_merge();

private:
	static int ebd_merge_init(my_thread* This, initzer* the_initzer);

	/* implementation of virtual functions declared in base classes. See
	 * comments in base classes for explainations */
	int handle_msg(uint32_t* msg_body);
	int start();
	int stop();
	int quit();
	int main_proc();

	/* see if it is OK to start the building process. (see comments in
	 * ebd_thread.h) 
	 * if yes, y_n = true, if no y_n = false;
	 * return 0 if succeed, otherwise return error code.
	 * */
	int can_build(bool& y_n);

private:
	/* for the meaning of this variables, refer to "ebd_thread.h" */
	int cur_rd;

};


#endif
