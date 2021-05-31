#include "madc32.h"
#include <unistd.h>

madc32::madc32()
{
	/* set buffer offset */
	buf_off = 0x0;
	
	name = "madc32";
	mod_id = 1;
	clk_freq = DEF_MADC32_CLK;
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
	int ret;
	uint16_t val;
	if (mcst_addr) {
		val = mcst_addr & 0xFF;
		ret = write_reg(0x6024, 16, &val);
		RET_IF_NONZERO(ret);
	}
	val = 0x80;
	return write_reg(0x6020, 16, &val);
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
	int ret;
	uint16_t val;

	val = 0x2;
	if (first)
		val &= 0x10;
	if (last)
		val &= 0x8;
	ret = write_reg(0x6020, 16, &val);
	RET_IF_NONZERO(ret);

	val = cblt_addr & 0xFF;
	return write_reg(0x6022, 16, &val);
}
int madc32::get_cblt_conf(uint16_t* addr, int* cblt_enable, int* cblt_first,
		int* cblt_last)
{
	int ret;
	uint16_t val;

	/* if we are in test mode, return cblt enable false */
	if (get_ctl()->get_name() == "test_ctl") {
		if (cblt_first) *cblt_first = 0;
		if (cblt_last) *cblt_last = 0;
		if (cblt_enable) *cblt_enable = 0;
		return 0;
	}

	/* get address */
	if (addr) {
		ret = read_reg(0x6022, 16, addr);
		RET_IF_NONZERO(ret);
	}
	
	ret = read_reg(0x6020, 16, &val);
	RET_IF_NONZERO(ret);
	/* get cblt_enable */
	if (cblt_enable) {
		if (val & 0x1)
			*cblt_enable = 1;
		else
			*cblt_enable = 0;
	}
	/* get cblt_first */
	if (cblt_first) {
		if (val & 0x10)
			*cblt_first = 1;
		else
			*cblt_first = 0;
	}
	/* get cblt_last */
	if (cblt_last) {
		if (val & 0x4)
			*cblt_last = 1;
		else
			*cblt_last = 0;
	}
	return 0;
}

int madc32::if_trig(bool& x)
{
	uint16_t data_ready;
	int ret;

	/* if we are in test mode, always return true */
	if (get_ctl()->get_name() == "test_ctl") {
		x = true;
//		std::cout<<"xxxxxxxxxx"<<std::endl;
		usleep(1000);
		return 0;
	}

	ret = read_reg(0x603e, 16, &data_ready);
	RET_IF_NONZERO(ret);
	if (data_ready)
		x = true;
	else
		x = false;

	return 0;
}

int madc32::on_start()
{
	/* clear the fifo */
	uint16_t dum = 0;
	int ret = write_reg(0x603c, 16, &dum);
	RET_IF_NONZERO(ret);

	/* start acq */
	dum = 1;
	ret = write_reg(0x603a, 16, &dum);
	RET_IF_NONZERO(ret);

	/* readout reset */
	dum = 0;
	ret = write_reg(0x6034, 16, &dum);
	RET_IF_NONZERO(ret);

	/* reset event counter */
	dum = 1;
	ret = write_reg(0x6090, 16, &dum);
	RET_IF_NONZERO(ret);

	//printf("debuf\n");
	return 0;
}

int madc32::on_stop()
{
	/* reset timestamp */
	uint16_t dum = 0xc;
	int ret;
	ret = write_reg(0x6090, 16, &dum);
	RET_IF_NONZERO(ret);

	return 0;
}
