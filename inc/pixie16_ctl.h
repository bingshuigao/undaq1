/* This class represents the pixie16 controller ,which unlike vme controllers,
 * does not physically exists. We also derive this class from vme_ctl class
 * though it sounds weird. But their lowest level principle of operations are
 * similar: open, read, write, close.
 * By B.Gao Nov. 2021 */

#ifndef PIXIE16_CTL_HHH
#define PIXIE16_CTL_HHH
#include "vme_ctl.h"
#include <stdint.h>
#include "../os_libs/pixie16/pixie16.h"

struct pixie16_ctl_open_par
{
	unsigned short mod_num;        /* total number of modules */
	unsigned short* pxi_slot_map;  /* an array containing the slot number
					  of each Pixie-16 module */
	const char *ComFPGAConfigFile; /* name of ComFPGA configuration file */
	const char *SPFPGAConfigFile;  /* name of SPFPGA configuration file */ 
	const char *TrigFPGAConfigFile;/* name of trigger FPGA file */         
	const char *DSPCodeFile;       /* name of DSP code file */             
	const char *DSPParFile;        /* name of DSP parameter file */        
	const char *DSPVarFile;        /* name of DSP variable names file */   
};

class pixie16_ctl : public vme_ctl
{
public:
	pixie16_ctl(); 
	~pixie16_ctl() {};

	void set_max_evt_sz(unsigned int sz) {evt_max_sz = sz;}
	/* open the device 
	 * @par a pointer of parameters 
	 * @return 0 if succeed, non-zero error codes are returned if error */
	int open(void* par);

	/* close the device 
	 * @return 0 if succeed, non-zero error codes are returned if error */
	int close();

	/* read/write controller register. Since the pixie16 controller does
	 * not physically exist at all, here the register is just regarded as
	 * parameters with address as their identifier */
	int read(unsigned long addr, void* data);
	int write(unsigned long addr, void* data);

	/* in the case of pixie16, reg is the parameter offset, am is the
	 * module number, dw is the channel number. */
	int read_reg(long reg, void* data);
	int write_reg(long reg, void* data);

	/* read event data from pixie16 modules. We use the
	 * Pixie16ReadDataFromExternalFIFO function. Since this function is not
	 * event-aligned readout, we check if the data read out truncated the
	 * last event and, if necessary, do a second read out to restore the
	 * 'event-aligned' feature.
	 * @param addr module number
	 * @param buf buffer where the data will be read out to
	 * @param sz_in size of data to read (in bytes)
	 * @param sz_out size of data actually read (in bytes).
	 * @return 0 if succeed, non-zero error codes in case of error. */
	int blt_read(unsigned long addr, void* buf, int sz_in, int* sz_out);
	int mblt_read(unsigned long addr, void* buf, int sz_in, int* sz_out);
	
	/* the out port 1 (the second port) will be used */
	virtual int send_pulse(bool invt);
	/* the out port 0 (the first port) will be used */
	virtual int send_pulse1(bool invt);
	/* the out port 2 (the third port) will be used */
	virtual int send_pulse2(bool invt);

	void set_fifo_thresh(uint32_t thresh) {fifo_rd_thresh = thresh;}
	uint32_t get_fifo_thresh() {return fifo_rd_thresh;}

private:
	/* check if the events in the buf are event aligned.
	 * @param buf buffer containing the events
	 * @param n_word length of the events in the buffer (number of 32-bit
	 * words)
	 * @return the number of remaining number of words should be readout to
	 * make it event-aligned.
	 * */
	unsigned int check_evt_align(uint32_t* buf, unsigned int n_word);

	/* set the module CSRB register bit 
	 * @param mod_n module number
	 * @param bit_n bit number to set
	 * @param val bit value to set 
	 * @return 0 if succeed, otherwise return error code. 
	 * */
	int set_modCSRB_bit(unsigned short mod_n, uint16_t bit_n, uint16_t val);

