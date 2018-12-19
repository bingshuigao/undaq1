#include "rd_trig.h"
#include "err_code.h"
#include <unistd.h>
#include <string.h>


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


int rd_trig::my_init(initzer* the_initzer)
{
	int ret;

	/* initialize the trigger module */
	trig_mod = the_initzer->get_trig_mod();
	if (!trig_mod)
		return -E_TRIG_MOD;

	/* the n_try */
	n_try = the_initzer->get_fe_ntry();

	return 0;

}

int rd_trig::try_rd_fe()
{
	bool trig;
	int ret, head_len;
	uint32_t p_head[100];
	
	ret = query_trig(trig);
	RET_IF_NONZERO(ret);
	if (trig) {
		for (auto it = mods.begin(); it != mods.end(); it++) {
			ret = do_rd_mods(*it);
			RET_IF_NONZERO(ret);
		}
		/* after reading all the trigger modules, do not forget add the
		 * End Of Reading mark (EOR). This very useful for the event
		 * builder. See comments in ebd_thread.h */
		do_rd_mods(NULL);
	}

	return 0;
}

