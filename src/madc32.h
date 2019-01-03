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

	/* The following functions privide read/write of specific registers,
	 * their meanings are self-explained by the names, when in daubt,
	 * please check the definitions. The reture values indicate error if
	 * nonzero, otherwise return zero.*/
	int set_threshold(int channel,  uint16_t threshold);
	int get_threshold(int channel,  uint16_t* threshold);
	int set_module_id( uint16_t id);
	int get_module_id( uint16_t* id);
	int set_soft_reset();
	int get_fw_version( uint16_t* version);
	int set_irq_level( uint16_t level);
	int get_irq_level( uint16_t* level);
	int set_irq_vector( uint16_t vec);
	int get_irq_vector( uint16_t* vec);
	int set_irq_test();
	int set_irq_reset();
	int set_irq_threshold( uint16_t threshold);
	int get_irq_threshold( uint16_t* threshold);
	int set_max_transfer_data( uint16_t max);
	int get_max_transfer_data( uint16_t* get);
	int set_withdraw_irq( uint16_t withdraw);
	int get_withdraw_irq( uint16_t* withdraw);
	int set_cblt_mcst_ctrl( uint16_t ctrl);
	int get_cblt_mcst_ctrl( uint16_t* ctrl);
	int set_cblt_addr( uint16_t addr);
	int get_cblt_addr( uint16_t* addr);
	int set_mcst_addr( uint16_t addr); /* manual says read-only */
	int get_mcst_addr( uint16_t* addr);
	int get_buf_data_len( uint16_t* len);
	int set_data_len_format( uint16_t format);
	int get_data_len_format( uint16_t* format);
	int set_readout_reset();
	int set_multievent( uint16_t mulevt);
	int get_multievent( uint16_t* mulevt);
	int set_marking_type( uint16_t type);
	int get_marking_type( uint16_t* type);
	int set_start_acq( uint16_t start);
	int get_start_acq( uint16_t* start);
	int set_fifo_reset();
	int get_data_ready( uint16_t* ready);
	int set_bank_operation( uint16_t operation);
	int get_bank_operation( uint16_t* operation);
	int set_adc_resolution( uint16_t resolution);
	int get_adc_resolution( uint16_t* resolution);
	int set_output_format( uint16_t format);
	int get_output_format( uint16_t* format);
	int set_adc_override( uint16_t adc_override);
	int get_adc_override( uint16_t* adc_override);
	int set_slc_off( uint16_t off);
	int get_slc_off( uint16_t* off);
	int set_oorange( uint16_t skip);
	int get_oorange( uint16_t* skip);
	int set_ignore_threshold( uint16_t ignore);
	int get_ignore_threshold( uint16_t* ignore);
	int set_hold_delay0( uint16_t val);
	int get_hold_delay0( uint16_t* val);
	int set_hold_delay1( uint16_t val);
	int get_hold_delay1( uint16_t* val);
	int set_hold_width0( uint16_t val);
	int get_hold_width0( uint16_t* val);
	int set_hold_width1( uint16_t val);
	int get_hold_width1( uint16_t* val);
	int set_use_gg( uint16_t use);
	int get_use_gg( uint16_t* use);
	int set_input_range( uint16_t range);
	int get_input_range( uint16_t* range);
	int set_ecl_term( uint16_t term);
	int get_ecl_term( uint16_t* term);
	int set_ecl_gate1_osc( uint16_t val);
	int get_ecl_gate1_osc( uint16_t* val);
	int set_ecl_fc_reset( uint16_t val);
	int get_ecl_fc_reset( uint16_t* val);
	int set_ecl_busy( uint16_t val);
	int get_ecl_busy( uint16_t* val);
	int set_nim_gate1_osc( uint16_t val);
	int get_nim_gate1_osc( uint16_t* val);
	int set_nim_fc_reset( uint16_t val);
	int get_nim_fc_reset( uint16_t* val);
	int set_nim_busy( uint16_t val);
	int get_nim_busy( uint16_t* val);
	int set_test_pulser( uint16_t pulser);
	int get_test_pulser( uint16_t* pulser);
	int set_reset_ctr_ab( uint16_t val);
	int get_reset_ctr_ab( uint16_t* val);
	int get_evtcnt_lo( uint16_t* val);
	int get_evtcnt_hi( uint16_t* val);
	int set_ts_source( uint16_t src);
	int get_ts_source( uint16_t* src);
	int set_ts_div( uint16_t div);
	int get_ts_div( uint16_t* div);
	int get_ts_cnt_lo( uint16_t* ts);
	int get_ts_cnt_hi( uint16_t* ts);
	int get_adc_busy_time_lo( uint16_t* busy_time);
	int get_adc_busy_time_hi( uint16_t* busy_time);
	int get_gate1_time_lo( uint16_t* val);
	int get_gate1_time_hi( uint16_t* val);
	int get_time0( uint16_t* val);
	int get_time1( uint16_t* val);
	int get_time2( uint16_t* val);
	int set_stop_cnt( uint16_t val);
	int get_stop_cnt( uint16_t* val);
};

#endif
