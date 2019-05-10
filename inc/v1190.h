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

	/* see comments in module.h */
	virtual int on_start();
	virtual int on_stop();


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
};
#endif
