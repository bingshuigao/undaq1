#include "v792.h"

v792::v792()
{
	/* set buffer offset */
	buf_off = 0x0;
	
	name = "v792";
	mod_id = 13;
	clk_freq = DEF_MADC32_CLK;
}

/* Read single event word-by-word from the event buffer 
 * @param am vme address modifier
 * @param evt Pointer to the buffer where the word will be saved
 * @param sz_out number of bytes read.
 * @return 0 if succeed, nonzero error codes if error. */
int v792::read_single_evt(int am, uint32_t *evt, int* sz_out)
{
	int ret, am_old;

	am_old = get_am();
	set_am(am);
	*sz_out = 0;
	do {
		/* event buffer address (offset) in the v792 is 0x0000 */
		ret = read_reg(base_addr, 32, (void*) evt);
		if (ret) {
			set_am(am_old);
			return ret;
		}
		evt++;
		*sz_out += 4;
		if ((((*evt) >> 24) & 0x7) == 4) {  /* end of event */
			set_am(am_old);
			return 0;
		}
	} while (1);
};

/* Enable MCST.
 * @param mcst_addr The mcst_addr to be used. If zero, use the default
 * address (0xBB)
 * @return 0 if succeed, nonzero error codes if error. */
int v792::enable_mcst(uint32_t mcst_addr)
{
	return -E_NOT_IMPLE;
}

/* Enable CBLT
 * @param cblt_addr The address to be used for CBLT readout.
 * @param first if true, set the module to be the first one in the cblt
 * chain
 * @param last  if true, set the module to be the last  one in the cblt
 * chain
 * @return 0 if succeed, nonzero error codes if error.*/
int v792::enable_cblt(uint32_t cblt_addr, int first, int last)
{
	return -E_NOT_IMPLE;
}
int v792::get_cblt_conf(uint16_t* addr, int* cblt_enable, int* cblt_first,
		int* cblt_last)
{
	int ret;
	uint32_t val;

	/* read the configuration */
	ret = read_reg(0x1004, 16, &val);
	RET_IF_NONZERO(ret);

	/* get address */
	if (addr) 
		*addr = val & 0xFF;
	
	/* get board position */
	if (cblt_enable)
		*cblt_enable = val & 0x3;
	if (cblt_first)
		*cblt_first = ((val & 0x3) == 0x2) ? 1 : 0;
	if (cblt_last)
		*cblt_last = ((val & 0x3) == 0x1) ? 1 : 0;
	return 0;
}

int v792::if_trig(bool& x)
{
	uint16_t data_ready;
	int ret;

	ret = read_reg(0x100e, 16, &data_ready);
	RET_IF_NONZERO(ret);
	if (data_ready & 0x1)
		x = true;
	else
		x = false;

	return 0;
}

int v792::on_start()
{
	/* clear the fifo and reset the evt counter */
	uint16_t dum = 0x4;
	int ret = write_reg(0x1032, 16, &dum);
	RET_IF_NONZERO(ret);
	ret = write_reg(0x1034, 16, &dum);
	RET_IF_NONZERO(ret);
	ret = write_reg(0x1040, 16, &dum);
	RET_IF_NONZERO(ret);

	return 0;
}

int v792::on_stop()
{
	/* reset timestamp */

	return 0;
}
