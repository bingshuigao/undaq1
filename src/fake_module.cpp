#include "fake_module.h"
#include <unistd.h>

fake_module::fake_module()
{
	/* set buffer offset */
	buf_off = 0x0;
	
	name = "fake_module";
	mod_id = 10;
	clk_freq = DEF_MADC32_CLK;
}

