#include "log_thread.h"

log_thread::log_thread()
{
	rb_trig = NULL;
	rb_scal = NULL;
	init_fun.push_back(&log_thread_init);
}

log_thread::~log_thread()
{
	/* Be careful! Do NOT do the following freeing memories. Because each
	 * object of the subclass will do the following things, meaning the
	 * memories will be freed multiple times! */
/*
	if (rb_scal)
		delete rb_scal;
	if (rb_evt)
		delete rb_evt;
	if (rb_fe)
		delete rb_fe;
	for (auto it = rb_data.begin(); it != rb_data.end(); it++) {
		if (*it)
			delete (*it);
	}
*/
	
}
int log_thread::log_thread_init(my_thread* This, initzer* the_initzer)
{
	log_thread* ptr = reinterpret_cast<log_thread*>(This);

	/* initialize the ring buffers */
	ptr->rb_trig = the_initzer->get_log_rb(0);
	if (!ptr->rb_trig)
		return -E_RING_BUF_DATA;
	ptr->rb_msg = the_initzer->get_log_rb(1);
	if (!ptr->rb_msg)
		return -E_RING_BUF_MSG;
	ptr->rb_scal = the_initzer->get_log_rb(2);
	if (!ptr->rb_scal)
		return -E_RING_BUF_DATA;
	return 0;
}
