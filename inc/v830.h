/* The class v830 represents the v830 module
 * By B.Gao Aug. 2018 */

#ifndef V830_HHH
#define V830_HHH

#include <stdint.h>
#include "vme_ctl.h"
#include "err_code.h"
#include "module.h"

class v830 : public module
{
public:
	v830(){};
	~v830(){};

	/* Read single event word-by-word from the event buffer 
	 * @param am vme address modifier
	 * @param evt Pointer to the buffer where the word will be saved
	 * @param sz_out number of bytes read.
	 * @return 0 if succeed, nonzero error codes if error. */
	int read_single_evt(int am, uint32_t *evt, int* sz_out);

	/* Enable MCST.
	 * @param mcst_addr The mcst_addr to be used. If zero, use the default
	 * address 
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

	/* The following functions privide read/write of specific registers,
	 * their meanings are self-explained by the names, when in daubt,
	 * please check the definitions. The reture values indicate error if
	 * nonzero, otherwise return zero.*/
	int get_cnt00(uint32_t* cnt);
	int get_cnt01(uint32_t* cnt);
	int get_cnt02(uint32_t* cnt);
	int get_cnt03(uint32_t* cnt);
	int get_cnt04(uint32_t* cnt);
	int get_cnt05(uint32_t* cnt);
	int get_cnt06(uint32_t* cnt);
	int get_cnt07(uint32_t* cnt);
	int get_cnt08(uint32_t* cnt);
	int get_cnt09(uint32_t* cnt);
	int get_cnt10(uint32_t* cnt);
	int get_cnt11(uint32_t* cnt);
	int get_cnt12(uint32_t* cnt);
	int get_cnt13(uint32_t* cnt);
	int get_cnt14(uint32_t* cnt);
	int get_cnt15(uint32_t* cnt);
	int get_cnt16(uint32_t* cnt);
	int get_cnt17(uint32_t* cnt);
	int get_cnt18(uint32_t* cnt);
	int get_cnt19(uint32_t* cnt);
	int get_cnt20(uint32_t* cnt);
	int get_cnt21(uint32_t* cnt);
	int get_cnt22(uint32_t* cnt);
	int get_cnt23(uint32_t* cnt);
	int get_cnt24(uint32_t* cnt);
	int get_cnt25(uint32_t* cnt);
	int get_cnt26(uint32_t* cnt);
	int get_cnt27(uint32_t* cnt);
	int get_cnt28(uint32_t* cnt);
	int get_cnt29(uint32_t* cnt);
	int get_cnt30(uint32_t* cnt);
	int get_cnt31(uint32_t* cnt);
	int set_test_reg(uint32_t val);
	int get_test_reg(uint32_t* val);
	int set_test_lcntl(uint16_t val);
	int get_test_lcntl(uint16_t* val);
	int set_test_lcnth(uint16_t val);
	int get_test_lcnth(uint16_t* val);
	int set_test_hcntl(uint16_t val);
	int get_test_hcntl(uint16_t* val);
	int set_test_hcnth(uint16_t val);
	int get_test_hcnth(uint16_t* val);
	int set_ch_enable(uint32_t val);
	int get_ch_enable(uint32_t* val);
	int set_dwell_time(uint32_t val);
	int get_dwell_time(uint32_t* val);
	int set_ctrl_reg(uint16_t val);
	int get_ctrl_reg(uint16_t* val);
	int set_bit_set_reg(uint16_t val);
	int set_bit_clr_reg(uint16_t val);
	int set_status_reg(uint16_t val);
	int get_status_reg(uint16_t* val);
	int set_geo_reg(uint16_t geo);
	int get_geo_reg(uint16_t* geo);
	int set_int_lev(uint16_t lev);
	int get_int_lev(uint16_t* lev);
	int set_int_vec(uint16_t vec);
	int get_int_vec(uint16_t* vec);
	int set_addr32(uint16_t addr32);
	int get_addr32(uint16_t* addr32);
	int set_addr24(uint16_t addr24);
	int get_addr24(uint16_t* addr24);
	int set_enable_ader(uint16_t val);
	int get_enable_ader(uint16_t* val);
	int set_mcst_base(uint16_t mcst_base);
	int get_mcst_base(uint16_t* mcst_base);
	int set_mcst_ctrl(uint16_t val);
	int get_mcst_ctrl(uint16_t* val);
	int set_mod_reset();
	int set_soft_clr();
	int set_soft_trig();
	int get_trig_cnt(uint16_t* cnt);
	int set_almost_ful(uint16_t val);
	int get_almost_ful(uint16_t* val);
	int set_blt_evt_num(uint16_t num);
	int get_blt_evt_num(uint16_t* num);
	int get_meb_evt_num(uint16_t* num);
	int set_dum32(uint32_t val);
	int get_dum32(uint32_t* val);
	int set_dum16(uint16_t val);
	int get_dum16(uint16_t* val);
	int get_conf_rom(uint16_t* val);
	int get_fw_ver(uint16_t* ver);
};

#endif
