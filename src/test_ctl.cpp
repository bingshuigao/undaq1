#include "test_ctl.h"
#include "err_code.h"
#include <stdlib.h>
#include <iostream>




/* (m)blt read from a module.
 * @param addr vme bus address 
 * @param buf buffer where the data will be read out to
 * @param sz_in size of data to read (in bytes)
 * @param sz_out size of data actually read (in bytes).
 * @return 0 if succeed, non-zero error codes in case of error. */
int test_ctl::blt_read(unsigned long addr, void* buf, int sz_in, int* sz_out)
{
	/* we mimic an madc32 data here */
	static long ts = 0;

	uint32_t hd = 0x40000003;
	uint32_t data = 0x4000000;
	uint32_t ett = 0x480000;
	uint32_t trail = 0xc0000000;
	uint32_t *p_dw = (uint32_t*)buf;

	*sz_out = 16;
	p_dw[0] = hd;
	p_dw[1] = data + (int)(8000.*rand()/RAND_MAX);
	p_dw[2] = ett + ((ts>>32)&0xffff);
	p_dw[3] = trail + (ts&0x3fffffff);
	ts++;

//	std::cout<<"okkkkkkkkk"<<std::endl;


//	return -E_DATA_MADC32;
	return 0;
}
