#include "v1190.h"

v1190::v1190()
{
	buf_off = 0x0;
	name = "v1190";
	mod_id = 2;
	geo = -1;
}

/* Read single event word-by-word from the event buffer 
 * Note: It works only for trigger-matching mode.
 * @param am vme address modifier
 * @param evt Pointer to the buffer where the word will be saved
 * @param sz_out number of bytes read.
 * @return 0 if succeed, nonzero error codes if error. */
int v1190::read_single_evt(int am, uint32_t *evt, int* sz_out)
{
	int ret, am_old;

	am_old = get_am();
	*sz_out = 0;
	do {
		/* event buffer address (offset) in the v1190 is 0x0000 */
		ret = read_reg(base_addr, 32, (void*) evt);
		if (ret) {
			set_am(am_old);
			return ret;
		}
		evt++;
		*sz_out += 4;
		if (((*evt) >> 27) == 0x10) {  /* end of event */
			set_am(am_old);
			return 0;
		}
	} while (1);
};

int v1190::get_cblt_conf(uint16_t* addr, int* cblt_enable, int* cblt_first,
		int* cblt_last)
{
	int ret;
	uint16_t val;

	ret = read_reg(0x1012, 16, &val);
	RET_IF_NONZERO(ret);
	if (cblt_enable)
		*cblt_enable = val & 0x3;
	if (cblt_first) {
		if (val == 0x2)
			*cblt_first = 1;
		else
			*cblt_first = 0;
	}
	if (cblt_last) {
		if (val == 0x1)
			*cblt_last = 1;
		else
			*cblt_last = 0;
	}
	if (addr) {
		ret = read_reg(0x1010, 16, &val);
		RET_IF_NONZERO(ret);
		*addr = val;
	}

	return 0;
}




/* Enable MCST.
 * @param mcst_addr The mcst_addr to be used. If zero, use the default
 * address (0xBB)
 * @return 0 if succeed, nonzero error codes if error. */
int v1190::enable_mcst(uint32_t mcst_addr)
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
int v1190::enable_cblt(uint32_t cblt_addr, int first, int last)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}

int write_micro( uint16_t ope_code, uint16_t* p_pars, int n)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int read_micro( uint16_t ope_code, uint16_t* p_pars, int n)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}

int v1190::set_control_reg( uint16_t val)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int v1190::get_control_reg( uint16_t* val)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int v1190::get_status_reg( uint16_t* status)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int v1190::set_int_lev( uint16_t lev)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int v1190::get_int_lev( uint16_t* lev)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int v1190::set_int_vec( uint16_t vec)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int v1190::get_int_vec( uint16_t* vec)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int v1190::set_geo_addr( uint16_t geo)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int v1190::get_geo_addr( uint16_t* geo)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int v1190::set_mcst_base( uint16_t mcst_base)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int v1190::get_mcst_base( uint16_t* mcst_base)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int v1190::set_mcst_control( uint16_t val)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int v1190::get_mcst_control( uint16_t* val)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int v1190::set_mod_reset()
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int v1190::set_soft_clear()
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int v1190::set_soft_evt_reset()
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int v1190::set_soft_trig()
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int v1190::get_evt_counter( uint32_t* cnt)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int v1190::get_evt_stored( uint16_t* val)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int v1190::set_almost_full_lev( uint16_t lev)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int v1190::get_almost_full_lev( uint16_t* lev)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int v1190::set_blt_evt_num( uint16_t num)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int v1190::get_blt_evt_num( uint16_t* num)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int v1190::get_fw_ver( uint16_t* ver)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int v1190::set_test_reg( uint32_t val)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int v1190::get_test_reg( uint32_t* val)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int v1190::set_output_prog_ctrl( uint16_t val)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int v1190::get_output_prog_ctrl( uint16_t* val)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int v1190::get_micro_handshake( uint16_t* val)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int v1190::set_select_flash( uint16_t val)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int v1190::get_select_flash( uint16_t* val)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int v1190::set_flash_mem( uint16_t val)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int v1190::get_flash_mem( uint16_t* val)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int v1190::set_sram_page( uint16_t val)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int v1190::get_sram_page( uint16_t* val)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int v1190::get_evt_fifo( uint32_t* val)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int v1190::get_evt_fifo_stored( uint16_t* val)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int v1190::get_evt_fifo_status( uint16_t* val)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int v1190::set_dummy32( uint32_t val)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int v1190::get_dummy32( uint32_t* val)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int v1190::set_dummy16( uint16_t val)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int v1190::get_dummy16( uint16_t* val)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int v1190::get_conf_rom( uint16_t* val)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
int v1190::get_com_sram( uint16_t* val)
{
	/* not implemented, don't use! */
	return -E_NOT_IMPLE;
}
