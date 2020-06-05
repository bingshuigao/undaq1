#include "v1190.h"
#include <iostream>

v1190::v1190()
{
	buf_off = 0x0;
	name = "v1190";
	mod_id = 2;
	geo = -1;
	clk_freq = DEF_V1190_CLK;
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
	return -E_NOT_SUPPORT;
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
	int ret;
	uint16_t val;
	
	val = cblt_addr & 0xFF;
	ret = write_reg(0x1010, 16, &val);
	RET_IF_NONZERO(ret);

	if (first & last)
		return -E_NOT_SUPPORT;
	else if (first & (!last))
		val = 2;
	else if ((!first) & last)
		val = 1;
	else if ((!first) & (!last))
		val = 3;
	return write_reg(0x1012, 16, &val);
}

int v1190::write_micro( uint16_t ope_code, uint16_t* p_pars, int n)
{
	uint16_t micro_hnd= 0;
	int ret;
	// wait for micro register write ok
	do {
		ret = read_reg(0x1030, 16, &micro_hnd);
		RET_IF_NONZERO(ret);
	} while(!(micro_hnd & 0x1));
	// write opcode to micro register
	ret = write_reg(0x102e, 16, &ope_code);
	RET_IF_NONZERO(ret);
	// write ope cod
	while(n--) {
		// wait for micro register write ok
		do {
			ret = read_reg(0x1030, 16, &micro_hnd);
			RET_IF_NONZERO(ret);
		} while(!(micro_hnd & 0x1));
		ret = write_reg(0x102e, 16, p_pars++);
		RET_IF_NONZERO(ret);
	}
	return 0;
}

int v1190::read_micro( uint16_t ope_code, uint16_t* p_pars, int n)
{
	uint16_t micro_hnd= 0;
	int ret;
	// wait for micro register write ok
	do {
		ret = read_reg(0x1030, 16, &micro_hnd);
		RET_IF_NONZERO(ret);
	} while(!(micro_hnd & 0x1));
	// write opcode to micro register
	ret = write_reg(0x102e, 16, &ope_code);
	RET_IF_NONZERO(ret);
	// read ope code 
	while(n--) {
		// wait for micro register write ok
		do {
			ret = read_reg(0x1030, 16, &micro_hnd);
			RET_IF_NONZERO(ret);
		} while(!(micro_hnd & 0x2));
		ret = read_reg(0x102e, 16, p_pars++);
		RET_IF_NONZERO(ret);
	}
	return 0;
	
}

int v1190::on_start()
{
	/* clear the fifo, we do it by reading all data out, we don't use the
	 * software clear because that also resets the coase time counter,
	 * which is not wanted. */
	int ret, sz_in, sz_out;
	char dst[8192];

	/* Note: berr has to be enabled,*/
	sz_in = 8192;
	do {
		/* am = A32_S_BLT is assumed. */
		ret = read_evt_blt(0x0f, dst, sz_in, &sz_out, 0, 0);
		if (ret != -E_VME_BUS)
			RET_IF_NONZERO(ret);
		/* debug...*/
//		std::cout<<"sz_out = "<<sz_out<<std::endl;
		
	} while (sz_out != 0);

	/* we also want to reset the event counter in case of merging using
	 * event counter. However, we don't do it softwarely, we do it
	 * hardwarely, i.e. we should send the pulse to front panel CLR. */

	return 0;
}

int v1190::on_stop()
{
	return 0;
}

int v1190::if_trig(bool& x)
{
	uint16_t data_ready;
	int ret;

	ret = read_reg(0x1020, 16, &data_ready);
	RET_IF_NONZERO(ret);

	if (data_ready)
		x = true;
	else
		x= false;

	return 0;
}
