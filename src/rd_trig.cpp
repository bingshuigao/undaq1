#include "rd_trig.h"
#include "err_code.h"
#include <unistd.h>
#include <string.h>
#include <time.h>


rd_trig::rd_trig()
{
	trig_mod = NULL;
	thread_id = 1;
}

rd_trig::~rd_trig()
{
}


int rd_trig::handle_msg(uint32_t* msg_body)
{
	/* The message type of the current thread are defined as following:
	 * msg_type == 1 --> run status transition. 
	 * msg_type == 2 --> readout scaler modules */
	uint32_t msg_type = msg_body[0] & 0xFFFFFF;
	uint32_t stat[4];
	struct timespec ts;
	uint64_t cur_t; /* current time, in the unit of ms */

	switch (msg_type) {
	case 1:
		/* run status transition */
		return switch_run(msg_body[1]);
	case 2:
		/* readout scaler modules */
		return do_rd_mods(reinterpret_cast<modules**>(msg_body + 1)[0]);
	case 3:
		/* a dummy message */
		return 0;
	case 4:
		/* (the ctrl thread) queries the statistics */
		clock_gettime(CLOCK_MONOTONIC_COARSE, &ts);
		cur_t = ts.tv_sec * 1000L + ts.tv_nsec/1000000;
		stat[0] = n_byte >> 32;        /* high word of n_byte */
		stat[1] = n_byte & 0xffffffff; /* low word of n_byte */
		stat[2] = cur_t >> 32;         /* high word of time */
		stat[3] = cur_t & 0xffffffff;  /* low word of time */
//		std::cout<<"sending rates from rd_trig..."<<std::endl;
		return send_msg(4, 4, stat, 16);
	case 5: 
		/* data stream pipe line is broken */
		is_pipe_broken = true;
		return send_text_mes("broken pipe line, stop writing to ring buffer!", 
				MSG_LEV_FATAL);
	default:
		return -E_MSG_TYPE;
	}
}


int rd_trig::my_init(initzer* the_initzer)
{
	int ret;

	/* initialize the trigger module */
	trig_mod = the_initzer->get_trig_mod();
	if (!trig_mod)
		return -E_TRIG_MOD;
	the_ctl = trig_mod->get_ctl();

	/* the n_try */
	n_try = the_initzer->get_fe_ntry();

	return 0;

}

int rd_trig::try_rd_fe(bool force_rd)
{
	bool trig;
	int ret, head_len;
	uint32_t p_head[100];
	
	ret = query_trig(trig);
	RET_IF_NONZERO(ret);
	if (trig | force_rd) {
		for (auto it = mods.begin(); it != mods.end(); it++) {
			ret = do_rd_mods(*it);
			RET_IF_NONZERO(ret);
		}
		/* send a pulse for whatever use (e.g. reset the busy) */
		for (auto it = mods.begin(); it != mods.end(); it++) {
			ret = (*it)->get_1st_mod()->get_ctl()->send_pulse2(false);
			RET_IF_NONZERO(ret);
			break;
		}
		/* after reading all the trigger modules, do not forget add the
		 * End Of Reading mark (EOR). This very useful for the event
		 * builder. See comments in ebd_thread.h */
		do_rd_mods(NULL);
	}

	return 0;
}

