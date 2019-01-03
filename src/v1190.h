/* The class v1190 represents the V1190A/B modules.
 * By B.Gao Aug. 2018 */

#ifndef V1190_HHH
#define V1190_HHH

#include <stdint.h>
#include "vme_ctl.h"
#include "err_code.h"
#include "module.h"

class v1190 : public module
{
public:
	v1190();
	~v1190(){};

	/* Read single event word-by-word from the event buffer 
	 * Note: it works only for trigger-matching mode.
	 * @param am vme address modifier
	 * @param evt Pointer to the buffer where the word will be saved
	 * @param sz_out number of bytes read.
	 * @return 0 if succeed, nonzero error codes if error. */
	int read_single_evt(int am, uint32_t *evt, int* sz_out);

	/* see the comments in base class */
	int write_reg(uint32_t addr, int dw, void* val)
	{
		if (addr == 0x100e) 
			geo = reinterpret_cast<uint16_t*>(val)[0];
		return module::write_reg(addr, dw, val);
	}

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
	
	/* write to and read from micro. The procedure is slightly different
	 * from reading or writing registers (READ_OK/WRITE_OK bits have to be
	 * tested before reading or writing). We copy the function from the
	 * file cvt_V1190.c provided by caen demo)
	 * @param ope_code The operation code 
	 * @param p_pars Pointer to an array of parameters
	 * @n Number of paramters 
	 * @return 0 if succeed, otherwise return the error code. */
	int write_micro( uint16_t ope_code, uint16_t* p_pars, int n);
	int read_micro( uint16_t ope_code, uint16_t* p_pars, int n);

	/* Get the configurations of cblt registers
	 * @param addr [out] cblt address
	 * @param cblt_enable [out] =1 if cblt is enabled, else = 0; 
	 * @param cblt_first  [out] =1 if set to the first in cblt chain
	 * @param cblt_last   [out] =1 if set to the last in cblt chain
	 * @return 0 if succeed, nonzero error codes if error. 
	 * Note: if any of the parameters are NULL, the corresponding
	 * information won't be returned */
	int get_cblt_conf(uint16_t* addr, int* cblt_enable, 
			int* cblt_first, int* cblt_last);

	int get_geo()
	{
		if (geo == -1)
			/* use default value */
			return 0x1F;
		else
			return geo;
	}





	/* The following functions privide read/write of specific registers,
	 * their meanings are self-explained by the names, when in daubt,
	 * please check the definitions. The reture values indicate error if
	 * nonzero, otherwise return zero.*/
	int set_control_reg( uint16_t val);
	int get_control_reg( uint16_t* val);
	int get_status_reg( uint16_t* status);
	int set_int_lev( uint16_t lev);
	int get_int_lev( uint16_t* lev);
	int set_int_vec( uint16_t vec);
	int get_int_vec( uint16_t* vec);
	int set_geo_addr( uint16_t geo);
	int get_geo_addr( uint16_t* geo);
	int set_mcst_base( uint16_t mcst_base);
	int get_mcst_base( uint16_t* mcst_base);
	int set_mcst_control( uint16_t val);
	int get_mcst_control( uint16_t* val);
	int set_mod_reset();
	int set_soft_clear();
	int set_soft_evt_reset();
	int set_soft_trig();
	int get_evt_counter( uint32_t* cnt);
	int get_evt_stored( uint16_t* val);
	int set_almost_full_lev( uint16_t lev);
	int get_almost_full_lev( uint16_t* lev);
	int set_blt_evt_num( uint16_t num);
	int get_blt_evt_num( uint16_t* num);
	int get_fw_ver( uint16_t* ver);
	int set_test_reg( uint32_t val);
	int get_test_reg( uint32_t* val);
	int set_output_prog_ctrl( uint16_t val);
	int get_output_prog_ctrl( uint16_t* val);
	int get_micro_handshake( uint16_t* val);
	int set_select_flash( uint16_t val);
	int get_select_flash( uint16_t* val);
	int set_flash_mem( uint16_t val);
	int get_flash_mem( uint16_t* val);
	int set_sram_page( uint16_t val);
	int get_sram_page( uint16_t* val);
	int get_evt_fifo( uint32_t* val);
	int get_evt_fifo_stored( uint16_t* val);
	int get_evt_fifo_status( uint16_t* val);
	int set_dummy32( uint32_t val);
	int get_dummy32( uint32_t* val);
	int set_dummy16( uint16_t val);
	int get_dummy16( uint16_t* val);
	int get_conf_rom( uint16_t* val);
	int get_com_sram( uint16_t* val);
};
#endif
