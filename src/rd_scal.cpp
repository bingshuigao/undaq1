#include "rd_scal.h"
#include "err_code.h"
#include <unistd.h>
#include <string.h>
#include <iostream>


rd_scal::rd_scal()
{
	thread_id = 2;
}

rd_scal::~rd_scal()
{
	for (auto it = mods.begin(); it != mods.end(); it++) {
		my_timer* t = (*it)->get_1st_mod()->get_timer();
		if (t) delete t;
	}
}


int rd_scal::handle_msg(uint32_t* msg_body)
{
	/* The message type of the current thread are defined as following:
	 * msg_type == 1 --> run status transition. 
	 * msg_type == 2 --> to be defined yet */
	uint32_t msg_type = msg_body[0] & 0xFFFFFF;

	switch (msg_type) {
	case 1:
		/* run status transition */
		return switch_run(msg_body[1]);
	default:
		return -E_MSG_TYPE;
	}
}


int rd_scal::my_init(initzer* the_initzer)
{
	int ret;
	
	/* we need to initialize the timers for each scaler-type module (only
	 * the first scaler-type module need the timer in a modules object); */
	for (auto it = mods.begin(); it != mods.end(); it++) {
		module* a_mod = (*it)->get_1st_mod();
		a_mod->set_timer(new my_timer);
		a_mod->reset_timer();
	}

	return 0;

}

int rd_scal::try_rd_fe(bool force_rd)
{
	int ret;
	/* see if any of the scaler-type modules need read (time out)*/
	for (auto it = mods.begin(); it != mods.end(); it++) {
		module* a_mod = (*it)->get_1st_mod();
		if (a_mod->time_out() | force_rd) {
			modules* p_mods = *it;
			a_mod->reset_timer();
			/* Careful! Don't read the scaler modules here which
			 * will introduce concurrent conditions. We send a
			 * message to the trigger thread where the actual
			 * readout will be done */
			ret = send_msg(1, 2, &p_mods, sizeof(p_mods));
			/* debug...*/
//			std::cout<<"reading scaler!"<<std::endl;

			RET_IF_NONZERO(ret);
		}
	}

	/* take a short break before returnning */
	usleep(100000);

	return 0;
}

