#include "v830.h"

int v830::read_single_evt(int am, uint32_t *evt, int* sz_out)
{
	/* This module does not support this read out mode! */
	return -E_GENERIC;
}

/* Enable MCST.
 * @param mcst_addr The mcst_addr to be used. If zero, use the default
 * address 
 * @return 0 if succeed, nonzero error codes if error. */
int v830::enable_mcst(uint32_t mcst_addr)
{
	/* Not implemented, do not use! */
	return -E_NOT_IMPLE;
}

/* Enable CBLT
 * @param cblt_addr The address to be used for CBLT readout.
 * @param first if true, set the module to be the first one in the cblt
 * chain
 * @param last  if true, set the module to be the last  one in the cblt
 * chain
 * @return 0 if succeed, nonzero error codes if error.*/
int v830::enable_cblt(uint32_t cblt_addr, int first, int last)
{
	/* Not implemented, do not use! */
	return -E_NOT_IMPLE;
}

/* The following functions privide read/write of specific registers,
 * their meanings are self-explained by the names, when in daubt,
 * please check the definitions. The reture values indicate error if
 * nonzero, otherwise return zero.*/
int v830::get_cnt00(uint32_t* cnt)
{
	/* Not implemented, do not use! */
	return -E_NOT_IMPLE;
}
int v830::get_cnt01(uint32_t* cnt)
{
	/* Not implemented, do not use! */
	return -E_NOT_IMPLE;
}
int v830::get_cnt02(uint32_t* cnt)
{
	/* Not implemented, do not use! */
	return -E_NOT_IMPLE;
}
int v830::get_cnt03(uint32_t* cnt)
{
	/* Not implemented, do not use! */
	return -E_NOT_IMPLE;
}
int v830::get_cnt04(uint32_t* cnt)
{
	/* Not implemented, do not use! */
	return -E_NOT_IMPLE;
}
int v830::get_cnt05(uint32_t* cnt)
{
	/* Not implemented, do not use! */
	return -E_NOT_IMPLE;
}
int v830::get_cnt06(uint32_t* cnt)
{
	/* Not implemented, do not use! */
	return -E_NOT_IMPLE;
}
int v830::get_cnt07(uint32_t* cnt)
{
	/* Not implemented, do not use! */
	return -E_NOT_IMPLE;
}
int v830::get_cnt08(uint32_t* cnt)
{
	/* Not implemented, do not use! */
	return -E_NOT_IMPLE;
}
int v830::get_cnt09(uint32_t* cnt)
{
	/* Not implemented, do not use! */
	return -E_NOT_IMPLE;
}
int v830::get_cnt10(uint32_t* cnt)
{
	/* Not implemented, do not use! */
	return -E_NOT_IMPLE;
}
int v830::get_cnt11(uint32_t* cnt)
{
	/* Not implemented, do not use! */
	return -E_NOT_IMPLE;
}
int v830::get_cnt12(uint32_t* cnt)
{
	/* Not implemented, do not use! */
	return -E_NOT_IMPLE;
}
int v830::get_cnt13(uint32_t* cnt)
{
	/* Not implemented, do not use! */
	return -E_NOT_IMPLE;
}
int v830::get_cnt14(uint32_t* cnt)
{
	/* Not implemented, do not use! */
	return -E_NOT_IMPLE;
}
int v830::get_cnt15(uint32_t* cnt)
{
	/* Not implemented, do not use! */
	return -E_NOT_IMPLE;
}
int v830::get_cnt16(uint32_t* cnt)
{
	/* Not implemented, do not use! */
	return -E_NOT_IMPLE;
}
int v830::get_cnt17(uint32_t* cnt)
{
	/* Not implemented, do not use! */
	return -E_NOT_IMPLE;
}
int v830::get_cnt18(uint32_t* cnt)
{
	/* Not implemented, do not use! */
	return -E_NOT_IMPLE;
}
int v830::get_cnt19(uint32_t* cnt)
{
	/* Not implemented, do not use! */
	return -E_NOT_IMPLE;
}
int v830::get_cnt20(uint32_t* cnt)
{
	/* Not implemented, do not use! */
	return -E_NOT_IMPLE;
}
int v830::get_cnt21(uint32_t* cnt)
{
	/* Not implemented, do not use! */
	return -E_NOT_IMPLE;
}
int v830::get_cnt22(uint32_t* cnt)
{
	/* Not implemented, do not use! */
	return -E_NOT_IMPLE;
}
int v830::get_cnt23(uint32_t* cnt)
{
	/* Not implemented, do not use! */
	return -E_NOT_IMPLE;
}
int v830::get_cnt24(uint32_t* cnt)
{
	/* Not implemented, do not use! */
	return -E_NOT_IMPLE;
}
int v830::get_cnt25(uint32_t* cnt)
{
	/* Not implemented, do not use! */
	return -E_NOT_IMPLE;
}
int v830::get_cnt26(uint32_t* cnt)
{
	/* Not implemented, do not use! */
	return -E_NOT_IMPLE;
}
int v830::get_cnt27(uint32_t* cnt)
{
	/* Not implemented, do not use! */
	return -E_NOT_IMPLE;
}
int v830::get_cnt28(uint32_t* cnt)
{
	/* Not implemented, do not use! */
	return -E_NOT_IMPLE;
}
int v830::get_cnt29(uint32_t* cnt)
{
	/* Not implemented, do not use! */
	return -E_NOT_IMPLE;
}
int v830::get_cnt30(uint32_t* cnt)
{
	/* Not implemented, do not use! */
	return -E_NOT_IMPLE;
}
int v830::get_cnt31(uint32_t* cnt)
{
	/* Not implemented, do not use! */
	return -E_NOT_IMPLE;
}
int v830::set_test_reg(uint32_t val)
{
	/* Not implemented, do not use! */
	return -E_NOT_IMPLE;
}
int v830::get_test_reg(uint32_t* val)
{
	/* Not implemented, do not use! */
	return -E_NOT_IMPLE;
}
int v830::set_test_lcntl(uint16_t val)
{
	/* Not implemented, do not use! */
	return -E_NOT_IMPLE;
}
int v830::get_test_lcntl(uint16_t* val)
{
	/* Not implemented, do not use! */
	return -E_NOT_IMPLE;
}
int v830::set_test_lcnth(uint16_t val)
{
	/* Not implemented, do not use! */
	return -E_NOT_IMPLE;
}
int v830::get_test_lcnth(uint16_t* val)
{
	/* Not implemented, do not use! */
	return -E_NOT_IMPLE;
}
int v830::set_test_hcntl(uint16_t val)
{
	/* Not implemented, do not use! */
	return -E_NOT_IMPLE;
}
int v830::get_test_hcntl(uint16_t* val)
{
	/* Not implemented, do not use! */
	return -E_NOT_IMPLE;
}
int v830::set_test_hcnth(uint16_t val)
{
	/* Not implemented, do not use! */
	return -E_NOT_IMPLE;
}
int v830::get_test_hcnth(uint16_t* val)
{
	/* Not implemented, do not use! */
	return -E_NOT_IMPLE;
}
int v830::set_ch_enable(uint32_t val)
{
	/* Not implemented, do not use! */
	return -E_NOT_IMPLE;
}
int v830::get_ch_enable(uint32_t* val)
{
	/* Not implemented, do not use! */
	return -E_NOT_IMPLE;
}
int v830::set_dwell_time(uint32_t val)
{
	/* Not implemented, do not use! */
}
int v830::get_dwell_time(uint32_t* val)
{
	/* Not implemented, do not use! */
}
int v830::set_ctrl_reg(uint16_t val)
{
	/* Not implemented, do not use! */
}
int v830::get_ctrl_reg(uint16_t* val)
{
	/* Not implemented, do not use! */
}
int v830::set_bit_set_reg(uint16_t val)
{
	/* Not implemented, do not use! */
}
int v830::set_bit_clr_reg(uint16_t val)
{
	/* Not implemented, do not use! */
}
int v830::set_status_reg(uint16_t val)
{
	/* Not implemented, do not use! */
}
int v830::get_status_reg(uint16_t* val)
{
	/* Not implemented, do not use! */
}
int v830::set_geo_reg(uint16_t geo)
{
	/* Not implemented, do not use! */
}
int v830::get_geo_reg(uint16_t* geo)
{
	/* Not implemented, do not use! */
}
int v830::set_int_lev(uint16_t lev)
{
	/* Not implemented, do not use! */
}
int v830::get_int_lev(uint16_t* lev)
{
	/* Not implemented, do not use! */
}
int v830::set_int_vec(uint16_t vec)
{
	/* Not implemented, do not use! */
}
int v830::get_int_vec(uint16_t* vec)
{
	/* Not implemented, do not use! */
}
int v830::set_addr32(uint16_t addr32)
{
	/* Not implemented, do not use! */
}
int v830::get_addr32(uint16_t* addr32)
{
	/* Not implemented, do not use! */
}
int v830::set_addr24(uint16_t addr24)
{
	/* Not implemented, do not use! */
}
int v830::get_addr24(uint16_t* addr24)
{
	/* Not implemented, do not use! */
}
int v830::set_enable_ader(uint16_t val)
{
	/* Not implemented, do not use! */
}
int v830::get_enable_ader(uint16_t* val)
{
	/* Not implemented, do not use! */
}
int v830::set_mcst_base(uint16_t mcst_base)
{
	/* Not implemented, do not use! */
}
int v830::get_mcst_base(uint16_t* mcst_base)
{
	/* Not implemented, do not use! */
}
int v830::set_mcst_ctrl(uint16_t val)
{
	/* Not implemented, do not use! */
}
int v830::get_mcst_ctrl(uint16_t* val)
{
	/* Not implemented, do not use! */
}
int v830::set_mod_reset()
{
	/* Not implemented, do not use! */
}
int v830::set_soft_clr()
{
	/* Not implemented, do not use! */
}
int v830::set_soft_trig()
{
	/* Not implemented, do not use! */
}
int v830::get_trig_cnt(uint16_t* cnt)
{
	/* Not implemented, do not use! */
}
int v830::set_almost_ful(uint16_t val)
{
	/* Not implemented, do not use! */
}
int v830::get_almost_ful(uint16_t* val)
{
	/* Not implemented, do not use! */
}
int v830::set_blt_evt_num(uint16_t num)
{
	/* Not implemented, do not use! */
}
int v830::get_blt_evt_num(uint16_t* num)
{
	/* Not implemented, do not use! */
}
int v830::get_meb_evt_num(uint16_t* num)
{
	/* Not implemented, do not use! */
}
int v830::set_dum32(uint32_t val)
{
	/* Not implemented, do not use! */
}
int v830::get_dum32(uint32_t* val)
{
	/* Not implemented, do not use! */
}
int v830::set_dum16(uint16_t val)
{
	/* Not implemented, do not use! */
}
int v830::get_dum16(uint16_t* val)
{
	/* Not implemented, do not use! */
}
int v830::get_conf_rom(uint16_t* val)
{
	/* Not implemented, do not use! */
}
int v830::get_fw_ver(uint16_t* ver)
{
	/* Not implemented, do not use! */
}
