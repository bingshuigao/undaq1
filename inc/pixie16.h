/* The class pixie16 represents one channel of Pixie16 module
 * By B.Gao Nov. 2021 */

#ifndef PIXIE16_HHH
#define PIXIE16_HHH

#include <stdint.h>
#include "vme_ctl.h"
#include "err_code.h"
#include "module.h"
#include "../os_libs/pixie16/pixie16.h"

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

	/* In the concept of VME DAQ, there must exist one 'trigger-module'
	 * which the DAQ framework uses to check if trigger arrives. However,
	 * in the pixie world, this is not applicable. How do we know if there
	 * is trigger then? We use the Pixie16CheckExternalFIFOStatus function
	 * to check each pixie module installed to see if data available in
	 * their FIFO to determine if trigger arrives. So we need to know how
	 * many modules are installed. This function will be called by initzer
	 * and tell us the number of installed pixie modules. */
	void set_total_mod(uint16_t n) {tot_mod_num = n;}

	void set_mod_num(uint16_t n) {this_mod_num = n;}
	
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

private:
	/* total number of installed pixie modules */
	uint16_t tot_mod_num;

	/* the module number of the pixie16 module to which the channel
	 * belongs. This module number is used in almost all the pixie16 api
	 * functions  */
	uint16_t this_mod_num;

	
};

#endif
