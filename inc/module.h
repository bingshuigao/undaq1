/* The class module represents the base class of vme modules.  After writing a
 * few classes for different vme modules, I realized that it might be a good
 * idea to have a base class for all vme modules.
 * By B.Gao Aug. 2018 */

#ifndef MODULE_HHH
#define MODULE_HHH

#include <stdint.h>
#include "vme_ctl.h"
#include "err_code.h"
#include <string>
#include "my_timer.h"
#include "imp_daq.h"

class module
{
public:
	module() 
	{
		trig_mod = false; 
		ctl = NULL;
		type = 'T';
		timer = NULL;
		geo = -1;
	}
	~module() {};

	/* Initialization routines which must be called before using the object
	 * */
	void set_ctl(vme_ctl* ctl) {this->ctl = ctl;}
	vme_ctl* get_ctl() {return ctl;}
	void set_slot(uint32_t n) {slot_n = n;}
	void set_crate(uint32_t n) {crate_n = n;}
	void set_base(uint32_t base) {base_addr = base;}
	uint32_t get_slot() {return slot_n;}
	uint32_t get_crate() {return crate_n;}
	uint32_t get_base() {return base_addr;}
	uint32_t get_mod_id() {return mod_id;}
	void set_am(int am) {am_reg = am;}
	int get_am() {return am_reg;}
	void set_period(int t) {period = t;}
	uint32_t get_period() {return period;}
	void set_type(char t) {type = t;}
	char get_type() {return type;}
	std::string get_name() {return name;}
	bool get_trig_mod() {return trig_mod;}
	void set_trig_mod(bool yesno) {trig_mod = yesno;}
	void set_timer(my_timer* t) {timer = t;}
	my_timer* get_timer() {return timer;}
	void reset_timer() {timer->reset();}



	/* General purpose read of register. 
	 * @param addr vme bus address
	 * @param dw vme data width, can be 8, 16, 32...
	 * @param out buffer where the result is saved
	 * @return 0 if succeed, nonzero error codes if error  */
	int read_reg(uint32_t addr, int dw, void* out)
	{
		ctl->set_am(am_reg);
		ctl->set_dw(dw);
		return ctl->read(addr + base_addr, out);
	}
	
	/* General purpose write of register. 
	 * @param addr vme bus address
	 * @param dw vme data width, can be 8, 16, 32...
	 * @param val pointer to value to write
	 * @return 0 if succeed, nonzero error codes if error. 
	 * Note: it is important to make sure the data width should be
	 * compatible with the content pointed by void* val. For example, if
	 * data width=16, the data pointed by void* val should have content of
	 * uint16_t types.*/
	virtual int write_reg(uint32_t addr, int dw, void* val)
	{
		ctl->set_am(am_reg);
		ctl->set_dw(dw);
		return ctl->write(base_addr + addr, val);
	}

	/* Read single event word-by-word from the event buffer 
	 * @param am vme address modifier
	 * @param evt Pointer to the buffer where the word will be saved
	 * @param sz_out number of bytes read.
	 * @return 0 if succeed, nonzero error codes if error. */
	virtual int read_single_evt(int am, uint32_t *evt, int* sz_out) = 0;

	/* get the GEO
	 * @return return the geo value */
	virtual int get_geo() = 0;

	/* Read the event buffer using (c)(m)blt mode.
	 * @param am vme address modifier
	 * @param dst Pointer to the output buffer
	 * @param sz_in requested number of bytes to read
	 * @param sz_out actual number of bytes read out.
	 * @param blt_addr blt address if use c(m)blt read out
	 * @param chain if true, use c(m)blt, otherwise use (m)blt
	 * @param return 0 if succeed, nonzero error codes if error.*/
	int read_evt_blt(int am, void* dst, int sz_in, int* sz_out, 
			uint32_t blt_addr, int chain)
	{
		uint32_t addr = base_addr + buf_off;
		ctl->set_am(am);
		ctl->set_dw(32);
		if (chain)
			addr = blt_addr;
		return ctl->blt_read(addr, dst, sz_in, sz_out);
	}
	int read_evt_mblt(int am, void* dst, int sz_in, int* sz_out, 
			uint32_t blt_addr, int chain)
	{
		uint32_t addr = base_addr + buf_off;
		ctl->set_am(am);
		ctl->set_dw(64);
		if (chain)
			addr = blt_addr;
		return ctl->mblt_read(addr, dst, sz_in, sz_out);
	}

	/* (only usefull if is scaler-type module) 
	 * return true if the timer is out since last reset, otherwise return
	 * false*/
	bool time_out() { return timer->time_out(1000UL * period); }

	/* Enable MCST.
	 * @param mcst_addr The mcst_addr to be used. If zero, use the default
	 * address 
	 * @return 0 if succeed, nonzero error codes if error. */
	virtual int enable_mcst(uint32_t mcst_addr = 0) = 0;

	/* Enable CBLT
	 * @param cblt_addr The address to be used for CBLT readout.
	 * @param first if true, set the module to be the first one in the cblt
	 * chain
	 * @param last  if true, set the module to be the last  one in the cblt
	 * chain
	 * @return 0 if succeed, nonzero error codes if error.*/
	virtual int enable_cblt(uint32_t cblt_addr, int first, int last) = 0;

	/* Get the configurations of cblt registers
	 * @param addr [out] cblt address
	 * @param cblt_enable [out] =1 if cblt is enabled, else = 0; 
	 * @param cblt_first  [out] =1 if set to the first in cblt chain
	 * @param cblt_last   [out] =1 if set to the last in cblt chain
	 * @return 0 if succeed, nonzero error codes if error. 
	 * Note: if any of the parameters are NULL, the corresponding
	 * information won't be returned */
	virtual int get_cblt_conf(uint16_t* addr, int* cblt_enable, 
			int* cblt_first, int* cblt_last) = 0;


	/* See if DAQ triggers (only makes sense for trigger module).
	 * return 0 if succeed, otherwise return error code.*/
	virtual int if_trig(bool& x) {x = false; return 0;}

	/* prepare to start/stop. This should be called at the begin/end of
	 * each run. 
	 * return 0 if succeed, otherwise return error code.*/
	virtual int on_start() {return 0;}
	virtual int on_stop() {return 0;}
	virtual int on_readout() {return 0;}

protected:
	/* base address of the module */
	uint32_t base_addr;

	/* slot number of the module, starting from 0 */
	uint32_t slot_n;

	/* crate number of the module , starting from 0*/
	uint32_t crate_n;

	/* the offset of event buffer */
	uint32_t buf_off;

	/* vme address modifier to be used for normal register access (for data
	 * transfer, the address modifiers are taken from function parameters)
	 * */
	int am_reg;

	/* the GEO */
	int32_t geo;

	/* module id 
	 * madc32 --> mod_id = 1;
	 * v1190  --> mod_id = 2;
	 * v830   --> mod_id = 3;
	 * */
	uint32_t mod_id;
	

	/* vme controller */
	vme_ctl* ctl;

	/* Period (in ms), only makes sense for Scaler-type modules. If the
	 * module type is Scaler-type, but the period is 0, then the module
	 * will not be readout in trigger event or scaler event (e.g. if the
	 * module is a trigger module (v977)).  */
	uint32_t period;
	
	/* type of the module T/S */
	char type;

	/* name of the module */
	std::string name;

	/* if the module is a trigger module */
	bool trig_mod;

	/* if the module is a scaler type module */
	my_timer* timer;
};

#endif
