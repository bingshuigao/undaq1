/* The class fake_module represents a fake vme module (trigger-type). It is
 * useful when there is no real trigger-type modules in the system.
 * By B.Gao Jun. 2020 */

#ifndef FAKE_MODULE_HHH
#define FAKE_MODULE_HHH

#include <stdint.h>
#include "vme_ctl.h"
#include "err_code.h"
#include "module.h"

class fake_module: public module
{
public:
	fake_module();
	~fake_module(){};


	/* will be called at end of each readout */
	virtual int on_readout() {return 0;}

	/* see the comments in base class */
	int write_reg(uint32_t addr, int dw, void* val) {return 0;}

	/* See if DAQ triggers (only makes sence for trigger module)
	 * return 0 if succeed, otherwise return error code.
	 * */
	virtual int if_trig(bool& x) {x = false; return 0;}

	/* see comments in module.h */
	virtual int on_start() {return 0;}
	virtual int on_stop() {return 0;}

	/* get the GEO of the module. 
	 * @return return the GEO */
	int get_geo() {return 0;}

	/* Read single event word-by-word from the event buffer 
	 * @param am vme address modifier
	 * @param evt Pointer to the buffer where the word will be saved
	 * @param sz_out number of bytes read.
	 * @return 0 if succeed, nonzero error codes if error. */
	int read_single_evt(int am, uint32_t *evt, int* sz_out)
	{
		/* this shouldn't happen... */
		return -E_VME_GENERIC;
	}

	/* Enable MCST.
	 * @param mcst_addr The mcst_addr to be used. If zero, use the default
	 * address (0xBB)
	 * @return 0 if succeed, nonzero error codes if error. */
	int enable_mcst(uint32_t mcst_addr = 0) {return 0;}

	/* Enable CBLT
	 * @param cblt_addr The address to be used for CBLT readout.
	 * @param first if true, set the module to be the first one in the cblt
	 * chain
	 * @param last  if true, set the module to be the last  one in the cblt
	 * chain
	 * @return 0 if succeed, nonzero error codes if error.*/
	int enable_cblt(uint32_t cblt_addr, int first, int last){return 0;}

	/* Get the configurations of cblt registers
	 * @param addr [out] cblt address
	 * @param cblt_enable [out] =1 if cblt is enabled, else = 0; 
	 * @param cblt_first  [out] =1 if set to the first in cblt chain
	 * @param cblt_last   [out] =1 if set to the last in cblt chain
	 * @return 0 if succeed, nonzero error codes if error. 
	 * Note: if any of the parameters are NULL, the corresponding
	 * information won't be returned */
	int get_cblt_conf(uint16_t* addr, int* cblt_enable, int* cblt_first,
			int* cblt_last) 
	{
		if (cblt_enable)
			*cblt_enable = 0; 
		if (cblt_first)
			*cblt_first = 0;
		if (cblt_last)
			*cblt_last = 0;
		return 0;
	}

};

#endif
