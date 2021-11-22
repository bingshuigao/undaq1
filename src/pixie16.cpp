#include "pixie16.h"
#include <unistd.h>

pixie16::pixie16()
{
	/* set buffer offset */
	buf_off = 0x0;
	
	name = "pixie16";
	mod_id = 14;
	clk_freq = DEF_PIXIE_CLK;
	tot_mod_num = 1000;
	this_mod_num = 1000;
}

int pixie16::read_single_evt(int am, uint32_t *evt, int* sz_out)
{
	return -E_DONT_CALL;
}

int pixie16::enable_mcst(uint32_t mcst_addr)
{
	return -E_DONT_CALL;
}

int pixie16::enable_cblt(uint32_t cblt_addr, int first, int last)
{
	return -E_DONT_CALL;
}
int pixie16::get_cblt_conf(uint16_t* addr, int* cblt_enable, int* cblt_first,
		int* cblt_last)
{
	if (cblt_enable)
		*cblt_enable = 1;
	if (cblt_first) 
		*cblt_first = (slot_n == 0) ? 1 : 0;
	if (cblt_last) 
		*cblt_last = (slot_n == 15) ? 1 : 0;
	if (addr) 
		*addr = this_mod_num;
	return 0;
}

int pixie16::if_trig(bool& x)
{
	int ret, i;
	unsigned int n_word;

	x = false;
	for (i = 0; i < tot_mod_num; i++) {
		ret = Pixie16CheckExternalFIFOStatus(&n_word, i);
		if (ret) {
			return -E_PIXIE_CHECK_FIFO_STAT;
		}
		if (n_word > 0) {
			x = true;
			return 0;
		}
	}
	return 0;
}

int pixie16::on_start()
{
	/* to start all the pixie modules simutaniously, we use the
	 * Pixie16StartListModeRun function and the total number of installed
	 * modules has to be known. Also this function should be called only
	 * once, therefore, we call this function only for 'trigger module'. */
	int ret;

	if (trig_mod) {
		ret = Pixie16StartListModeRun(tot_mod_num, 0x100, NEW_RUN);
		if (ret) {
			return -E_PIXIE_START_RUN;
		}
	}

	return 0;
}

int pixie16::on_stop()
{
	/* similarly as the on_start(), only need to stop the run for the
	 * 'trigger module' */
	int ret;
	if (trig_mod) {
		ret = Pixie16EndRun(0);
		if (ret) {
			return -E_PIXIE_STOP_RUN;
		}
	}

	return 0;
}