	/* set module/channel register bit 
	 * @param mod_n module number
	 * @param ch_n channel number
	 * @param par_name parameter name to set
	 * @param start_bit starting bit number to set
	 * @param n_bit number of bit to set
	 * @param bit_val new bit value to set 
	 * @return 0 if succeed, otherwise return error code. 
	 * */
	int set_modreg_bit(unsigned short mod_n, const char* par_name, uint16_t
			start_bit, uint16_t n_bit, uint32_t bit_val);
	int set_chreg_bit(unsigned short mod_n, unsigned short ch_n, const
			char* par_name, uint16_t start_bit, uint16_t n_bit,
			uint32_t bit_val);

	/* set the channel CSRA register bit 
	 * @param mod_n module number
	 * @param ch_n channel number
	 * @param bit_n bit number to set
	 * @param val bit value to set 
	 * @return 0 if succeed, otherwise return error code. 
	 * */
	int set_chCSRA_bit(unsigned short mod_n, unsigned short ch_n, uint16_t
			bit_n, uint16_t val);

	/* write the filter parameters into dsp. because those parameters are
	 * not independent, they have to be written in some certain order,
	 * therefore, in this function we collect all those parameters and once
	 * they are all there written into dsp in some certain order.
	 * @param mod_n the module number 
	 * @param ch_n  the channel number
	 * @return 0 if succeed, otherwise return error code. */
	int do_conf_filter_pars(uint16_t mod_n, uint16_t ch_n);

	/* similarly as do_conf_filter_pars, here we config the baseline cut */
	int do_conf_blcut(uint16_t mod_n, uint16_t ch_n);

	/* similarly as do_conf_filter_pars, here we config the group trigger
	 * selections (TrigConfig1 bit0-31, TrigConfig2 bit0-23) for group
	 * 'grp_n' of module 'mod_n'*/
	int do_conf_group_trig(int grp_n, uint16_t mod_n, uint16_t ch_n);

	double set_bits_double(double val, uint16_t start_bit, uint16_t n_bit,
			uint32_t bit_val)
	{
		uint32_t val32 = (uint32_t)val;
		return (double)set_bits_int(val32, start_bit, n_bit, bit_val);
	}

	uint32_t set_bits_int(uint32_t val32, uint16_t start_bit, uint16_t n_bit,
			uint32_t bit_val)
	{
		uint32_t tmp32 = ((1L<<n_bit) - 1) << start_bit;
		val32 &= ~tmp32;
		val32 |= bit_val << start_bit;
		return val32;
	}

private:
	/* total number of pixie16 modules installed in the system */
	unsigned short mod_num;
	/* max size of single event (number of 32-bit words) */
	unsigned int evt_max_sz;
	
	/* below are a list of dsp parameters.  We store those parameters
	 * because their values are not independent, so they have to be written
	 * into the pixie16 module in some certain order.*/
	int    T_Filter_Range[MAX_CRATE][16];
	int    E_Filter_Range[MAX_CRATE][16];
	int    auto_blcut[MAX_CRATE][16];
	int    group0_trigLMR[MAX_CRATE][16];
	int    group1_trigLMR[MAX_CRATE][16];
	int    group2_trigLMR[MAX_CRATE][16];
	int    group3_trigLMR[MAX_CRATE][16];
	int    group0_trigCh[MAX_CRATE][16];
	int    group1_trigCh[MAX_CRATE][16];
	int    group2_trigCh[MAX_CRATE][16];
	int    group3_trigCh[MAX_CRATE][16];
	double trace_length[MAX_CRATE][16];
	double trace_delay[MAX_CRATE][16];
	double T_Risetime_us[MAX_CRATE][16];
	double T_FlatTop_us[MAX_CRATE][16];
	double T_Threshold[MAX_CRATE][16];
	double E_FlatTop_us[MAX_CRATE][16];
	double E_Risetime_us[MAX_CRATE][16];
	double bl_cut[MAX_CRATE][16];

	/* fifo threshold (see comments in pixie16.cpp). This parameter is not
	 * used by the controller, but by the trigger module. The varable here
	 * is just used to pass the value to the trigger module because this
	 * varable is defined as a parameter of the controller. */
	uint32_t fifo_rd_thresh;

	/* if reset the clock for new runs */
	uint32_t reset_clk;


};
#endif

