#include "fe_thread.h"
#include "err_code.h"
#include <unistd.h>
#include <string.h>


fe_thread::fe_thread()
{
	rb_data = NULL;
	init_fun.push_back(&fe_thread_init);
}

fe_thread::~fe_thread()
{
	/* Caution: Do NOT delete rb_data or rb_msg. If you do it, every
	 * instance of the (sub)class will delete the rb_data or rb_msg, which
	 * will cause double-free fault. */
	// delete rb_data;
	// delete rb_msg;
}

int fe_thread::fe_thread_init(my_thread* This, initzer* the_initzer)
{
	fe_thread* ptr = static_cast<fe_thread*>(This);
	/* initialize the ring buffers */
	ptr->rb_data = the_initzer->get_fe_rb(1);
	if (!ptr->rb_data)
		return -E_RING_BUF_DATA;
	ptr->rb_msg = the_initzer->get_fe_rb(2);
	if (!ptr->rb_msg)
		return -E_RING_BUF_MSG;

	return 0;
}

