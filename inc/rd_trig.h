/* This class is the thread of the frontend which reads the trigger event. 
 *
 *
 * By B.Gao Oct. 2018 */

#ifndef RD_TIRG_HHH
#define RD_TIRG_HHH

#include "initzer.h"
#include <vector>
#include "module.h"
#include <pthread.h>
#include "my_timer.h"
#include "rd_fe.h"

class rd_trig : public rd_fe
{
public:
	rd_trig();
	~rd_trig();



private:

	/* query trigger, return zero if succeed, otherwise return error code.*/
	int query_trig(bool& x)
	{
		int i, ret;
		for (i = 0; i < n_try; i++) {
			ret = trig_mod->if_trig(x);
			RET_IF_NONZERO(ret);
			if (x)
				break;
		}
		return 0;
	}



	/* handle the message. The pointer points to the message body (not
	 * including the message header).
	 * return 0 if succeed, otherwise return error code */
	virtual int handle_msg(uint32_t* msg_body);

	/* partially initialize. Initialize only the special variables, the
	 * common initialization are done in the function init(). should be
	 * called by the implementations of the init function in the
	 * baseclasses. 
	 * Return 0 if succeed, return error code in case of error */
	virtual int my_init(initzer* the_initzer);

	/* try to read the frontend, it is called in the mainloop if the daq is
	 * in the running status.
	 * In this function, we should first check if we need to readout the
	 * data. (for trigger-type modules, check trigger condition, for
	 * scaler-type module, check timer). If we do need to readout data from
	 * modules, then we read and handle the data.
	 * Return 0 if succeed, otherwise return error
	 * code. */
	virtual int try_rd_fe(bool force_rd = false);



private:

	/* the module which tells us when a trigger happens. */
	module* trig_mod;


	/* max query time when checking if trigger arrives */
	int n_try;

};



#endif 
