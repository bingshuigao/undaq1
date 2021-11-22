#include "pixie16_ctl.h"
//#include "CAENVMElib.h"
#include "err_code.h"
#include <iostream>



/* open the device 
 * @par a pointer of parameters 
 * @return 0 if succeed, non-zero error codes are returned if error */
int pixie16_ctl::open(void* par)
{
	struct pixie16_ctl_open_par* tmp = (struct pixie16_ctl_open_par*)par;
	int ret;
	
	mod_num = tmp->mod_num;
	if (Pixie16InitSystem(tmp->mod_num, tmp->pxi_slot_map, 0)) 
		return -E_PIXIE_INIT;
	ret = Pixie16BootModule(
			tmp->ComFPGAConfigFile, /* ComFPGA configuration file */
			tmp->SPFPGAConfigFile,  /* SPFPGA configuration file */ 
			tmp->TrigFPGAConfigFile,/* trigger FPGA file */          
			tmp->DSPCodeFile,       /* DSP code file */             
			tmp->DSPParFile,        /* DSP parameter file */        
			tmp->DSPVarFile,        /* DSP variable names file */   
			tmp->mod_num,           /* boot all modules */
			0x7f                    /* boot all chips */
			);
	switch (ret) {
	case -1:
		return -E_PIXIE_MOD_NUM;
	case -2:
	case -5:
		return -E_ComFPGAConfigFile;
	case -10:
	case -13:
		return -E_SPFPGAConfigFile;
	case -16:
		return -E_DSPCodeFile;
	case -17:
	case -18:
		return -E_DSPParFile;
	case -19:
		return -E_DSPVarFile;
	default:
		return -E_PIXIE_GENERAL;
	}
	return 0;
}


/* close the device 
 * @return 0 if succeed, non-zero error codes are returned if error */
int pixie16_ctl::close()
{
	int ret;

	ret = Pixie16ExitSystem(mod_num);
	switch (ret) {
	case -1:
		return -E_PIXIE_MOD_NUM;
	case 0:
		return 0;
	default:
		return -E_PIXIE_GENERAL;
	}
}

int pixie16_ctl::blt_read(unsigned long addr, void* buf, int sz_in, int* sz_out)
{
	int ret;
	unsigned short mod_n = addr;
	unsigned int n_word, n_word2;
	
	/* First, we need to check fifo status to get number of words */
	ret = Pixie16CheckExternalFIFOStatus(&n_word, mod_n);
	if (ret) 
		return -E_PIXIE_MOD_NUM;
	/* need to reserve space for a whole evt, just in case we need a second
	 * readout */
	sz_in -= 4*evt_max_sz;
	if (sz_in <= 0) 
		return -E_BLT_BUFF_SZ;
	if ((n_word*4) > sz_in) 
		n_word = sz_in/4;

	/* Second, do a readout */
	ret = Pixie16ReadDataFromExternalFIFO((unsigned int*)buf, n_word, mod_n);
	*sz_out = n_word*4;
	if (ret == -1) 
		return -E_PIXIE_MOD_NUM;
	else if (ret == -2)
		return -E_PIXIE_GENERAL;

	/* third, check if a second readout is needed to assure event-align */
	n_word2 = check_evt_align((unsigned int*)buf, n_word);
	if (n_word2 == 0) 
		return 0;
	while (1) {
		unsigned int n_word3;
		ret = Pixie16CheckExternalFIFOStatus(&n_word3, mod_n);
		if (ret) 
			return -E_PIXIE_MOD_NUM;
		if (n_word3 >= n_word2) 
			break;
	}
	ret = Pixie16ReadDataFromExternalFIFO(((unsigned int*)buf)+n_word, n_word2, mod_n);
	*sz_out += n_word2*4;
	if (ret == -1) 
		return -E_PIXIE_MOD_NUM;
	else if (ret == -2)
		return -E_PIXIE_GENERAL;
	if (*sz_out > sz_in) 
		return -E_BLT_BUFF_SZ;

	return 0;
}

unsigned int pixie16_ctl::check_evt_align(uint32_t* buf, unsigned int n_word)
{
	uint32_t evt_len, rem_len;

	rem_len = n_word;
	evt_len = (buf[0]>>17) & 0x3fff;
	while (rem_len > evt_len) {
		buf += evt_len;
		rem_len -= evt_len;
		evt_len = (buf[0]>>17) & 0x3fff;
	}
	return evt_len - rem_len;
}

int pixie16_ctl::mblt_read(unsigned long addr, void* buf, int sz_in, int* sz_out)
{
	return blt_read(addr, buf, sz_in, sz_out);
}

int pixie16_ctl::send_pulse(bool invt)
{
	/* To be implemented */
	return 0;
}

int pixie16_ctl::send_pulse1(bool invt)
{
	/* To be implemented */
	return 0;
}


int pixie16_ctl::send_pulse2(bool invt)
{
	/* To be implemented */
	return 0;
}
