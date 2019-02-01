#include "ana_thread.h"

ana_thread::ana_thread()
{
	rb_evt = NULL;
	init_fun.push_back(&ana_thread_init);
}

ana_thread::~ana_thread()
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
int ana_thread::ana_thread_init(my_thread* This, initzer* the_initzer)
{
	ana_thread* ptr = reinterpret_cast<ana_thread*>(This);

	/* initialize the ring buffers */
	ptr->rb_evt = the_initzer->get_ana_rb(0);
	ptr->rb_msg = the_initzer->get_ana_rb(1);
	ptr->rb_scal = the_initzer->get_ana_rb(2);
	return 0;
}
