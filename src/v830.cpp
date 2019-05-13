#include "v830.h"

v830::v830()
{
	buf_off = 0x0;
	name = "v830";
	mod_id = 3;
	geo = -1;
	 

}

int v830::read_single_evt(int am, uint32_t *evt, int* sz_out)
{
	/* This module does not support this read out mode! */
	return -E_NOT_SUPPORT;
}

int v830::get_cblt_conf(uint16_t* addr, int* cblt_enable, int* cblt_first,
			int* cblt_last)
{
	int ret;
	uint16_t val;

	ret = read_reg(0x111e, 16, &val);
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
		ret = read_reg(0x111c, 16, &val);
		RET_IF_NONZERO(ret);
		*addr = val;
	}

	return 0;

}



/* Enable MCST.
 * @param mcst_addr The mcst_addr to be used. If zero, use the default
 * address 
 * @return 0 if succeed, nonzero error codes if error. */
int v830::enable_mcst(uint32_t mcst_addr)
{
	/* Not implemented, do not use! */
	return -E_NOT_SUPPORT;
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
	int ret;
	uint16_t val;
	
	val = cblt_addr & 0xFF;
	ret = write_reg(0x111c, 16, &val);
	RET_IF_NONZERO(ret);

	if (first & last)
		return -E_NOT_SUPPORT;
	else if (first & (!last))
		val = 2;
	else if ((!first) & last)
		val = 1;
	else if ((!first) & (!last))
		val = 3;
	return write_reg(0x111e, 16, &val);

}

int v830::on_start()
{
	/* reset the counters and mem buffer */
	uint16_t dum = 0;
	int ret = write_reg(0x1122, 16, &dum);
	RET_IF_NONZERO(ret);

	return 0;
}

int v830::on_stop()
{
	std::cout<<"v830 stopped"<<std::endl;
	return 0;
}
