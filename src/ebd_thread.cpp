#include "ebd_thread.h"

ebd_thread::ebd_thread()
{
	rb_evt = NULL;
	rb_fe = NULL;
	rb_msg = NULL;
	rb_scal = NULL;
	init_fun.push_back(&ebd_thread_init);
}

ebd_thread::~ebd_thread()
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
int ebd_thread::ebd_thread_init(my_thread* This, initzer* the_initzer)
{
	ebd_thread* ptr = reinterpret_cast<ebd_thread*>(This);

	/* initialize the ring buffers */
	ptr->rb_data = the_initzer->get_ebd_rbs();
	if (ptr->rb_data.size() == 0)
		return -E_RING_BUF_DATA;
	for (auto it = ptr->rb_data.begin(); it != ptr->rb_data.end(); it++) {
		if ((*it) == NULL)
			return -E_RING_BUF_DATA;
	}
	ptr->rb_fe = the_initzer->get_ebd_rb(1);
	if (!ptr->rb_fe)
		return -E_RING_BUF_DATA;
	ptr->rb_msg = the_initzer->get_ebd_rb(2);
	if (!ptr->rb_msg)
		return -E_RING_BUF_MSG;
	ptr->rb_evt = the_initzer->get_ebd_rb(3);
	if (!ptr->rb_evt)
		return -E_RING_BUF_DATA;
	ptr->rb_scal = the_initzer->get_ebd_rb(5);
	if (!ptr->rb_scal)
		return -E_RING_BUF_DATA;


	return 0;
}
