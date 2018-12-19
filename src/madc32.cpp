#include "madc32.h"

madc32::madc32()
{
	/* set buffer offset */
	buf_off = 0x0;
	
	name = "madc32";
	mod_id = 1;
}

/* Read single event word-by-word from the event buffer 
 * @param am vme address modifier
 * @param evt Pointer to the buffer where the word will be saved
 * @param sz_out number of bytes read.
 * @return 0 if succeed, nonzero error codes if error. */
int madc32::read_single_evt(int am, uint32_t *evt, int* sz_out)
{
	int ret, am_old;

	am_old = get_am();
	set_am(am);
	*sz_out = 0;
	do {
		/* event buffer address (offset) in the madc32 is 0x0000 */
		ret = read_reg(base_addr, 32, (void*) evt);
		if (ret) {
			set_am(am_old);
			return ret;
		}
		evt++;
		*sz_out += 4;
		if (((*evt) >> 30) == 3) {  /* end of event */
			set_am(am_old);
			return 0;
		}
	} while (1);
};

/* Enable MCST.
 * @param mcst_addr The mcst_addr to be used. If zero, use the default
 * address (0xBB)
 * @return 0 if succeed, nonzero error codes if error. */
int madc32::enable_mcst(uint32_t mcst_addr)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}

/* Enable CBLT
 * @param cblt_addr The address to be used for CBLT readout.
 * @param first if true, set the module to be the first one in the cblt
 * chain
 * @param last  if true, set the module to be the last  one in the cblt
 * chain
 * @return 0 if succeed, nonzero error codes if error.*/
int madc32::enable_cblt(uint32_t cblt_addr, int first, int last)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}

/* The following functions privide read/write of specific registers,
 * their meanings are self-explained by the names, when in daubt,
 * please check the definitions. The reture values indicate error if
 * nonzero, otherwise return zero.*/
int madc32::set_threshold(int channel,  uint16_t threshold)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::get_threshold(int channel,  uint16_t* threshold)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::set_module_id( uint16_t id)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::get_module_id( uint16_t* id)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::set_soft_reset()
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::get_fw_version( uint16_t* version)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::set_irq_level( uint16_t level)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::get_irq_level( uint16_t* level)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::set_irq_vector( uint16_t vec)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::get_irq_vector( uint16_t* vec)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::set_irq_test()
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::set_irq_reset()
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::set_irq_threshold( uint16_t threshold)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::get_irq_threshold( uint16_t* threshold)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::set_max_transfer_data( uint16_t max)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::get_max_transfer_data( uint16_t* get)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::set_withdraw_irq( uint16_t withdraw)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::get_withdraw_irq( uint16_t* withdraw)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::set_cblt_mcst_ctrl( uint16_t ctrl)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::get_cblt_mcst_ctrl( uint16_t* ctrl)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::set_cblt_addr( uint16_t addr)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::get_cblt_addr( uint16_t* addr)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::set_mcst_addr( uint16_t addr)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}/* manual says read-only */
int madc32::get_mcst_addr( uint16_t* addr)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::get_buf_data_len( uint16_t* len)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::set_data_len_format( uint16_t format)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::get_data_len_format( uint16_t* format)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::set_readout_reset()
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::set_multievent( uint16_t mulevt)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::get_multievent( uint16_t* mulevt)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::set_marking_type( uint16_t type)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::get_marking_type( uint16_t* type)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::set_start_acq( uint16_t start)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::get_start_acq( uint16_t* start)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::set_fifo_reset()
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::get_data_ready( uint16_t* ready)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::set_bank_operation( uint16_t operation)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::get_bank_operation( uint16_t* operation)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::set_adc_resolution( uint16_t resolution)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::get_adc_resolution( uint16_t* resolution)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::set_output_format( uint16_t format)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::get_output_format( uint16_t* format)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::set_adc_override( uint16_t adc_override)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::get_adc_override( uint16_t* adc_override)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::set_slc_off( uint16_t off)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::get_slc_off( uint16_t* off)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::set_oorange( uint16_t skip)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::get_oorange( uint16_t* skip)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::set_ignore_threshold( uint16_t ignore)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::get_ignore_threshold( uint16_t* ignore)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::set_hold_delay0( uint16_t val)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::get_hold_delay0( uint16_t* val)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::set_hold_delay1( uint16_t val)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::get_hold_delay1( uint16_t* val)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::set_hold_width0( uint16_t val)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::get_hold_width0( uint16_t* val)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::set_hold_width1( uint16_t val)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::get_hold_width1( uint16_t* val)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::set_use_gg( uint16_t use)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::get_use_gg( uint16_t* use)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::set_input_range( uint16_t range)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::get_input_range( uint16_t* range)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::set_ecl_term( uint16_t term)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::get_ecl_term( uint16_t* term)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::set_ecl_gate1_osc( uint16_t val)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::get_ecl_gate1_osc( uint16_t* val)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::set_ecl_fc_reset( uint16_t val)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::get_ecl_fc_reset( uint16_t* val)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::set_ecl_busy( uint16_t val)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::get_ecl_busy( uint16_t* val)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::set_nim_gate1_osc( uint16_t val)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::get_nim_gate1_osc( uint16_t* val)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::set_nim_fc_reset( uint16_t val)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::get_nim_fc_reset( uint16_t* val)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::set_nim_busy( uint16_t val)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::get_nim_busy( uint16_t* val)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::set_test_pulser( uint16_t pulser)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::get_test_pulser( uint16_t* pulser)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::set_reset_ctr_ab( uint16_t val)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::get_reset_ctr_ab( uint16_t* val)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::get_evtcnt_lo( uint16_t* val)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::get_evtcnt_hi( uint16_t* val)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::set_ts_source( uint16_t src)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::get_ts_source( uint16_t* src)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::set_ts_div( uint16_t div)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::get_ts_div( uint16_t* div)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::get_ts_cnt_lo( uint16_t* ts)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::get_ts_cnt_hi( uint16_t* ts)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::get_adc_busy_time_lo( uint16_t* busy_time)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::get_adc_busy_time_hi( uint16_t* busy_time)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::get_gate1_time_lo( uint16_t* val)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::get_gate1_time_hi( uint16_t* val)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::get_time0( uint16_t* val)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::get_time1( uint16_t* val)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::get_time2( uint16_t* val)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::set_stop_cnt( uint16_t val)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int madc32::get_stop_cnt( uint16_t* val)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
