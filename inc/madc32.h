/* The class madc32 represents the mesytec madc32
 * By B.Gao Jul. 2018 */

#ifndef MADC32_HHH
#define MADC32_HHH

#include <stdint.h>
#include "vme_ctl.h"
#include "err_code.h"
#include "module.h"

class madc32: public module
{
public:
	madc32();
	~madc32(){};

	/* Used for testing purposes, when the test mode is set, all
	 * corresponding read and write of registers will not really
	 * communicate with the hardware 
	 * @param test 1: set test mode; 0 unset test mode. */
	/* should be moved to madc32s 
	void set_test(int test); */

	/* will be called at end of each readout */
	virtual int on_readout() 
	{
		uint16_t dum = 0; 
		return write_reg(0x6034, 16, &dum);
	}

	/* see the comments in base class */
	int write_reg(uint32_t addr, int dw, void* val)
	{
		if (addr == 0x6004) 
			geo = reinterpret_cast<uint16_t*>(val)[0];
		return module::write_reg(addr, dw, val);
	}

	/* See if DAQ triggers (only makes sence for trigger module)
	 * return 0 if succeed, otherwise return error code.
	 * */
	virtual int if_trig(bool& x);

	/* see comments in module.h */
	virtual int on_start();

	/* get the GEO of the module. 
	 * @return return the GEO */
	int get_geo()
	{
		if (geo == 0xFF || geo == -1)
			return base_addr >> 24;
		else
			return geo;
	}

	/* Read single event word-by-word from the event buffer 
	 * @param am vme address modifier
	 * @param evt Pointer to the buffer where the word will be saved
	 * @param sz_out number of bytes read.
	 * @return 0 if succeed, nonzero error codes if error. */
	int read_single_evt(int am, uint32_t *evt, int* sz_out);

	/* Enable MCST.
	 * @param mcst_addr The mcst_addr to be used. If zero, use the default
	 * address (0xBB)
	 * @return 0 if succeed, nonzero error codes if error. */
	int enable_mcst(uint32_t mcst_addr = 0);

	/* Enable CBLT
	 * @param cblt_addr The address to be used for CBLT readout.
	 * @param first if true, set the module to be the first one in the cblt
	 * chain
	 * @param last  if true, set the module to be the last  one in the cblt
	 * chain
	 * @return 0 if succeed, nonzero error codes if error.*/
	int enable_cblt(uint32_t cblt_addr, int first, int last);

	/* Get the configurations of cblt registers
	 * @param addr [out] cblt address
	 * @param cblt_enable [out] =1 if cblt is enabled, else = 0; 
	 * @param cblt_first  [out] =1 if set to the first in cblt chain
	 * @param cblt_last   [out] =1 if set to the last in cblt chain
	 * @return 0 if succeed, nonzero error codes if error. 
	 * Note: if any of the parameters are NULL, the corresponding
	 * information won't be returned */
	int get_cblt_conf(uint16_t* addr, int* cblt_enable, int* cblt_first,
			int* cblt_last);

};

#endif
