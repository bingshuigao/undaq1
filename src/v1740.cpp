#include "v1740.h"

v1740::v1740()
{
	/* set buffer offset */
	buf_off = 0x0;
	
	name = "v1740";
	mod_id = 5;
}

/* Read single event word-by-word from the event buffer 
 * @param am vme address modifier
 * @param evt Pointer to the buffer where the word will be saved
 * @param sz_out number of bytes read.
 * @return 0 if succeed, nonzero error codes if error. */
int v1740::read_single_evt(int am, uint32_t *evt, int* sz_out)
{
	int ret, am_old, len_dw;

	am_old = get_am();
	set_am(am);
	*sz_out = 4;
	/* first, read the event length */
	ret = read_reg(base_addr, 32, (void*)evt);
	if (ret) 
		goto end;
	len_dw = 0xfffffff & evt[0];
	while (--len_dw) {
		evt++;
		/* event buffer address (offset) in the v1740 is 0x0000 */
		ret = read_reg(base_addr, 32, (void*) evt);
		if (ret) 
			goto end;
		*sz_out += 4;
	}

end:
	set_am(am_old);
	return ret;
};

/* Enable MCST.
 * @param mcst_addr The mcst_addr to be used. If zero, use the default
 * address (0xBB)
 * @return 0 if succeed, nonzero error codes if error. */
int v1740::enable_mcst(uint32_t mcst_addr)
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
int v1740::enable_cblt(uint32_t cblt_addr, int first, int last)
{
	return -E_NOT_IMPLE;
}
int v1740::get_cblt_conf(uint16_t* addr, int* cblt_enable, int* cblt_first,
		int* cblt_last)
{
	int ret;
	uint32_t val;

	/* read the configuration */
	ret = read_reg(0xef0c, 32, &val);
	RET_IF_NONZERO(ret);

	/* get address */
	if (addr) 
		*addr = val & 0xFF;
	
	/* get board position */
	if (cblt_enable)
		*cblt_enable = val & 0x300;
	if (cblt_first)
		*cblt_first = ((val & 0x300) == 0x200) ? 1 : 0;
	if (cblt_last)
		*cblt_first = ((val & 0x300) == 0x100) ? 1 : 0;
	return 0;
}

int v1740::if_trig(bool& x)
{
	uint32_t acq_status;
	int ret;

	ret = read_reg(0x8104, 32, &acq_status);
	RET_IF_NONZERO(ret);
	if (acq_status & 0x8)
		x = true;
	else
		x = false;

	return 0;
}

int v1740::on_start()
{
	/* clear the fifo */
	uint32_t dum = 0;
	int ret = write_reg(0xef28, 32, &dum);
	RET_IF_NONZERO(ret);

	/* start acquasition */
	do {
		ret = read_reg(0x8104, 32, &dum);
		RET_IF_NONZERO(ret);
	} while (!(dum & 0x100))
	ret = read_reg(0x8100, 32, &dum);
	RET_IF_NONZERO(ret);
	dum |= 0x4;
	ret = write_reg(0x8100, 32, &dum);
	RET_IF_NONZERO(ret);

	return 0;
}

int v1740::on_stop()
{
	/* stop acquasition */
	uint32_t val;
	int ret;

	ret = read_reg(0x8100, 32, &val);
	RET_IF_NONZERO(ret);
	val &= 0xfffffffb;
	ret = write_reg(0x8100, 32, &val);
	RET_IF_NONZERO(ret);

	return 0;
}
