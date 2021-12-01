/* The class pixie16 represents one channel of Pixie16 module
 * By B.Gao Nov. 2021 */

#ifndef PIXIE16_HHH
#define PIXIE16_HHH

#include <stdint.h>
#include "vme_ctl.h"
#include "err_code.h"
#include "module.h"
//#include "../os_libs/pixie16/pixie16.h"

class pixie16: public module
{
public:
	pixie16();
	~pixie16(){};


	/* will be called at end of each readout */
	virtual int on_readout() 
	{
		return 0;
	}


	/* See if DAQ triggers (only makes sence for trigger module)
	 * return 0 if succeed, otherwise return error code.
	 * */
	virtual int if_trig(bool& x);

	/* see comments in module.h */
	virtual int on_start();
	virtual int on_stop();

	/* set slot/crate ID (which is reported in its data structure) */
	int set_dsp_slot_id(int slot_id)
	{
		uint32_t off = 1008;
		uint64_t val = slot_id;
		return write_reg(off, 0, &val);
	}
	int set_dsp_crate_id(int crate_id)
	{
		uint32_t off = 1009;
		uint64_t val = crate_id;
		return write_reg(off, 0, &val);
	}


	void set_mod_num(uint16_t n) 
	{
		mod_num = n; 
		/* am is used as modules number by the controller */
		set_am(n);
	}

	void set_fifo_thresh(uint32_t thresh) {fifo_rd_thresh = thresh;}

	uint16_t get_mod_num() {return mod_num;}


	/* return true if a pixie16 config parameter with offset = off is a
	 * module parameter, otherwise return false. */
	int is_mod_par(int off)
	{
		if ((off >= 58) && (off <= 65)) 
			return 1;
		if (off <= 6) 
			return 1;
		else if (off <= 39)
			return 0;
		else if (off <= 41)
			return 1;
		else if (off <= 71)
			return 0;
		else
			return 1;
	}
	
	/* get the GEO of the module. 
	 * @return return the GEO */
	int get_geo()
	{
		return -E_DONT_CALL;
	}

	/* do not call */
	int read_single_evt(int am, uint32_t *evt, int* sz_out);

	/* do not call */
	int enable_mcst(uint32_t mcst_addr = 0);

	/* do not call */
	int enable_cblt(uint32_t cblt_addr, int first, int last);

	int get_cblt_conf(uint16_t* addr, int* cblt_enable, int* cblt_first,
			int* cblt_last);

	/* read/write pixie16 registers. Note that those are not real
	 * registers, but just configuration parameters. The offsets are labels
	 * (id) of those parameters. Some of those parameters are channel-wide
	 * and some are module-wide. For module-wide parameters, we only do the
	 * write for the last channel (slot_n == 15)
	 * @param off offset (lable or id) of the parameter 
	 * @val value of the parameter 
	 * @ret 0 if succeed, otherwise return error code.
	 * */
//	int write_pixie_reg(uint32_t off, uint64_t val)
//	{
//		return ctl->write_pixie_reg
//	}
//	int read_pixie_reg(uint32_t off, uint64_t* val);

private:

	/* the module number of the pixie16 module to which the channel
	 * belongs. This module number is used in almost all the pixie16 api
	 * functions  */
	uint16_t mod_num;

	/* fifo threshold to determine if a read should be performed. (the
	 * threshold is for all the modules, i.e. the total number of words of
	 * all pixie16 modules installed in the system )*/
	uint32_t fifo_rd_thresh;


	
};

#endif
