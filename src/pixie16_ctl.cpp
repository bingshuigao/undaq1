#include "pixie16_ctl.h"
#include <stdio.h>
//#include "CAENVMElib.h"
#include "err_code.h"
#include <iostream>
#include <string.h>
#include <math.h>


int pixie16_ctl::read_dsp_par()
{
	uint32_t dsp_par[1280];
	int i, j;
	FILE* fp = fopen("pixie16_firmware/initial_par.set", "rb");

	if (!fp) {
		printf("cannot open initial_par.set\n");
		return -E_DSPParFile;
	}
	if (1280 != fread(dsp_par, 4, 1280, fp)) {
		fclose(fp);
		return -E_DSPParFile;
	}
	fclose(fp);

	for (i = 0; i < MAX_CRATE; i++) 
		memcpy(Pixie_Devices[i].DSP_Parameter_Values, dsp_par, 4*1280);
	return 0;
}

/* open the device 
 * @par a pointer of parameters 
 * @return 0 if succeed, non-zero error codes are returned if error */
int pixie16_ctl::open(void* par)
{
	struct pixie16_ctl_open_par* tmp = (struct pixie16_ctl_open_par*)par;
	int ret, i;
	unsigned short bit_msk;

	strcpy(ComFPGAConfigFile, tmp->ComFPGAConfigFile);
	strcpy(SPFPGAConfigFile, tmp->SPFPGAConfigFile);
	strcpy(TrigFPGAConfigFile, tmp->TrigFPGAConfigFile);
	strcpy(DSPCodeFile, tmp->DSPCodeFile);
	strcpy(DSPParFile, tmp->DSPParFile);
	strcpy(DSPVarFile, tmp->DSPVarFile);
	if (tmp->DSPParFile[0] == 0) {
		/* dsp parameter file name is empty, don't boot the module now
		 * but at later times. */
		ret = read_dsp_par();
		RET_IF_NONZERO(ret);
		strcpy(DSPParFile, "./pixie16_firmware/dsp.set");
		reg_acc = 1;
	}
	else {
		reg_acc = 0;
	}

	mod_num = tmp->mod_num;
	if (mod_num > MAX_CRATE)
		return -E_MAX_CRATE;
	if (myPixie_Init_DSPVarAddress(DSPVarFile, 0)) 
		return -E_DSPVarFile;
	for (i = 1; i < mod_num; i++) {
		if(myPixie_Copy_DSPVarAddress(0, i))
			return -E_DSPVarFile;
	}

	memcpy(pxi_slot_map, tmp->pxi_slot_map, tmp->mod_num*4);
	if (Pixie16InitSystem(tmp->mod_num, tmp->pxi_slot_map, 0)) 
		return -E_PIXIE_INIT;
	for (i = 0; i < mod_num; i++) {
		if (Pixie16ReadModuleInfo(i, &Module_Information[i].Module_Rev,
					&Module_Information[i].Module_SerNum,
					&Module_Information[i].Module_ADCBits,
					&Module_Information[i].Module_ADCMSPS))
			return -E_PIXIE_GENERAL;
	}
	
	if (!reg_acc) {
		/* if dsp par file name is not empty, boot the module now,
		 * otherwise postpone to later times (try_load_dsp()). */
		ret = boot_modules();
		RET_IF_NONZERO(ret);
	}
	
	return 0;
}

int pixie16_ctl::boot_modules()
{
	int ret;
	ret = Pixie16BootModule(
			ComFPGAConfigFile, /* ComFPGA configuration file */
			SPFPGAConfigFile,  /* SPFPGA configuration file */ 
			TrigFPGAConfigFile,/* trigger FPGA file */          
			DSPCodeFile,       /* DSP code file */             
			DSPParFile,        /* DSP parameter file */        
			DSPVarFile,        /* DSP variable names file */   
			mod_num,           /* boot all modules */
			0x7f               /* boot which chips */
			);
	
	if (ret != 0) {
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
	}
	
	if (Pixie16AdjustOffsets(mod_num))
		return -E_PIXIE_GENERAL;

	return 0;
}

pixie16_ctl::pixie16_ctl()
{
	name = "pixie16_ctl";
	evt_max_sz = DEF_PIXIE16_EVT_MAX;
	fifo_rd_thresh = DEF_PIXIE_FIFO_THRESH;
	reset_clk = 1;
	
	int i, j;
	for (i = 0; i < MAX_CRATE; i++) {
		for (j = 0; j < 16; j++) {
			T_Filter_Range[i][j] = -1;
			E_Filter_Range[i][j] = -1;
			trace_length[i][j] = -1.;
			trace_delay[i][j] = -1.;
			T_Risetime_us[i][j] = -1.;
			T_FlatTop_us[i][j] = -1.;
			T_Threshold[i][j] = -1.;
			E_FlatTop_us[i][j] = -1.;
			E_Risetime_us[i][j] = -1.;
			bl_cut[i][j] = -1.;
			auto_blcut[i][j] = -1;
			group0_trigLMR[i][j] = -1;
			group1_trigLMR[i][j] = -1;
			group2_trigLMR[i][j] = -1;
			group3_trigLMR[i][j] = -1;
			 group0_trigCh[i][j] = -1;
			 group1_trigCh[i][j] = -1;
			 group2_trigCh[i][j] = -1;
			 group3_trigCh[i][j] = -1;
		}
	}
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

int pixie16_ctl::try_load_dsp()
{
	FILE* fp;
	int i, ret;
	char buf[1000];

	if (reg_acc == 0) 
		return 0;

	/* first, let's save the dsp parameter into file:
	 * "./pixie16_firmware/dsp.set */
	sprintf(buf, "%s", "./pixie16_firmware/dsp.set");
	fp = fopen(buf, "wb");
	if (!fp) 
		return -E_PIXIE_LOAD_DSP;
	for (i = 0; i < PRESET_MAX_MODULES; i++) {
		int n;
		n = (i < MAX_CRATE) ? i : 0;
		if (1280 != fwrite(Pixie_Devices[n].DSP_Parameter_Values, 4,
					1280, fp))
			return -E_PIXIE_LOAD_DSP;
	}
	
	/* now let's load the dsp parameters. Note: instead of using the
	 * Pixie16LoadDSPParametersFromFile() function, we use the
	 * Pixie16BootModule() function. Because I found the former one may
	 * cause synchronization problems.  */
	ret = boot_modules();
	RET_IF_NONZERO(ret);

	return 0;
}

int pixie16_ctl::blt_read(unsigned long addr, void* buf, int sz_in, int* sz_out)
{
	int ret;
	unsigned short mod_n = am;
	unsigned int n_word, n_word2, n_word3;
	
	/* First, we need to check fifo status to get number of words */
	ret = Pixie16CheckExternalFIFOStatus(&n_word, mod_n);
	if (ret) 
		return -E_PIXIE_MOD_NUM;
	/* the n_word could be zero, e.g. in case of flushing the buffer at the
	 * stop of run. In this case, NEVER try to read the Pixie16 FIFO, or
	 * else you may halt the kernel! */
	if (n_word == 0) {
		*sz_out = 0;
		return 0;
	}

	/* need to reserve space for a whole evt, just in case we need a second
	 * readout, the -8 is due to possible artifical read */
	sz_in -= 4*evt_max_sz;
	sz_in -= 8;
	if (sz_in <= 0) 
		return -E_BLT_BUFF_SZ;
	if ((n_word*4) > sz_in) 
		n_word = sz_in/4;

	/* Second, do a readout */
	n_word3 = artifical_read((unsigned int*)buf);
	ret = Pixie16ReadDataFromExternalFIFO(((unsigned int*)buf)+n_word3,
			n_word, mod_n);
	n_word += n_word3;
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
		if (n_word3 >= ((n_word2>2) ? n_word2 : 3)) 
			break;
	}
	ret = Pixie16ReadDataFromExternalFIFO(((unsigned int*)buf)+n_word,
			(n_word2>2) ? n_word2 : 3, mod_n);
	/* why we need this artifical thing? see comments in the artifical_read */
	artifical_write(((unsigned int*)buf)+n_word, n_word2);
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

int pixie16_ctl::write_reg(long reg, void* data)
{
	/* when writing into pixie16 module register values, the pixie16 api
	 * will determine if the value is appropriate, which may also depend on
	 * other register values. Although we have taken care of this in the
	 * config.py GUI, the order in which the registers are written still
	 * matters here (because the pixie16 api will read other register
	 * values from the module if dependences exists). In this function
	 * here, we have analyzed the api functions for each register setting
	 * to resolve the dependences and made sure correct values will be set
	 * for these registers. */
	
	unsigned short mod_n = am;
	unsigned short ch_n = dw;
	uint64_t val64 = ((uint64_t*)data)[0];
	uint32_t val32 = ((uint32_t*)data)[0];
	uint16_t val16 = ((uint16_t*)data)[0];
	double   valf64  = val64/1000.; /* see comments in pixie16.py */
	uint16_t bit_n, bit_val, bit_n2, bit_val2;
	int ret, i;
	char buf[100];

	/* the register access is way too slow, we give up this approach. */
	if ((reg < 1000) && (!reg_acc)) 
		return 0;

	switch (reg) {
	case 0:
		/* module number, we do nother here */
		break;
	case 1:
		/* energy filter range */
		val64 += 1;
		E_Filter_Range[mod_n][ch_n] = val64;
		return do_conf_filter_pars(mod_n, ch_n);
	case 2:
		/* timing filter range */
		val64 += 1;
		T_Filter_Range[mod_n][ch_n] = val64;
		return do_conf_filter_pars(mod_n, ch_n);
	case 3:
		/* CPLD, Module Control RegisterB bit0. Although it is a
		 * configurable parameter, the first module should set it to 1,
		 * other ones should have it 0. So we just ignore configuration
		 * and set it this way. */
		if (mod_n == 0) 
			bit_val = 1;
		else
			bit_val = 0;
		bit_n = 0;
		return set_modCSRB_bit(mod_n, bit_n, bit_val);
	case 4:
		/* sort event, module control registerB, bit12 */
		bit_n = 12;
		bit_val = val64;
		return set_modCSRB_bit(mod_n, bit_n, bit_val);
	case 5:
		/* inhibit enable, module control registerB, bit10 */
		bit_n = 10;
		bit_val = val64;
		return set_modCSRB_bit(mod_n, bit_n, bit_val);
	case 6:
		/* multicrate enable, module control registerB, bit11 */
		bit_n = 11;
		bit_val = val64;
		return set_modCSRB_bit(mod_n, bit_n, bit_val);
	case 7:
		/* XDT (us) */
		if (myPixie16WriteSglChanPar("XDT", valf64, mod_n, ch_n))
			return -E_PIXIE_GENERAL;
		break;
	case 8:
		/* if good channel */
		bit_n = 2;
		bit_val = val64;
		return set_chCSRA_bit(mod_n, ch_n, bit_n, bit_val);
	case 9:
		/* Channel Polarity */
		bit_n = 5;
		bit_val = val64;
		return set_chCSRA_bit(mod_n, ch_n, bit_n, bit_val);
	case 10:
		/* channel gain */
		bit_n = 14;
		bit_val = val64;
		return set_chCSRA_bit(mod_n, ch_n, bit_n, bit_val);
	case 11:
		/* adjust base line */
		valf64 *= 1000.; /* because it was integer in the config GUI */
		if (myPixie16WriteSglChanPar("BASELINE_PERCENT", valf64, mod_n,
					ch_n))
			return -E_PIXIE_GENERAL;
		//if (Pixie16AdjustOffsets(mod_n))
			//return -E_PIXIE_GENERAL;
		break;
	case 12:
		/* fast filter rise time (us) */
		T_Risetime_us[mod_n][ch_n] = valf64;
		return do_conf_filter_pars(mod_n, ch_n);
	case 13:
		/* fast filter flat top (us) */
		T_FlatTop_us[mod_n][ch_n] = valf64;
		return do_conf_filter_pars(mod_n, ch_n);
	case 14:
		/* trigger threshold */
		valf64 *= 1000.; /* because it was integer in the config GUI */
		T_Threshold[mod_n][ch_n] = valf64;
		return do_conf_filter_pars(mod_n, ch_n);
	case 15:
		/* energy filter rise time (us) */
		E_Risetime_us[mod_n][ch_n] = valf64;
		return do_conf_filter_pars(mod_n, ch_n);
	case 16:
		/* energy filter flat top (us) */
		E_FlatTop_us[mod_n][ch_n] = valf64;
		return do_conf_filter_pars(mod_n, ch_n);
	case 17:
		/* preamplifier decay const (Tau (us))*/
		if (myPixie16WriteSglChanPar("TAU", valf64, mod_n, ch_n))
			return -E_PIXIE_GENERAL;
		break;
	case 18:
		/* pile up (channel csra bit15,16) */
		bit_n = 15;
		bit_val = val64 & 0x1;
		if (set_chCSRA_bit(mod_n, ch_n, bit_n, bit_val))
			return -E_PIXIE_GENERAL;
		bit_n = 16;
		bit_val = (val64>>1) & 0x1;
		return set_chCSRA_bit(mod_n, ch_n, bit_n, bit_val);
	case 19:
		/* sync data (channel csra bit4) */
		bit_n = 4;
		bit_val = val64;
		return set_chCSRA_bit(mod_n, ch_n, bit_n, bit_val);
	case 20:
		/* External timestamps in event header (channel csra bit21) */
		bit_n = 21;
		bit_val = val64;
		return set_chCSRA_bit(mod_n, ch_n, bit_n, bit_val);
	case 21:
		/* Enable capture raw energy sums and baselines (channel csra
		 * bit12) */
		bit_n = 12;
		bit_val = val64;
		return set_chCSRA_bit(mod_n, ch_n, bit_n, bit_val);
	case 22:
		/* Enable CFD trigger mode (channel csra bit10) */
		bit_n = 10;
		bit_val = val64;
		return set_chCSRA_bit(mod_n, ch_n, bit_n, bit_val);
	case 23:
		/* cfd scaler in the cfd algrithm */
		valf64 *= 1000;
		if (myPixie16WriteSglChanPar("CFDScale", valf64, mod_n, ch_n))
			return -E_PIXIE_GENERAL;
		break;
	case 24:
		/* cfd delay used in the cfd algrithm */
		if (myPixie16WriteSglChanPar("CFDDelay", valf64, mod_n, ch_n))
			return -E_PIXIE_GENERAL;
		break;
	case 25:
		/* cfd threshold in the cfd algrithm */
		valf64 *= 1000;
		if (myPixie16WriteSglChanPar("CFDThresh", valf64, mod_n, ch_n))
			return -E_PIXIE_GENERAL;
		break;
	case 26:
		/* record trace (channel csra bit8) */
		bit_n = 8;
		bit_val = val64;
		return set_chCSRA_bit(mod_n, ch_n, bit_n, bit_val);
	case 27:
		/* trace pretrig (trace delay) (us) */
		trace_delay[mod_n][ch_n] = valf64;
		return do_conf_filter_pars(mod_n, ch_n);
	case 28:
		/* trace length  (us) */
		trace_length[mod_n][ch_n] = valf64;
		return do_conf_filter_pars(mod_n, ch_n);
	case 29:
		/* record qdc (channel csra bit9) */
		bit_n = 9;
		bit_val = val64;
		return set_chCSRA_bit(mod_n, ch_n, bit_n, bit_val);
	case 30:
	case 31:
	case 32:
	case 33:
	case 34:
	case 35:
	case 36:
	case 37:
		/* qdc len0-7 */
		sprintf(buf, "QDCLen%d", reg-30);
		if (myPixie16WriteSglChanPar(buf, valf64, mod_n, ch_n))
			return -E_PIXIE_GENERAL;
		break;
	case 38:
		/* BL cut */
		bl_cut[mod_n][ch_n] = val64;
		return do_conf_blcut(mod_n, ch_n);
	case 39:
		/* channel number, we do nothing here */
		break;
	case 40:
		/* fast trig back plane enable (left, bit0-15) */
		return set_modreg_bit(mod_n, "FastTrigBackplaneEna", 0, 16, val64);
	case 41:
		/* fast trig back plane enable (right, bit16-31) */
		return set_modreg_bit(mod_n, "FastTrigBackplaneEna", 16, 16, val64);
	case 42:
		/* channel fast trigger selection (channel csra bit0, bit18.
		 * this is based on Wang Jianguo's code) */
		bit_n = 0;
		bit_n2 = 18;
		if (val64 == 0) {
			bit_val = 0;
			bit_val2 = 0;
		}
		else if (val64 == 1) {
			bit_val = 1;
			bit_val2 = 0;
		}
		else {
			bit_val = 0;
			bit_val2 = 1;
		}
		if (set_chCSRA_bit(mod_n, ch_n, bit_n, bit_val))
			return -E_PIXIE_GENERAL;
		return set_chCSRA_bit(mod_n, ch_n, bit_n2, bit_val2);
	case 43:
		/* enable channel VT (channel csra bit13) */
		bit_n = 13;
		bit_val = val64;
		return set_chCSRA_bit(mod_n, ch_n, bit_n, bit_val);
	case 44:
		/* channel vt selection (channel csra bit3) */
		bit_n = 3;
		bit_val = val64;
		return set_chCSRA_bit(mod_n, ch_n, bit_n, bit_val);
	case 45:
		/* enable module VT (channel csra bit11) */
		bit_n = 11;
		bit_val = val64;
		return set_chCSRA_bit(mod_n, ch_n, bit_n, bit_val);
	case 46:
		/* module vt selection (channel csra bit1) */
		bit_n = 1;
		bit_val = val64;
		return set_chCSRA_bit(mod_n, ch_n, bit_n, bit_val);
	case 47:
		/* enable channel veto (channel csra bit6). if enabled, use OR
		 * of module veto and channel veto signals  */
		bit_n = 6;
		bit_val = val64;
		return set_chCSRA_bit(mod_n, ch_n, bit_n, bit_val);
	case 48:
		/* channel veto signal selection (channel csra bit19) */
		bit_n = 19;
		bit_val = val64;
		return set_chCSRA_bit(mod_n, ch_n, bit_n, bit_val);
	case 49:
		/* channel module veto signal selection (channel csra bit20) */
		bit_n = 20;
		bit_val = val64;
		return set_chCSRA_bit(mod_n, ch_n, bit_n, bit_val);
	case 50:
		/* channel trigger selection (if used as channel VT).
		 * MultMaskHix bit16 and bit31, TrigConfig2 bit24-26. The
		 * following code is based on Wang Jianguo's code */
		if (myPixie16ReadSglChanPar("MultiplicityMaskH", &valf64, mod_n, ch_n))
			return -E_PIXIE_GENERAL;
		if (val64 >= 2) {
			valf64 = set_bits_double(valf64, 31, 1, 1);
			uint32_t data1;
			uint16_t ChNum=ch_n;
			ChNum /= 4;
			if (myPixie16ReadSglModPar("TrigConfig2", &data1, mod_n))
				return -E_PIXIE_GENERAL;
			if (val64 == 2)
				data1 = set_bits_int(data1, 24+ChNum, 1, 1);
			else
				data1 = set_bits_int(data1, 24+ChNum, 1, 0);
			if (myPixie16WriteSglModPar("TrigConfig2", data1, mod_n))
				return -E_PIXIE_GENERAL;
		}
		else {
			if (val64 == 0) {
				valf64 = set_bits_double(valf64, 16, 1, 1);
				valf64 = set_bits_double(valf64, 31, 1, 0);
			}
			else {
				valf64 = set_bits_double(valf64, 16, 1, 0);
				valf64 = set_bits_double(valf64, 31, 1, 0);
			}
		}
		if (myPixie16WriteSglChanPar("MultiplicityMaskH", valf64, mod_n, ch_n))
			return -E_PIXIE_GENERAL;
		break;
	case 51:
		/* dsp channel parameter MultiplicityMaskH bit0-15 */
		return set_chreg_bit(mod_n, ch_n, "MultiplicityMaskH", 0, 16, val64);
	case 52:
		/* dsp channel parameter MultiplicityMaskL bit0-15 */
		return set_chreg_bit(mod_n, ch_n, "MultiplicityMaskL", 0, 16, val64);
	case 53:
		/* dsp channel parameter MultiplicityMaskL bit16-31 */
		return set_chreg_bit(mod_n, ch_n, "MultiplicityMaskL", 16, 16, val64);
	case 54:
		/* dsp channel parameter MultiplicityMaskH bits17-21
		 * (multiplicity threshold) */
		return set_chreg_bit(mod_n, ch_n, "MultiplicityMaskH", 17, 5, val64);
	case 55:
		/* dsp channel parameter MultiplicityMaskH bits28-30
		 * (coincident trigger threshold of the left neighbor) */
		return set_chreg_bit(mod_n, ch_n, "MultiplicityMaskH", 28, 3, val64);
	case 56:
		/* dsp channel parameter MultiplicityMaskH bits22-24
		 * (coincident trigger threshold of the module itself ) */
		return set_chreg_bit(mod_n, ch_n, "MultiplicityMaskH", 22, 3, val64);
	case 57:
		/* dsp channel parameter MultiplicityMaskH bits25-27
		 * (coincident trigger threshold of the right neighbor ) */
		return set_chreg_bit(mod_n, ch_n, "MultiplicityMaskH", 25, 3, val64);
	case 58:
		/* Group0 trigger selection TrigConfig2 bit16-17 */
		group0_trigLMR[mod_n][ch_n] = val64;
		return do_conf_group_trig(0, mod_n, ch_n);
	//	return set_modreg_bit(mod_n, "TrigConfig2", 16, 2, val64);
	case 59:
		/* Group1 trigger selection TrigConfig2 bit18-19 */
		group1_trigLMR[mod_n][ch_n] = val64;
		return do_conf_group_trig(1, mod_n, ch_n);
	//	return set_modreg_bit(mod_n, "TrigConfig2", 18, 2, val64);
	case 60:
		/* Group2 trigger selection TrigConfig2 bit20-21 */
		group2_trigLMR[mod_n][ch_n] = val64;
		return do_conf_group_trig(2, mod_n, ch_n);
	//	return set_modreg_bit(mod_n, "TrigConfig2", 20, 2, val64);
	case 61:
		/* Group3 trigger selection TrigConfig2 bit22-23 */
		group3_trigLMR[mod_n][ch_n] = val64;
		return do_conf_group_trig(3, mod_n, ch_n);
	//	return set_modreg_bit(mod_n, "TrigConfig2", 22, 2, val64);
	case 62:
		/* Group0 trigger channel selection TrigConfig1 bit16-17 */
		group0_trigCh[mod_n][ch_n] = val64;
		return do_conf_group_trig(0, mod_n, ch_n);
		//return set_chreg_bit(mod_n, ch_n, "TrigConfig2", 16, 2, val64);
	case 63:
		/* Group1 trigger channel selection TrigConfig1 bit18-19 */
		group1_trigCh[mod_n][ch_n] = val64;
		return do_conf_group_trig(1, mod_n, ch_n);
		//return set_chreg_bit(mod_n, ch_n, "TrigConfig2", 18, 2, val64);
	case 64:
		/* Group2 trigger channel selection TrigConfig2 bit20-21 */
		group2_trigCh[mod_n][ch_n] = val64;
		return do_conf_group_trig(2, mod_n, ch_n);
		//return set_chreg_bit(mod_n, ch_n, "TrigConfig2", 20, 2, val64);
	case 65:
		/* Group3 trigger channel selection TrigConfig2 bit22-23 */
		group3_trigCh[mod_n][ch_n] = val64;
		return do_conf_group_trig(3, mod_n, ch_n);
		//return set_chreg_bit(mod_n, ch_n, "TrigConfig2", 22, 2, val64);
	case 66:
		/* LCFT Delay */
		if (myPixie16WriteSglChanPar("FtrigoutDelay", valf64, mod_n, ch_n))
			return -E_PIXIE_GENERAL;
		break;
	case 67:
		/* LCFT Width  */
		if (myPixie16WriteSglChanPar("FASTTRIGBACKLEN", valf64, mod_n, ch_n))
			return -E_PIXIE_GENERAL;
		break;
	case 68:
		/* fifo delay */
		if (myPixie16WriteSglChanPar("ExternDelayLen", valf64, mod_n, ch_n))
			return -E_PIXIE_GENERAL;
		break;
	case 69:
		/* ChVT Width */
		if (myPixie16WriteSglChanPar("ChanTrigStretch", valf64, mod_n, ch_n))
			return -E_PIXIE_GENERAL;
		break;
	case 70:
		/* ModVT Width */
		if (myPixie16WriteSglChanPar("ExtTrigStretch", valf64, mod_n, ch_n))
			return -E_PIXIE_GENERAL;
		break;
	case 71:
		/* Veto Width */
		if (myPixie16WriteSglChanPar("VetoStretch", valf64, mod_n, ch_n))
			return -E_PIXIE_GENERAL;
		break;
	case 72:
		/* select external trig validation signal source */
		return set_modreg_bit(mod_n, "TrigConfig0", 28, 4, val64);
	case 73:
		bit_n = 8;
		bit_val = val64;
		return set_modCSRB_bit(mod_n, bit_n, bit_val);
	case 74:
		/* IntVT_Sel */
		return set_modreg_bit(mod_n, "TrigConfig0", 8, 4, val64);
	case 75:
		/* ChanTrig_Sel */
		return set_modreg_bit(mod_n, "TrigConfig2", 28, 4, val64);
	case 76:
		/* Ext_FT_In */
		return set_modreg_bit(mod_n, "TrigConfig0", 4, 4, val64);
	case 77:
		/* Ext_FT_Sel */
		bit_n = 7;
		bit_val = val64;
		return set_modCSRB_bit(mod_n, bit_n, bit_val);
	case 78:
		/* IntFT_Sel */
		return set_modreg_bit(mod_n, "TrigConfig0", 0, 4, val64);
	case 79:
		/* Crate Master */
		bit_n = 6;
		bit_val = val64;
		return set_modCSRB_bit(mod_n, bit_n, bit_val);
	case 80:
		/* MulCrate Master */
		bit_n = 4;
		bit_val = val64;
		return set_modCSRB_bit(mod_n, bit_n, bit_val);
	case 81:
		/* Mod_VT */
		return set_modreg_bit(mod_n, "TrigConfig0", 26, 2, val64);
	case 82:
		/* Mod_FT */
		return set_modreg_bit(mod_n, "TrigConfig0", 24, 2, val64);
	case 83:
		/* TTL FP Ena */
		return set_modreg_bit(mod_n, "TrigConfig0", 15, 1, val64);
	case 84:
		/* Ch_Sel */
		return set_modreg_bit(mod_n, "TrigConfig0", 16, 4, val64);
	case 85:
		/* Output_Sel */
		return set_modreg_bit(mod_n, "TrigConfig0", 12, 3, val64);
	case 86:
		/* Fo7_Sel */
		return set_modreg_bit(mod_n, "TrigConfig0", 20, 4, val64);
	case 87:
		/* Auto BL cut */
		auto_blcut[mod_n][ch_n] = val64;
		return do_conf_blcut(mod_n, ch_n);
	case 88:
		/* module revision, do nothing */
		return 0;
	case 1003:
		/* start a new run (based on Wang Jianguo's code) */
	
		/* reset n_word_art */
		n_word_art = 0;

		/* start runs synchronisely */
		mod_n = 0;
		val32 = 1;
		sprintf(buf, "%s", "SYNCH_WAIT");
		if (Pixie16WriteSglModPar(buf, val32, mod_n)) 
			return -E_PIXIE_GENERAL;
		
		/* reset clock */
		if (reset_clk) {
			mod_n = 0;
			val32 = 0;
			sprintf(buf, "%s", "IN_SYNCH");
			if (Pixie16WriteSglModPar(buf, val32, mod_n)) 
				return -E_PIXIE_GENERAL;
		}

		/* start list mode run (NEW_RUN) */
		if (Pixie16StartListModeRun(mod_num,LIST_MODE_RUN,NEW_RUN)) 
			return -E_PIXIE_GENERAL;

		/* wait until all modules have started the new run */
		for (i = 0; i < mod_num; i++) {
			while (1) {
				ret = Pixie16CheckRunStatus(i);
				if (ret == 1) 
					break;
				if (ret == -1) 
					return -E_PIXIE_GENERAL;
			}
		}

		/* check synchronization */
		mod_n = 0;
		sprintf(buf, "%s", "IN_SYNCH");
		if (Pixie16ReadSglModPar(buf, &val32, mod_n))
			return -E_PIXIE_GENERAL;
		if (val32 != 1) 
			return -E_PIXIE_SYNC;

		break;
	case 1007:
		if (Pixie16EndRun(0)) 
			return -E_PIXIE_GENERAL;
		/* make sure all modules stopped */
		for (i = 0; i < mod_num; i++) {
			while (1) {
				ret = Pixie16CheckRunStatus(i);
				if (ret == 0) 
					break;
				if (ret == -1) 
					return -E_PIXIE_GENERAL;
			}
		}
		break;
	case 1008:
		/* dsp slot number */
		sprintf(buf, "%s", "SlotID");
		if (reg_acc) 
			ret = myPixie16WriteSglModPar(buf, val64, mod_n);
		else
			ret = Pixie16WriteSglModPar(buf, val64, mod_n);
		if (ret)
			return -E_PIXIE_GENERAL;
		break;
	case 1009:
		/* dsp crate number */
		sprintf(buf, "%s", "CrateID");
		if (reg_acc) 
			ret = myPixie16WriteSglModPar(buf, val64, mod_n);
		else
			ret = Pixie16WriteSglModPar(buf, val64, mod_n);
		if (ret)
			return -E_PIXIE_GENERAL;
		break;
	default:
		return -E_PIXIE_UNKNOW_REG;


	}
	return 0;
}

int pixie16_ctl::set_modreg_bit(unsigned short mod_n, const char* par_name,
		uint16_t start_bit, uint16_t n_bit, uint32_t bit_val)
{
	uint32_t val32;
	if (myPixie16ReadSglModPar(par_name, &val32, mod_n))
		return -E_PIXIE_GENERAL;
	val32 = set_bits_int(val32, start_bit, n_bit, bit_val);
	if (myPixie16WriteSglModPar(par_name, val32, mod_n))
		return -E_PIXIE_GENERAL;
	return 0;
}



int 
pixie16_ctl::set_modCSRB_bit(unsigned short mod_n, uint16_t bit_n, uint16_t val)
{
	return set_modreg_bit(mod_n, "MODULE_CSRB", bit_n, 1, val);
}

int pixie16_ctl::set_chreg_bit(unsigned short mod_n, unsigned short ch_n, const
	char* par_name, uint16_t start_bit, uint16_t n_bit, uint32_t bit_val)
{
	/* It's kind of difficult to understand when the function
	 * Pixie16ReadSglChanPar uses 'double' as its parameter to read/write
	 * some channel registers (e.g. CSRA) while those registers should be
	 * interpreted as uint32_t. By looking into the pixie16 api, I found
	 * that the 'double' was just converted to uint32_t by brute force
	 * like: 
	 * double x; 
	 * uint32_t y = (uint32_t)x;
	 * or the other way round:
	 * uint32_t x;
	 * double y = (double)x;
	 * Since double is a 64-bit number, and the mantissa is represented by
	 * 52 bit, therefore, each 32-bit integer can be accurately represented
	 * by double without information loss. That means:
	 * uint32_t x = any_32bit_num;
	 * double y = (double)x;
	 * uint32_t z = (uint32_t)y
	 * Then z is exactly x.
	 * */

	uint32_t val32, tmp32;
	double valf64;
	if (myPixie16ReadSglChanPar(par_name, &valf64, mod_n, ch_n))
		return -E_PIXIE_GENERAL;
	valf64 = set_bits_double(valf64, start_bit, n_bit, bit_val);
	if (myPixie16WriteSglChanPar(par_name, valf64, mod_n, ch_n))
		return -E_PIXIE_GENERAL;
	
	return 0;
}



int pixie16_ctl::set_chCSRA_bit(unsigned short mod_n, unsigned short ch_n,
		uint16_t bit_n, uint16_t val)
{
	int ret;
	ret = set_chreg_bit(mod_n, ch_n, "CHANNEL_CSRA", bit_n, 1, val);
	RET_IF_NONZERO(ret);
	
	/* According to Wang Jianguo's code, adjusting  baseline should be
	 * performed if CSRA was written */
	//if (Pixie16AdjustOffsets(mod_n))
		//return -E_PIXIE_GENERAL;
	return 0;
}

int pixie16_ctl::read_reg(long reg, void* data)
{
	int i;
	unsigned int n_word_x, n_word_all;

	switch (reg) {
	case 1002:
		/* fifo status of all modules (sum of total number of words of
		 * all modules to be read out) */
		n_word_all = 0;
		for (i = 0; i < mod_num; i++) {
			if (Pixie16CheckExternalFIFOStatus(&n_word_x, i)) 
				return -E_PIXIE_CHECK_FIFO_STAT;
			n_word_all += n_word_x;
		}
		((unsigned int*)data)[0] = n_word_all;
		break;
	default:
		return -E_PIXIE_UNKNOW_REG;
	}
	return 0;
}

int pixie16_ctl::write(unsigned long reg, void* data)
{
	uint64_t val64 = ((uint64_t*) data)[0];
	uint32_t val32 = ((uint32_t*) data)[0];
	uint16_t val16 = ((uint16_t*) data)[0];
	switch (reg) {
	case 0:
		/* max evt size (number of 32-bit words) */
		evt_max_sz = val64;
		break;
	case 1:
		/* fifo threshold */
		fifo_rd_thresh = val64;
		break;
	case 2:
		/* fifo threshold */
		reset_clk = val64;
		break;
	default:
		return -E_PIXIE_UNKNOW_REG;
	}
	return 0;
}

int pixie16_ctl::read(unsigned long reg, void* data)
{
	return -E_NOT_IMPLE;
}

int pixie16_ctl::do_conf_filter_pars(uint16_t mod_n, uint16_t ch_n)
{
	/* the following pars must be present: 
	 * fast filter range, 
	 * slow filter range, 
	 * trace length,
	 * trace delay,
	 * fast rise time, 
	 * fast flattop, 
	 * fast threshold, 
	 * slow rise time,
	 * slow flattop
	 * */
	int t_range = T_Filter_Range[mod_n][ch_n];
	int e_range = E_Filter_Range[mod_n][ch_n];
	double tr_len = trace_length[mod_n][ch_n];
	double tr_del = trace_delay[mod_n][ch_n];
	double t_gap = T_FlatTop_us[mod_n][ch_n];
	double t_rise = T_Risetime_us[mod_n][ch_n];
	double t_thre = T_Threshold[mod_n][ch_n];
	double e_gap = E_FlatTop_us[mod_n][ch_n];
	double e_rise = E_Risetime_us[mod_n][ch_n];
	char buf[100];
	if (t_range < 0) return 0;
	if (t_gap   < 0) return 0;
	if (t_rise  < 0) return 0;
	if (t_thre  < 0) return 0;
	if (t_range < 0) return 0;
	if (e_range < 0) return 0;
	if (tr_len  < 0) return 0;
	if (tr_del  < 0) return 0;
	if (t_gap   < 0) return 0;
	if (t_rise  < 0) return 0;
	if (t_thre  < 0) return 0;
	if (e_gap   < 0) return 0;
	if (e_rise  < 0) return 0;


	/* those parameters should be written into module in the
	 * following order (yes, the flattops are written twice) : */
	int ret = 0;
	ret |= myPixie16WriteSglModPar("FAST_FILTER_RANGE" , t_range, mod_n      );
	ret |= myPixie16WriteSglModPar("SLOW_FILTER_RANGE" , e_range, mod_n      );
	ret |= myPixie16WriteSglChanPar("TRACE_LENGTH"     , tr_len , mod_n, ch_n);
	ret |= myPixie16WriteSglChanPar("TRACE_DELAY"      , tr_del , mod_n, ch_n);
	ret |= myPixie16WriteSglChanPar("TRIGGER_FLATTOP"  , t_gap  , mod_n, ch_n);
	ret |= myPixie16WriteSglChanPar("TRIGGER_RISETIME" , t_rise , mod_n, ch_n);
	ret |= myPixie16WriteSglChanPar("TRIGGER_FLATTOP"  , t_gap  , mod_n, ch_n);
	ret |= myPixie16WriteSglChanPar("TRIGGER_THRESHOLD", t_thre , mod_n, ch_n);
	ret |= myPixie16WriteSglChanPar("ENERGY_FLATTOP"   , e_gap  , mod_n, ch_n);
	ret |= myPixie16WriteSglChanPar("ENERGY_RISETIME"  , e_rise , mod_n, ch_n);
	ret |= myPixie16WriteSglChanPar("ENERGY_FLATTOP"   , e_gap  , mod_n, ch_n);
	if (ret)
		return -E_PIXIE_GENERAL;

	return 0;
}

int pixie16_ctl::do_conf_blcut(uint16_t mod_n, uint16_t ch_n)
{
	/* the following pars must be present: 
	 * bl_cut,
	 * auto_blcut
	 * */
	int if_auto = auto_blcut[mod_n][ch_n];
	double cut_val = bl_cut[mod_n][ch_n];
	char buf[100];
	if (if_auto < 0) return 0;
	if (cut_val < 0) return 0;

	if (if_auto) {
		return 0;
		//unsigned int tmp;
		//if (Pixie16BLcutFinder(mod_n, ch_n, &tmp))
			//return -E_PIXIE_GENERAL;
	}
	else {
		sprintf(buf, "%s", "BLCUT");
		if (myPixie16WriteSglChanPar(buf, cut_val, mod_n, ch_n))
			return -E_PIXIE_GENERAL;
	}

	return 0;
}


int pixie16_ctl::do_conf_group_trig(int grp_n, uint16_t mod_n, uint16_t ch_n)
{
	/* the following pars must be present: 
	 * groupx_trigCh,
	 * groupx_trigLMR
	 * */

	int grp_LMR, grp_ch;
	
	switch (grp_n ) {
	case 0:
		grp_LMR = group0_trigLMR[mod_n][ch_n];
		grp_ch  = group0_trigCh[mod_n][ch_n];
		break;
	case 1:
		grp_LMR = group1_trigLMR[mod_n][ch_n];
		grp_ch  = group1_trigCh[mod_n][ch_n];
		break;
	case 2:
		grp_LMR = group2_trigLMR[mod_n][ch_n];
		grp_ch  = group2_trigCh[mod_n][ch_n];
		break;
	case 3:
		grp_LMR = group3_trigLMR[mod_n][ch_n];
		grp_ch  = group3_trigCh[mod_n][ch_n];
		break;
	}
	if (grp_LMR < 0) return 0;
	if (grp_ch  < 0) return 0;

	/* left, middle, right selection */
	if (set_modreg_bit(mod_n, "TrigConfig2", 16+grp_n*2, 2, grp_LMR))
		return -E_PIXIE_GENERAL;

	/* channel selection */
	if ((grp_n*12 + grp_LMR*4) <= 28) {
		if (set_modreg_bit(mod_n, "TrigConfig1", grp_n*12+grp_LMR*4, 4,
					grp_ch))
			return -E_PIXIE_GENERAL;
	}
	else {
		if (set_modreg_bit(mod_n, "TrigConfig2",
					(grp_n-2)*12+grp_LMR*4-8, 4, grp_ch))
			return -E_PIXIE_GENERAL;
	}

	return 0;
}

/* codes based on (copied from) the Pixie16WriteSglChanPar function */
int pixie16_ctl::myPixie16WriteSglChanPar(const char* ChanParName, double
		ChanParData, int ModNum, int ChanNum)
{
	unsigned int FL, FG, SL, SG, FastFilterRange, SlowFilterRange, FifoLength;
	unsigned int fastthresh, peaksample, peaksep, preamptau, tracelength, tracedelay;
	unsigned int paflength, triggerdelay, offsetdac;
	unsigned int xwait, lastxwait;
	unsigned int baselinepercent, energylow, log2ebin, newchancsra, oldchancsra, chancsrb;
	unsigned int baselinecut, fasttrigbacklen, baselineaverage;
	int retval;
	unsigned int cfddelay, cfdscale, qdclen, exttrigstretch, vetostretch, externdelaylen, multiplicitymaskl, multiplicitymaskh, ftrigoutdelay;
	unsigned int chantrigstretch, cfdthresh;

	// Write channel parameter
	
	if(strcmp(ChanParName,"TRIGGER_RISETIME") == 0)
	{
		
		// Calculate fast length
		FastFilterRange = Pixie_Devices[ModNum].DSP_Parameter_Values[FastFilterRange_Address[ModNum] - DATA_MEMORY_ADDRESS];
		if(Module_Information[ModNum].Module_ADCMSPS == 100)
			FL = (unsigned int)ROUND(ChanParData * (double)Module_Information[ModNum].Module_ADCMSPS / pow(2.0, (double)FastFilterRange));
		else if(Module_Information[ModNum].Module_ADCMSPS == 250)
			FL = (unsigned int)ROUND(ChanParData * (double)(Module_Information[ModNum].Module_ADCMSPS / 2) / pow(2.0, (double)FastFilterRange));
		else if(Module_Information[ModNum].Module_ADCMSPS == 500)
			FL = (unsigned int)ROUND(ChanParData * (double)(Module_Information[ModNum].Module_ADCMSPS / 5) / pow(2.0, (double)FastFilterRange));

		// Check fast length limit
		FG = Pixie_Devices[ModNum].DSP_Parameter_Values[FastGap_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS];
		if( (FL + FG) > FASTFILTER_MAX_LEN )
		{
			FL = FASTFILTER_MAX_LEN - FG;
		}
		if( FL < MIN_FASTLENGTH_LEN )
		{
			FL = MIN_FASTLENGTH_LEN;
			if( (FL + FG) > FASTFILTER_MAX_LEN )
			{
				FG = FASTFILTER_MAX_LEN - MIN_FASTLENGTH_LEN;
			}
		}
		
		// Update DSP parameter FastLength
		Pixie_Devices[ModNum].DSP_Parameter_Values[FastLength_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS] = FL;
		
		// Download to the selected Pixie module
		//Pixie16IMbufferIO(&FL, 1, (unsigned int)(FastLength_Address[ModNum] + ChanNum), MOD_WRITE, ModNum);
		
		// Update DSP parameter FastGap
		Pixie_Devices[ModNum].DSP_Parameter_Values[FastGap_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS] = FG;
		
		// Download to the selected Pixie module
		//Pixie16IMbufferIO(&FG, 1, (unsigned int)(FastGap_Address[ModNum] + ChanNum), MOD_WRITE, ModNum);
		
		// Program FiPPI to apply FastLength settings to the FPGA
		//retval = Pixie16ProgramFippi(ModNum);
	}
	else if(strcmp(ChanParName,"TRIGGER_FLATTOP") == 0)
	{
		
		// Calculate fast gap
		FastFilterRange = Pixie_Devices[ModNum].DSP_Parameter_Values[FastFilterRange_Address[ModNum] - DATA_MEMORY_ADDRESS];
		if(Module_Information[ModNum].Module_ADCMSPS == 100)
			FG = (unsigned int)ROUND(ChanParData * (double)Module_Information[ModNum].Module_ADCMSPS / pow(2.0, (double)FastFilterRange));
		else if(Module_Information[ModNum].Module_ADCMSPS == 250)
			FG = (unsigned int)ROUND(ChanParData * (double)(Module_Information[ModNum].Module_ADCMSPS / 2) / pow(2.0, (double)FastFilterRange));
		else if(Module_Information[ModNum].Module_ADCMSPS == 500)
			FG = (unsigned int)ROUND(ChanParData * (double)(Module_Information[ModNum].Module_ADCMSPS / 5) / pow(2.0, (double)FastFilterRange));
		
		// Check fast gap limit
		FL = Pixie_Devices[ModNum].DSP_Parameter_Values[FastLength_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS];
		if( (FL + FG) > FASTFILTER_MAX_LEN )
		{
			FG = FASTFILTER_MAX_LEN - FL;
		}
		
		// Update DSP parameter FastGap
		Pixie_Devices[ModNum].DSP_Parameter_Values[FastGap_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS] = FG;
		
		// Download to the selected Pixie module
//		Pixie16IMbufferIO(&FG, 1, (unsigned int)(FastGap_Address[ModNum] + ChanNum), MOD_WRITE, ModNum);
		
		// Program FiPPI to apply FastGap settings to the FPGA
//		retval = Pixie16ProgramFippi(ModNum);
//		if(retval < 0)
//		{
//			sprintf(ErrMSG, "*ERROR* (Pixie16WriteSglChanPar): ProgramFippi failed in module %d channel %d after downloading TriggerFlatTop, retval=%d", ModNum, ChanNum, retval);
//			Pixie_Print_MSG(ErrMSG);
//			return(-4);
//		}
	}
	else if(strcmp(ChanParName,"TRIGGER_THRESHOLD") == 0)
	{
		
		// Calculate FastThresh
		FL = Pixie_Devices[ModNum].DSP_Parameter_Values[FastLength_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS];
		if(Module_Information[ModNum].Module_ADCMSPS == 100)
			fastthresh = (unsigned int)(ChanParData * (double)FL);
		else if(Module_Information[ModNum].Module_ADCMSPS == 250)
			fastthresh = (unsigned int)(ChanParData * (double)FL * 2.0);
		else if(Module_Information[ModNum].Module_ADCMSPS == 500)
			fastthresh = (unsigned int)(ChanParData * (double)FL * 5.0);

		// Check FastThresh limit
		if( fastthresh >= FAST_THRESHOLD_MAX )
		{
			fastthresh = (unsigned int)(((double)FAST_THRESHOLD_MAX / (double)FL - 0.5) * (double)FL);	// in ADC counts
		}
		
		// Update DSP parameter FastThresh
		Pixie_Devices[ModNum].DSP_Parameter_Values[FastThresh_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS] = fastthresh;
		
		// Download to the selected Pixie module
		//Pixie16IMbufferIO(&fastthresh, 1, (unsigned int)(FastThresh_Address[ModNum] + ChanNum), MOD_WRITE, ModNum);
		
		// Program FiPPI to apply FastThresh settings to the FPGA
		//retval = Pixie16ProgramFippi(ModNum);
		//if(retval < 0)
		//{
			//sprintf(ErrMSG, "*ERROR* (Pixie16WriteSglChanPar): ProgramFippi failed in module %d channel %d after downloading TriggerThreshold, retval=%d", ModNum, ChanNum, retval);
			//Pixie_Print_MSG(ErrMSG);
			//return(-4);
		//}
	}
	else if((strcmp(ChanParName,"ENERGY_RISETIME") == 0) || (strcmp(ChanParName,"ENERGY_FLATTOP") == 0))
	{
		// Get the current TraceDelay
		FastFilterRange = Pixie_Devices[ModNum].DSP_Parameter_Values[FastFilterRange_Address[ModNum] - DATA_MEMORY_ADDRESS];
		paflength = Pixie_Devices[ModNum].DSP_Parameter_Values[PAFlength_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS];
		triggerdelay = Pixie_Devices[ModNum].DSP_Parameter_Values[TriggerDelay_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS];
		tracedelay = paflength - (unsigned int)((double)triggerdelay / pow(2.0, (double)FastFilterRange));
		
		// Get the current SlowFilterRange
		SlowFilterRange = Pixie_Devices[ModNum].DSP_Parameter_Values[SlowFilterRange_Address[ModNum] - DATA_MEMORY_ADDRESS];
		
		if(strcmp(ChanParName,"ENERGY_RISETIME") == 0)
		{
			
			// Calculate slow length
		if(Module_Information[ModNum].Module_ADCMSPS == 100)
			SL = (unsigned int)ROUND(ChanParData * (double)Module_Information[ModNum].Module_ADCMSPS / pow(2.0, (double)SlowFilterRange));
		else if(Module_Information[ModNum].Module_ADCMSPS == 250)
			SL = (unsigned int)ROUND(ChanParData * (double)(Module_Information[ModNum].Module_ADCMSPS / 2) / pow(2.0, (double)SlowFilterRange));
		else if(Module_Information[ModNum].Module_ADCMSPS == 500)
			SL = (unsigned int)ROUND(ChanParData * (double)(Module_Information[ModNum].Module_ADCMSPS / 5) / pow(2.0, (double)SlowFilterRange));

			// Check slow length limit
			SG = Pixie_Devices[ModNum].DSP_Parameter_Values[SlowGap_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS];
			if( (SL + SG) > SLOWFILTER_MAX_LEN)
			{
				SL = SLOWFILTER_MAX_LEN - SG;
			}
			if( SL < MIN_SLOWLENGTH_LEN)
			{
				SL = MIN_SLOWLENGTH_LEN;
				if((SL + SG) > SLOWFILTER_MAX_LEN)
				{
					SG = SLOWFILTER_MAX_LEN - MIN_SLOWLENGTH_LEN;
				}
			}
		}
		else if(strcmp(ChanParName,"ENERGY_FLATTOP") == 0)
		{
			
			// Calculate slow gap
		if(Module_Information[ModNum].Module_ADCMSPS == 100)
			SG = (unsigned int)ROUND(ChanParData * (double)Module_Information[ModNum].Module_ADCMSPS / pow(2.0, (double)SlowFilterRange));
		else if(Module_Information[ModNum].Module_ADCMSPS == 250)
			SG = (unsigned int)ROUND(ChanParData * (double)(Module_Information[ModNum].Module_ADCMSPS / 2) / pow(2.0, (double)SlowFilterRange));
		else if(Module_Information[ModNum].Module_ADCMSPS == 500)
			SG = (unsigned int)ROUND(ChanParData * (double)(Module_Information[ModNum].Module_ADCMSPS / 5) / pow(2.0, (double)SlowFilterRange));
			
			// Check slow gap limit
			SL = Pixie_Devices[ModNum].DSP_Parameter_Values[SlowLength_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS];
			if( (SL + SG) > SLOWFILTER_MAX_LEN)
			{
				SG = SLOWFILTER_MAX_LEN - SL;
			}
			if( SG < MIN_SLOWGAP_LEN)
			{
				SG = MIN_SLOWGAP_LEN;
				if((SL + SG) > SLOWFILTER_MAX_LEN)
				{
					SL = SLOWFILTER_MAX_LEN - MIN_SLOWGAP_LEN;
				}
			}
		}
		
		// Update DSP parameter SlowLength
		Pixie_Devices[ModNum].DSP_Parameter_Values[SlowLength_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS] = SL;
		// Download to the selected Pixie module
		//Pixie16IMbufferIO(&SL, 1, (unsigned int)(SlowLength_Address[ModNum] + ChanNum), MOD_WRITE, ModNum);
		
		// Update DSP parameter SlowGap
		Pixie_Devices[ModNum].DSP_Parameter_Values[SlowGap_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS] = SG;
		// Download to the selected Pixie module
		//Pixie16IMbufferIO(&SG, 1, (unsigned int)(SlowGap_Address[ModNum] + ChanNum), MOD_WRITE, ModNum);
		
		// Re-calculate PeakSample and PeakSep
		switch (SlowFilterRange)
		{
			case 1:
				peaksample = SL + SG - 3;
				break;
			case 2:
				peaksample = SL + SG - 2;
				break;
			case 3:
				peaksample = SL + SG - 2;
				break;
			case 4:
				peaksample = SL + SG - 1;
				break;
			case 5:
				peaksample = SL + SG;
				break;
			case 6:
				peaksample = SL + SG + 1;
				break;
			default:
				peaksample = SL + SG - 2;
				break;
		}

		peaksep = SL + SG;

		// Update DSP parameter PeakSample
		Pixie_Devices[ModNum].DSP_Parameter_Values[PeakSample_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS] = peaksample;
		// Download to the selected Pixie module
		//Pixie16IMbufferIO(&peaksample, 1, (unsigned int)(PeakSample_Address[ModNum] + ChanNum), MOD_WRITE, ModNum);
		
		// Update DSP parameter PeakSep
		Pixie_Devices[ModNum].DSP_Parameter_Values[PeakSep_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS] = peaksep;
		// Download to the selected Pixie module
		//Pixie16IMbufferIO(&peaksep, 1, (unsigned int)(PeakSep_Address[ModNum] + ChanNum), MOD_WRITE, ModNum);
		
		// Update FIFO settings (TriggerDelay and PAFLength)
		myPixie_ComputeFIFO(tracedelay, ModNum, ChanNum);
		
		// Program FiPPI to apply settings to the FPGA
		//retval = Pixie16ProgramFippi(ModNum);
		//if(retval < 0)
		//{
			//sprintf(ErrMSG, "*ERROR* (Pixie16WriteSglChanPar): ProgramFippi failed in module %d channel %d after downloading EnergyRiseTime or EnergyFlatTop, retval=%d", ModNum, ChanNum, retval);
			//Pixie_Print_MSG(ErrMSG);
			//return(-4);
		//}
		
		// Update baseline cut value
		//retval = Pixie16BLcutFinder(ModNum, ChanNum, &baselinecut);
		//if(retval < 0)
		//{
			//sprintf(ErrMSG, "*ERROR* (Pixie16WriteSglChanPar): BLcutFinder failed in module %d channel %d after downloading EnergyRiseTime or EnergyFlatTop, retval=%d", ModNum, ChanNum, retval);
			//Pixie_Print_MSG(ErrMSG);
			//return(-5);
		//}
	}
	else if(strcmp(ChanParName,"TAU") == 0)
	{
		
		// Calculate PreampTau
		preamptau = Decimal2IEEEFloating(ChanParData);
		
		// Update DSP parameter PreampTau
		Pixie_Devices[ModNum].DSP_Parameter_Values[PreampTau_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS] = preamptau;
		// Download to the selected Pixie module
		//Pixie16IMbufferIO(&preamptau, 1, (unsigned int)(PreampTau_Address[ModNum] + ChanNum), MOD_WRITE, ModNum);
		
		// Use Program_FiPPI to recompute the coefficients
		//retval = Pixie16ProgramFippi(ModNum);
		//if(retval < 0)
		//{
			//sprintf(ErrMSG, "*ERROR* (Pixie16WriteSglChanPar): ProgramFippi failed in module %d channel %d after downloading Tau, retval=%d", ModNum, ChanNum, retval);
			//Pixie_Print_MSG(ErrMSG);
			//return(-4);
		//}
		
		// Update baseline cut value
		//retval = Pixie16BLcutFinder(ModNum, ChanNum, &baselinecut);
		//if(retval < 0)
		//{
			//sprintf(ErrMSG, "*ERROR* (Pixie16WriteSglChanPar): BLcutFinder failed in module %d channel %d after downloading Tau, retval=%d", ModNum, ChanNum, retval);
			//Pixie_Print_MSG(ErrMSG);
			//return(-5);
		//}
	}
	else if(strcmp(ChanParName,"TRACE_LENGTH") == 0)
	{
		
		// Get the current FastFilterRange
		FastFilterRange = Pixie_Devices[ModNum].DSP_Parameter_Values[FastFilterRange_Address[ModNum] - DATA_MEMORY_ADDRESS];
		
		// Compute the requested TraceLength
		tracelength = (unsigned int)(ChanParData * (double)Module_Information[ModNum].Module_ADCMSPS / pow(2.0, (double)FastFilterRange));

		if(Module_Information[ModNum].Module_ADCMSPS == 500)
		{
			// Ensure TraceLength is multiple of 10 to fit 10-to-5*2 packing in the FPGA
			tracelength = ((unsigned int)(tracelength / 10)) * 10;
			if(tracelength < TRACELEN_MIN_500MHZADC)
			{
				// Enforce a minimum value
				tracelength = TRACELEN_MIN_500MHZADC;
			}
		}
		else if((Module_Information[ModNum].Module_ADCMSPS == 250) || (Module_Information[ModNum].Module_ADCMSPS == 100))
		{
			// Ensure TraceLength is an even number to fit 2-to-1 packing in the FPGA
			tracelength = APP32_ClrBit(0, tracelength);
			if(tracelength < TRACELEN_MIN_250OR100MHZADC)
			{
				// Enforce a minimum value
				tracelength = TRACELEN_MIN_250OR100MHZADC;
			}
		}

		// Check if TraceLength exceeds FifoLength
		FifoLength = Pixie_Devices[ModNum].DSP_Parameter_Values[FIFOLength_Address[ModNum] - DATA_MEMORY_ADDRESS];
		if(tracelength > FifoLength)
		{
			tracelength = FifoLength;
		}
		
		// Update DSP parameter TraceLength
		Pixie_Devices[ModNum].DSP_Parameter_Values[TraceLength_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS] = tracelength;
		// Download to the selected Pixie module
		//Pixie16IMbufferIO(&tracelength, 1, (unsigned int)(TraceLength_Address[ModNum] + ChanNum), MOD_WRITE, ModNum);

		// Program FiPPI to apply TraceLength settings to the FPGA
		//retval = Pixie16ProgramFippi(ModNum);
		//if(retval < 0)
		//{
			//sprintf(ErrMSG, "*ERROR* (Pixie16WriteSglChanPar): ProgramFippi failed in module %d channel %d after downloading TraceLength, retval=%d", ModNum, ChanNum, retval);
			//Pixie_Print_MSG(ErrMSG);
			//return(-4);
		//}
	}
	else if(strcmp(ChanParName,"TRACE_DELAY") == 0)
	{
		
		// Get the current FastFilterRange
		FastFilterRange = Pixie_Devices[ModNum].DSP_Parameter_Values[FastFilterRange_Address[ModNum] - DATA_MEMORY_ADDRESS];
		
		// Check if TraceDelay exceeds TraceLength
		if(Module_Information[ModNum].Module_ADCMSPS == 100)
			tracedelay = (unsigned int)(ChanParData * (double)Module_Information[ModNum].Module_ADCMSPS / pow(2.0, (double)FastFilterRange));
		else if(Module_Information[ModNum].Module_ADCMSPS == 250)
			tracedelay = (unsigned int)(ChanParData * (double)(Module_Information[ModNum].Module_ADCMSPS / 2) / pow(2.0, (double)FastFilterRange));
		else if(Module_Information[ModNum].Module_ADCMSPS == 500)
			tracedelay = (unsigned int)(ChanParData * (double)(Module_Information[ModNum].Module_ADCMSPS / 5) / pow(2.0, (double)FastFilterRange));

		tracelength = Pixie_Devices[ModNum].DSP_Parameter_Values[TraceLength_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS];
		if(tracedelay > tracelength)
		{
			tracedelay = (unsigned int)((double)tracelength / 2.0);
		}
		
		// Check if TraceDelay exceeds TRACEDELAY_MAX
		if(tracedelay > TRACEDELAY_MAX)
		{
			tracedelay = TRACEDELAY_MAX;
		}

		// Update FIFO settings (TriggerDelay and PAFLength)
		myPixie_ComputeFIFO(tracedelay, ModNum, ChanNum);
		
		// Program FiPPI to apply settings to the FPGA
		//retval = Pixie16ProgramFippi(ModNum);
		//if(retval < 0)
		//{
			//sprintf(ErrMSG, "*ERROR* (Pixie16WriteSglChanPar): ProgramFippi failed in module %d channel %d after downloading TraceDelay, retval=%d", ModNum, ChanNum, retval);
			//Pixie_Print_MSG(ErrMSG);
			//return(-4);
		//}
	}
	else if(strcmp(ChanParName,"VOFFSET") == 0)
	{
		
		offsetdac = (unsigned int)(65536.0 * (ChanParData / DAC_VOLTAGE_RANGE  + 1.0 / 2.0));
		
		// Check limit
		if(offsetdac > 65535)
		{
			offsetdac = 65535;	// a 16-bit DAC
		}
		
		// Update DSP parameter OffsetDAC
		Pixie_Devices[ModNum].DSP_Parameter_Values[OffsetDAC_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS] = offsetdac;
		// Download to the selected Pixie module
		//Pixie16IMbufferIO(&offsetdac, 1, (unsigned int)(OffsetDAC_Address[ModNum] + ChanNum), MOD_WRITE, ModNum);
		
		// Set DACs to apply the new DAC settings
		//retval = Pixie16SetDACs(ModNum);
		//if(retval < 0)
		//{
			//sprintf(ErrMSG, "*ERROR* (Pixie16WriteSglChanPar): SetDACs failed in module %d channel %d after downloading OffsetDACs, retval=%d", ModNum, ChanNum, retval);
			//Pixie_Print_MSG(ErrMSG);
			//return(-6);
		//}
	}
	else if(strcmp(ChanParName,"XDT") == 0)
	{
		// Get the last Xwait
		lastxwait = Pixie_Devices[ModNum].DSP_Parameter_Values[Xwait_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS];
		// Compute the new Xwait
		xwait = (unsigned int)ROUND(ChanParData * (double)DSP_CLOCK_MHZ);
		if((Module_Information[ModNum].Module_ADCMSPS == 100) || (Module_Information[ModNum].Module_ADCMSPS == 500))
		{
			// For 100 MSPS RevB/C/D or RevF, or 500 MSPS RevF, xwait should be multiples of 6

			if(xwait < 6)	// xwait should be at least 6
			{
				xwait = 6;
			}
			
			if(xwait > lastxwait) // increase Xwait
			{
				xwait = (unsigned int)(ceil((double)xwait / 6.0) * 6.0);
			}
			else	// decrease Xwait
			{
				xwait = (unsigned int)(floor((double)xwait / 6.0) * 6.0);
			}
		}
		else
		{
			// For 250 RevF, xwait should be multiples of 8

			if(xwait < 8)	// xwait should be at least 8
			{
				xwait = 8;
			}
			
			if(xwait > lastxwait) // increase Xwait
			{
				xwait = (unsigned int)(ceil((double)xwait / 8.0) * 8.0);
			}
			else	// decrease Xwait
			{
				xwait = (unsigned int)(floor((double)xwait / 8.0) * 8.0);
			}
		}

		// Update DSP parameter Xwait
		Pixie_Devices[ModNum].DSP_Parameter_Values[Xwait_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS] = xwait;
		// Download to the selected Pixie module
		//Pixie16IMbufferIO(&xwait, 1, (unsigned int)(Xwait_Address[ModNum] + ChanNum), MOD_WRITE, ModNum);
		
	}
	else if(strcmp(ChanParName,"BASELINE_PERCENT") == 0)
	{
		
		// Get the new BaselinePercent
		baselinepercent = (unsigned int)ChanParData;
		
		// Check limit
		if(baselinepercent < 1)
		{
			baselinepercent = 1;
		}
		else if(baselinepercent > 99)
		{
			baselinepercent = 99;
		}
		
		// Update DSP parameter BaselinePercent
		Pixie_Devices[ModNum].DSP_Parameter_Values[BaselinePercent_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS] = baselinepercent;
		// Download to the selected Pixie module
		//Pixie16IMbufferIO(&baselinepercent, 1, (unsigned int)(BaselinePercent_Address[ModNum] + ChanNum), MOD_WRITE, ModNum);
		
	}
	else if(strcmp(ChanParName,"EMIN") == 0)
	{
		
		// Get the new EnergyLow
		energylow = (unsigned int)ChanParData;
		
		// Update DSP parameter EnergyLow
		Pixie_Devices[ModNum].DSP_Parameter_Values[EnergyLow_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS] = energylow;
		// Download to the selected Pixie module
		//Pixie16IMbufferIO(&energylow, 1, (unsigned int)(EnergyLow_Address[ModNum] + ChanNum), MOD_WRITE, ModNum);
		
	}
	else if(strcmp(ChanParName,"BINFACTOR") == 0)
	{
		
		// Get the new Log2Ebin
		log2ebin = (unsigned int)ChanParData;
		
		// Check limit
		if(log2ebin < 1)
		{
			log2ebin = 1;
		}
		if(log2ebin > 6)
		{
			log2ebin = 6;
		}
		
		// Convert to a signed negative number
		log2ebin = (unsigned int)(pow(2.0, 32.0) - (double)log2ebin);
		
		// Update DSP parameter Log2Ebin
		Pixie_Devices[ModNum].DSP_Parameter_Values[Log2Ebin_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS] = log2ebin;
		// Download to the selected Pixie module
		//Pixie16IMbufferIO(&log2ebin, 1, (unsigned int)(Log2Ebin_Address[ModNum] + ChanNum), MOD_WRITE, ModNum);
		
	}
	else if(strcmp(ChanParName,"BASELINE_AVERAGE") == 0)
	{
		
		// Get the new baselineaverage
		baselineaverage = (unsigned int)ChanParData;

		// Check limit
		if(baselineaverage > 16)
		{
			baselineaverage = 16;
		}
		
		// Convert to a signed negative number
		if(baselineaverage > 0)
		{
			baselineaverage = (unsigned int)(pow(2.0, 32.0) - (double)baselineaverage);
		}
		
		// Update DSP parameter Log2Bweight
		Pixie_Devices[ModNum].DSP_Parameter_Values[Log2Bweight_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS] = baselineaverage;
		// Download to the selected Pixie module
		//Pixie16IMbufferIO(&baselineaverage, 1, (unsigned int)(Log2Bweight_Address[ModNum] + ChanNum), MOD_WRITE, ModNum);
	
	}
	else if(strcmp(ChanParName,"CHANNEL_CSRA") == 0)
	{
		
		// Get the new ChanCSRa
		newchancsra = (unsigned int)ChanParData;
		oldchancsra = Pixie_Devices[ModNum].DSP_Parameter_Values[ChanCSRa_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS];
		
		// Update DSP parameter ChanCSRa
		Pixie_Devices[ModNum].DSP_Parameter_Values[ChanCSRa_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS] = newchancsra;
		// Download to the selected Pixie module
		//Pixie16IMbufferIO(&newchancsra, 1, (unsigned int)(ChanCSRa_Address[ModNum] + ChanNum), MOD_WRITE, ModNum);
		
		// Program FiPPI to apply settings to the FPGA
		//retval = Pixie16ProgramFippi(ModNum);
		//if(retval < 0)
		//{
			//sprintf(ErrMSG, "*ERROR* (Pixie16WriteSglChanPar): ProgramFippi failed in module %d channel %d after downloading ChanCSRA, retval=%d", ModNum, ChanNum, retval);
			//Pixie_Print_MSG(ErrMSG);
			//return(-4);
		//}
		
		// Set DACs to apply the new DAC settings
		//retval = Pixie16SetDACs(ModNum);
		//if(retval < 0)
		//{
			//sprintf(ErrMSG, "*ERROR* (Pixie16WriteSglChanPar): SetDACs failed in module %d channel %d after downloading ChanCSRA, retval=%d", ModNum, ChanNum, retval);
			//Pixie_Print_MSG(ErrMSG);
			//return(-6);
		//}
	
		// Check if we need to update baseline cut value (only needed if Vgain changed)
		//if(APP32_TstBit(CCSRA_ENARELAY, newchancsra) != APP32_TstBit(CCSRA_ENARELAY, oldchancsra))
		//{
			//retval = Pixie16BLcutFinder(ModNum, ChanNum, &baselinecut);
			//if(retval < 0)
			//{
				//sprintf(ErrMSG, "*ERROR* (Pixie16WriteSglChanPar): BLcutFinder failed in module %d channel %d after downloading ChanCSRA, retval=%d", ModNum, ChanNum, retval);
				//Pixie_Print_MSG(ErrMSG);
				//return(-5);
			//}
		//}
	}
	else if(strcmp(ChanParName,"CHANNEL_CSRB") == 0)
	{
		
		// Get the new ChanCSRb
		chancsrb = (unsigned int)ChanParData;
		
		// Update DSP parameter ChanCSRb
		Pixie_Devices[ModNum].DSP_Parameter_Values[ChanCSRb_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS] = chancsrb;
		// Download to the selected Pixie module
		//Pixie16IMbufferIO(&chancsrb, 1, (unsigned int)(ChanCSRb_Address[ModNum] + ChanNum), MOD_WRITE, ModNum);
		
	}
	else if(strcmp(ChanParName,"BLCUT") == 0)
	{
		
		// Get the new BLcut
		baselinecut = (unsigned int)ChanParData;
		
		// Update DSP parameter BLcut
		Pixie_Devices[ModNum].DSP_Parameter_Values[BLcut_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS] = baselinecut;
		// Download to the selected Pixie module
		//Pixie16IMbufferIO(&baselinecut, 1, (unsigned int)(BLcut_Address[ModNum] + ChanNum), MOD_WRITE, ModNum);
		
	}
	else if(strcmp(ChanParName,"FASTTRIGBACKLEN") == 0)
	{
		
		// Get the new FastTrigBackLen
		if(Module_Information[ModNum].Module_ADCMSPS == 100)
			fasttrigbacklen = (unsigned int)ROUND(ChanParData * (double)Module_Information[ModNum].Module_ADCMSPS);
		else if(Module_Information[ModNum].Module_ADCMSPS == 250)
			fasttrigbacklen = (unsigned int)ROUND(ChanParData * (double)(Module_Information[ModNum].Module_ADCMSPS / 2));
		else if(Module_Information[ModNum].Module_ADCMSPS == 500)
			fasttrigbacklen = (unsigned int)ROUND(ChanParData * (double)(Module_Information[ModNum].Module_ADCMSPS / 5));

		// Range check for FastTrigBackLen
		if((Module_Information[ModNum].Module_ADCMSPS == 100) || (Module_Information[ModNum].Module_ADCMSPS == 500))
		{
			if(fasttrigbacklen < FASTTRIGBACKLEN_MIN_100MHZFIPCLK)
			{
				fasttrigbacklen = FASTTRIGBACKLEN_MIN_100MHZFIPCLK;
			}
		}
		else if(Module_Information[ModNum].Module_ADCMSPS == 250)
		{
			if(fasttrigbacklen < FASTTRIGBACKLEN_MIN_125MHZFIPCLK)
			{
				fasttrigbacklen = FASTTRIGBACKLEN_MIN_125MHZFIPCLK;
			}
		}

		if(fasttrigbacklen > FASTTRIGBACKLEN_MAX)
		{
			fasttrigbacklen = FASTTRIGBACKLEN_MAX;
		}
		
		// Update DSP parameter FastTrigBackLen
		Pixie_Devices[ModNum].DSP_Parameter_Values[FastTrigBackLen_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS] = fasttrigbacklen;
		// Download to the selected Pixie module
		//Pixie16IMbufferIO(&fasttrigbacklen, 1, (unsigned int)(FastTrigBackLen_Address[ModNum] + ChanNum), MOD_WRITE, ModNum);
		
		// Program FiPPI to apply FastTrigBackLen settings to the FPGA
		//retval = Pixie16ProgramFippi(ModNum);
		//if(retval < 0)
		//{
			//sprintf(ErrMSG, "*ERROR* (Pixie16WriteSglChanPar): ProgramFippi failed in module %d channel %d after downloading FastTrigBackLen, retval=%d", ModNum, ChanNum, retval);
			//Pixie_Print_MSG(ErrMSG);
			//return(-4);
		//}
	}
	else if(strcmp(ChanParName,"CFDDelay") == 0)
	{
		// Get the new CFDDelay
		if(Module_Information[ModNum].Module_ADCMSPS == 100)
			cfddelay = (unsigned int)ROUND(ChanParData * (double)Module_Information[ModNum].Module_ADCMSPS);
		else if(Module_Information[ModNum].Module_ADCMSPS == 250)
			cfddelay = (unsigned int)ROUND(ChanParData * (double)(Module_Information[ModNum].Module_ADCMSPS / 2));
		else if(Module_Information[ModNum].Module_ADCMSPS == 500)
			cfddelay = (unsigned int)ROUND(ChanParData * (double)(Module_Information[ModNum].Module_ADCMSPS / 5));

		// Range check for CFDDelay
		if(cfddelay < CFDDELAY_MIN)
		{
			cfddelay = CFDDELAY_MIN;
		}
		if(cfddelay > CFDDELAY_MAX)
		{
			cfddelay = CFDDELAY_MAX;
		}

		// Update DSP parameter CFDDelay
		Pixie_Devices[ModNum].DSP_Parameter_Values[CFDDelay_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS] = cfddelay;
		// Download to the selected Pixie module
		//Pixie16IMbufferIO(&cfddelay, 1, (unsigned int)(CFDDelay_Address[ModNum] + ChanNum), MOD_WRITE, ModNum);

		// Program FiPPI to apply CFDDelay settings to the FPGA
		//retval = Pixie16ProgramFippi(ModNum);
		//if(retval < 0)
		//{
			//sprintf(ErrMSG, "*ERROR* (Pixie16WriteSglChanPar): ProgramFippi failed in module %d channel %d after downloading CFDDelay, retval=%d", ModNum, ChanNum, retval);
			//Pixie_Print_MSG(ErrMSG);
			//return(-4);
		//}
	}
	else if(strcmp(ChanParName,"CFDScale") == 0)
	{
		// Get the new CFDScale
		cfdscale = (unsigned int)ChanParData;

		// Range check for the value of CFDScale
		if(cfdscale > CFDSCALE_MAX)
		{
			cfdscale = CFDSCALE_MAX;
		}

		// Update DSP parameter CFDScale
		Pixie_Devices[ModNum].DSP_Parameter_Values[CFDScale_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS] = cfdscale;
		// Download to the selected Pixie module
		//Pixie16IMbufferIO(&cfdscale, 1, (unsigned int)(CFDScale_Address[ModNum] + ChanNum), MOD_WRITE, ModNum);

		// Program FiPPI to apply CFDScale settings to the FPGA
		//retval = Pixie16ProgramFippi(ModNum);
		//if(retval < 0)
		//{
			//sprintf(ErrMSG, "*ERROR* (Pixie16WriteSglChanPar): ProgramFippi failed in module %d channel %d after downloading CFDScale, retval=%d", ModNum, ChanNum, retval);
			//Pixie_Print_MSG(ErrMSG);
			//return(-4);
		//}
	}
	else if(strcmp(ChanParName,"CFDThresh") == 0)
	{
		// Get the new CFDThresh
		cfdthresh = (unsigned int)ChanParData;

		// Range check for the value of CFDThresh
		if( (cfdthresh >= CFDTHRESH_MIN) && (cfdthresh <= CFDTHRESH_MAX) )
		{
			// Update DSP parameter CFDThresh
			Pixie_Devices[ModNum].DSP_Parameter_Values[CFDThresh_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS] = cfdthresh;
			// Download to the selected Pixie module
			//Pixie16IMbufferIO(&cfdthresh, 1, (unsigned int)(CFDThresh_Address[ModNum] + ChanNum), MOD_WRITE, ModNum);

			// Program FiPPI to apply CFDThresh settings to the FPGA
			//retval = Pixie16ProgramFippi(ModNum);
			//if(retval < 0)
			//{
				//sprintf(ErrMSG, "*ERROR* (Pixie16WriteSglChanPar): ProgramFippi failed in module %d channel %d after downloading CFDThresh, retval=%d", ModNum, ChanNum, retval);
				//Pixie_Print_MSG(ErrMSG);
				//return(-4);
			//}
		}
		else
		{
			//sprintf(ErrMSG, "*ERROR* (Pixie16WriteSglChanPar): Value of CFDThresh being set was out of its valid range");
			//Pixie_Print_MSG(ErrMSG);
		}
	}
	else if(strcmp(ChanParName,"QDCLen0") == 0)
	{
		// Get the new QDCLen0
		if(Module_Information[ModNum].Module_ADCMSPS == 500)
			qdclen = (unsigned int)ROUND(ChanParData * (double)(Module_Information[ModNum].Module_ADCMSPS / 5));
		else
			qdclen = (unsigned int)ROUND(ChanParData * (double)Module_Information[ModNum].Module_ADCMSPS);

		// Range check for QDC length
		if(qdclen < QDCLEN_MIN)
		{
			qdclen = QDCLEN_MIN;
		}
		if(qdclen > QDCLEN_MAX)
		{
			qdclen = QDCLEN_MAX;
		}

		// Update DSP parameter QDCLen0
		Pixie_Devices[ModNum].DSP_Parameter_Values[QDCLen0_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS] = qdclen;
		// Download to the selected Pixie module
		//Pixie16IMbufferIO(&qdclen, 1, (unsigned int)(QDCLen0_Address[ModNum] + ChanNum), MOD_WRITE, ModNum);

		// Program FiPPI to apply QDCLen0 settings to the FPGA
		//retval = Pixie16ProgramFippi(ModNum);
		//if(retval < 0)
		//{
			//sprintf(ErrMSG, "*ERROR* (Pixie16WriteSglChanPar): ProgramFippi failed in module %d channel %d after downloading QDCLen0, retval=%d", ModNum, ChanNum, retval);
			//Pixie_Print_MSG(ErrMSG);
			//return(-4);
		//}
	}
	else if(strcmp(ChanParName,"QDCLen1") == 0)
	{
		// Get the new QDCLen1
		if(Module_Information[ModNum].Module_ADCMSPS == 500)
			qdclen = (unsigned int)ROUND(ChanParData * (double)(Module_Information[ModNum].Module_ADCMSPS / 5));
		else
			qdclen = (unsigned int)ROUND(ChanParData * (double)Module_Information[ModNum].Module_ADCMSPS);

		// Range check for QDC length
		if(qdclen < QDCLEN_MIN)
		{
			qdclen = QDCLEN_MIN;
		}
		if(qdclen > QDCLEN_MAX)
		{
			qdclen = QDCLEN_MAX;
		}

		// Update DSP parameter QDCLen1
		Pixie_Devices[ModNum].DSP_Parameter_Values[QDCLen1_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS] = qdclen;
		// Download to the selected Pixie module
		//Pixie16IMbufferIO(&qdclen, 1, (unsigned int)(QDCLen1_Address[ModNum] + ChanNum), MOD_WRITE, ModNum);

		// Program FiPPI to apply QDCLen1 settings to the FPGA
		//retval = Pixie16ProgramFippi(ModNum);
		//if(retval < 0)
		//{
			//sprintf(ErrMSG, "*ERROR* (Pixie16WriteSglChanPar): ProgramFippi failed in module %d channel %d after downloading QDCLen1, retval=%d", ModNum, ChanNum, retval);
			//Pixie_Print_MSG(ErrMSG);
			//return(-4);
		//}
	}
	else if(strcmp(ChanParName,"QDCLen2") == 0)
	{
		// Get the new QDCLen2
		if(Module_Information[ModNum].Module_ADCMSPS == 500)
			qdclen = (unsigned int)ROUND(ChanParData * (double)(Module_Information[ModNum].Module_ADCMSPS / 5));
		else
			qdclen = (unsigned int)ROUND(ChanParData * (double)Module_Information[ModNum].Module_ADCMSPS);

		// Range check for QDC length
		if(qdclen < QDCLEN_MIN)
		{
			qdclen = QDCLEN_MIN;
		}
		if(qdclen > QDCLEN_MAX)
		{
			qdclen = QDCLEN_MAX;
		}

		// Update DSP parameter QDCLen2
		Pixie_Devices[ModNum].DSP_Parameter_Values[QDCLen2_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS] = qdclen;
		// Download to the selected Pixie module
		//Pixie16IMbufferIO(&qdclen, 1, (unsigned int)(QDCLen2_Address[ModNum] + ChanNum), MOD_WRITE, ModNum);

		// Program FiPPI to apply QDCLen2 settings to the FPGA
		//retval = Pixie16ProgramFippi(ModNum);
		//if(retval < 0)
		//{
			//sprintf(ErrMSG, "*ERROR* (Pixie16WriteSglChanPar): ProgramFippi failed in module %d channel %d after downloading QDCLen2, retval=%d", ModNum, ChanNum, retval);
			//Pixie_Print_MSG(ErrMSG);
			//return(-4);
		//}
	}
	else if(strcmp(ChanParName,"QDCLen3") == 0)
	{
		// Get the new QDCLen3
		if(Module_Information[ModNum].Module_ADCMSPS == 500)
			qdclen = (unsigned int)ROUND(ChanParData * (double)(Module_Information[ModNum].Module_ADCMSPS / 5));
		else
			qdclen = (unsigned int)ROUND(ChanParData * (double)Module_Information[ModNum].Module_ADCMSPS);

		// Range check for QDC length
		if(qdclen < QDCLEN_MIN)
		{
			qdclen = QDCLEN_MIN;
		}
		if(qdclen > QDCLEN_MAX)
		{
			qdclen = QDCLEN_MAX;
		}

		// Update DSP parameter QDCLen3
		Pixie_Devices[ModNum].DSP_Parameter_Values[QDCLen3_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS] = qdclen;
		// Download to the selected Pixie module
		//Pixie16IMbufferIO(&qdclen, 1, (unsigned int)(QDCLen3_Address[ModNum] + ChanNum), MOD_WRITE, ModNum);

		// Program FiPPI to apply QDCLen3 settings to the FPGA
		//retval = Pixie16ProgramFippi(ModNum);
		//if(retval < 0)
		//{
			//sprintf(ErrMSG, "*ERROR* (Pixie16WriteSglChanPar): ProgramFippi failed in module %d channel %d after downloading QDCLen3, retval=%d", ModNum, ChanNum, retval);
			//Pixie_Print_MSG(ErrMSG);
			//return(-4);
		//}
	}
	else if(strcmp(ChanParName,"QDCLen4") == 0)
	{
		// Get the new QDCLen4
		if(Module_Information[ModNum].Module_ADCMSPS == 500)
			qdclen = (unsigned int)ROUND(ChanParData * (double)(Module_Information[ModNum].Module_ADCMSPS / 5));
		else
			qdclen = (unsigned int)ROUND(ChanParData * (double)Module_Information[ModNum].Module_ADCMSPS);

		// Range check for QDC length
		if(qdclen < QDCLEN_MIN)
		{
			qdclen = QDCLEN_MIN;
		}
		if(qdclen > QDCLEN_MAX)
		{
			qdclen = QDCLEN_MAX;
		}

		// Update DSP parameter QDCLen4
		Pixie_Devices[ModNum].DSP_Parameter_Values[QDCLen4_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS] = qdclen;
		// Download to the selected Pixie module
		//Pixie16IMbufferIO(&qdclen, 1, (unsigned int)(QDCLen4_Address[ModNum] + ChanNum), MOD_WRITE, ModNum);

		// Program FiPPI to apply QDCLen4 settings to the FPGA
		//retval = Pixie16ProgramFippi(ModNum);
		//if(retval < 0)
		//{
			//sprintf(ErrMSG, "*ERROR* (Pixie16WriteSglChanPar): ProgramFippi failed in module %d channel %d after downloading QDCLen4, retval=%d", ModNum, ChanNum, retval);
			//Pixie_Print_MSG(ErrMSG);
			//return(-4);
		//}
	}
	else if(strcmp(ChanParName,"QDCLen5") == 0)
	{
		// Get the new QDCLen5
		if(Module_Information[ModNum].Module_ADCMSPS == 500)
			qdclen = (unsigned int)ROUND(ChanParData * (double)(Module_Information[ModNum].Module_ADCMSPS / 5));
		else
			qdclen = (unsigned int)ROUND(ChanParData * (double)Module_Information[ModNum].Module_ADCMSPS);

		// Range check for QDC length
		if(qdclen < QDCLEN_MIN)
		{
			qdclen = QDCLEN_MIN;
		}
		if(qdclen > QDCLEN_MAX)
		{
			qdclen = QDCLEN_MAX;
		}

		// Update DSP parameter QDCLen5
		Pixie_Devices[ModNum].DSP_Parameter_Values[QDCLen5_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS] = qdclen;
		// Download to the selected Pixie module
		//Pixie16IMbufferIO(&qdclen, 1, (unsigned int)(QDCLen5_Address[ModNum] + ChanNum), MOD_WRITE, ModNum);

		// Program FiPPI to apply QDCLen5 settings to the FPGA
		//retval = Pixie16ProgramFippi(ModNum);
		//if(retval < 0)
		//{
			//sprintf(ErrMSG, "*ERROR* (Pixie16WriteSglChanPar): ProgramFippi failed in module %d channel %d after downloading QDCLen5, retval=%d", ModNum, ChanNum, retval);
			//Pixie_Print_MSG(ErrMSG);
			//return(-4);
		//}
	}
	else if(strcmp(ChanParName,"QDCLen6") == 0)
	{
		// Get the new QDCLen6
		if(Module_Information[ModNum].Module_ADCMSPS == 500)
			qdclen = (unsigned int)ROUND(ChanParData * (double)(Module_Information[ModNum].Module_ADCMSPS / 5));
		else
			qdclen = (unsigned int)ROUND(ChanParData * (double)Module_Information[ModNum].Module_ADCMSPS);

		// Range check for QDC length
		if(qdclen < QDCLEN_MIN)
		{
			qdclen = QDCLEN_MIN;
		}
		if(qdclen > QDCLEN_MAX)
		{
			qdclen = QDCLEN_MAX;
		}

		// Update DSP parameter QDCLen6
		Pixie_Devices[ModNum].DSP_Parameter_Values[QDCLen6_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS] = qdclen;
		// Download to the selected Pixie module
		//Pixie16IMbufferIO(&qdclen, 1, (unsigned int)(QDCLen6_Address[ModNum] + ChanNum), MOD_WRITE, ModNum);

		// Program FiPPI to apply QDCLen6 settings to the FPGA
		//retval = Pixie16ProgramFippi(ModNum);
		//if(retval < 0)
		//{
			//sprintf(ErrMSG, "*ERROR* (Pixie16WriteSglChanPar): ProgramFippi failed in module %d channel %d after downloading QDCLen6, retval=%d", ModNum, ChanNum, retval);
			//Pixie_Print_MSG(ErrMSG);
			//return(-4);
		//}
	}
	else if(strcmp(ChanParName,"QDCLen7") == 0)
	{
		// Get the new QDCLen7
		if(Module_Information[ModNum].Module_ADCMSPS == 500)
			qdclen = (unsigned int)ROUND(ChanParData * (double)(Module_Information[ModNum].Module_ADCMSPS / 5));
		else
			qdclen = (unsigned int)ROUND(ChanParData * (double)Module_Information[ModNum].Module_ADCMSPS);

		// Range check for QDC length
		if(qdclen < QDCLEN_MIN)
		{
			qdclen = QDCLEN_MIN;
		}
		if(qdclen > QDCLEN_MAX)
		{
			qdclen = QDCLEN_MAX;
		}

		// Update DSP parameter QDCLen7
		Pixie_Devices[ModNum].DSP_Parameter_Values[QDCLen7_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS] = qdclen;
		// Download to the selected Pixie module
		//Pixie16IMbufferIO(&qdclen, 1, (unsigned int)(QDCLen7_Address[ModNum] + ChanNum), MOD_WRITE, ModNum);

		// Program FiPPI to apply QDCLen7 settings to the FPGA
		//retval = Pixie16ProgramFippi(ModNum);
		//if(retval < 0)
		//{
			//sprintf(ErrMSG, "*ERROR* (Pixie16WriteSglChanPar): ProgramFippi failed in module %d channel %d after downloading QDCLen7, retval=%d", ModNum, ChanNum, retval);
			//Pixie_Print_MSG(ErrMSG);
			//return(-4);
		//}
	}
	else if(strcmp(ChanParName,"ExtTrigStretch") == 0)
	{
		// Get the new ExtTrigStretch
		if(Module_Information[ModNum].Module_ADCMSPS == 100)
			exttrigstretch = (unsigned int)ROUND(ChanParData * (double)Module_Information[ModNum].Module_ADCMSPS);
		else if(Module_Information[ModNum].Module_ADCMSPS == 250)
			exttrigstretch = (unsigned int)ROUND(ChanParData * (double)(Module_Information[ModNum].Module_ADCMSPS / 2));
		else if(Module_Information[ModNum].Module_ADCMSPS == 500)
			exttrigstretch = (unsigned int)ROUND(ChanParData * (double)(Module_Information[ModNum].Module_ADCMSPS / 5));

		// Range check for ExtTrigStretch
		if(exttrigstretch < EXTTRIGSTRETCH_MIN)
		{
			exttrigstretch = EXTTRIGSTRETCH_MIN;
		}
		if(exttrigstretch > EXTTRIGSTRETCH_MAX)
		{
			exttrigstretch = EXTTRIGSTRETCH_MAX;
		}

		// Update DSP parameter ExtTrigStretch
		Pixie_Devices[ModNum].DSP_Parameter_Values[ExtTrigStretch_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS] = exttrigstretch;
		// Download to the selected Pixie module
		//Pixie16IMbufferIO(&exttrigstretch, 1, (unsigned int)(ExtTrigStretch_Address[ModNum] + ChanNum), MOD_WRITE, ModNum);

		// Program FiPPI to apply ExtTrigStretch settings to the FPGA
		//retval = Pixie16ProgramFippi(ModNum);
		//if(retval < 0)
		//{
			//sprintf(ErrMSG, "*ERROR* (Pixie16WriteSglChanPar): ProgramFippi failed in module %d channel %d after downloading ExtTrigStretch, retval=%d", ModNum, ChanNum, retval);
			//Pixie_Print_MSG(ErrMSG);
			//return(-4);
		//}
	}
	else if(strcmp(ChanParName,"VetoStretch") == 0)
	{
		// Get the new VetoStretch
		if(Module_Information[ModNum].Module_ADCMSPS == 100)
			vetostretch = (unsigned int)ROUND(ChanParData * (double)Module_Information[ModNum].Module_ADCMSPS);
		else if(Module_Information[ModNum].Module_ADCMSPS == 250)
			vetostretch = (unsigned int)ROUND(ChanParData * (double)(Module_Information[ModNum].Module_ADCMSPS / 2));
		else if(Module_Information[ModNum].Module_ADCMSPS == 500)
			vetostretch = (unsigned int)ROUND(ChanParData * (double)(Module_Information[ModNum].Module_ADCMSPS / 5));

		// Range check for VetoStretch
		if(vetostretch < VETOSTRETCH_MIN)
		{
			vetostretch = VETOSTRETCH_MIN;
		}
		if(vetostretch > VETOSTRETCH_MAX)
		{
			vetostretch = VETOSTRETCH_MAX;
		}

		// Update DSP parameter VetoStretch
		Pixie_Devices[ModNum].DSP_Parameter_Values[VetoStretch_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS] = vetostretch;
		// Download to the selected Pixie module
		//Pixie16IMbufferIO(&vetostretch, 1, (unsigned int)(VetoStretch_Address[ModNum] + ChanNum), MOD_WRITE, ModNum);

		// Program FiPPI to apply VetoStretch settings to the FPGA
		//retval = Pixie16ProgramFippi(ModNum);
		//if(retval < 0)
		//{
			//sprintf(ErrMSG, "*ERROR* (Pixie16WriteSglChanPar): ProgramFippi failed in module %d channel %d after downloading VetoStretch, retval=%d", ModNum, ChanNum, retval);
			//Pixie_Print_MSG(ErrMSG);
			//return(-4);
		//}
	}
	else if(strcmp(ChanParName,"MultiplicityMaskL") == 0)
	{
		// Get the new MultiplicityMaskL
		multiplicitymaskl = (unsigned int)ChanParData;

		// Update DSP parameter MultiplicityMaskL
		Pixie_Devices[ModNum].DSP_Parameter_Values[MultiplicityMaskL_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS] = multiplicitymaskl;
		// Download to the selected Pixie module
		//Pixie16IMbufferIO(&multiplicitymaskl, 1, (unsigned int)(MultiplicityMaskL_Address[ModNum] + ChanNum), MOD_WRITE, ModNum);

		// Program FiPPI to apply MultiplicityMaskL settings to the FPGA
		//retval = Pixie16ProgramFippi(ModNum);
		//if(retval < 0)
		//{
			//sprintf(ErrMSG, "*ERROR* (Pixie16WriteSglChanPar): ProgramFippi failed in module %d channel %d after downloading MultiplicityMaskL, retval=%d", ModNum, ChanNum, retval);
			//Pixie_Print_MSG(ErrMSG);
			//return(-4);
		//}
	}
	else if(strcmp(ChanParName,"MultiplicityMaskH") == 0)
	{
		// Get the new MultiplicityMaskH
		multiplicitymaskh = (unsigned int)ChanParData;

		// Update DSP parameter MultiplicityMaskH
		Pixie_Devices[ModNum].DSP_Parameter_Values[MultiplicityMaskH_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS] = multiplicitymaskh;
		// Download to the selected Pixie module
		//Pixie16IMbufferIO(&multiplicitymaskh, 1, (unsigned int)(MultiplicityMaskH_Address[ModNum] + ChanNum), MOD_WRITE, ModNum);

		// Program FiPPI to apply MultiplicityMaskH settings to the FPGA
		//retval = Pixie16ProgramFippi(ModNum);
		//if(retval < 0)
		//{
			//sprintf(ErrMSG, "*ERROR* (Pixie16WriteSglChanPar): ProgramFippi failed in module %d channel %d after downloading MultiplicityMaskH, retval=%d", ModNum, ChanNum, retval);
			//Pixie_Print_MSG(ErrMSG);
			//return(-4);
		//}
	}
	else if(strcmp(ChanParName,"ExternDelayLen") == 0)
	{
		// Get the new ExternDelayLen
		if(Module_Information[ModNum].Module_ADCMSPS == 100)
			externdelaylen = (unsigned int)ROUND(ChanParData * (double)Module_Information[ModNum].Module_ADCMSPS);
		else if(Module_Information[ModNum].Module_ADCMSPS == 250)
			externdelaylen = (unsigned int)ROUND(ChanParData * (double)(Module_Information[ModNum].Module_ADCMSPS / 2));
		else if(Module_Information[ModNum].Module_ADCMSPS == 500)
			externdelaylen = (unsigned int)ROUND(ChanParData * (double)(Module_Information[ModNum].Module_ADCMSPS / 5));

		// Range check for ExternDelayLen
		if(externdelaylen < EXTDELAYLEN_MIN)
		{
			externdelaylen = EXTDELAYLEN_MIN;
		}

		if ((Module_Information[ModNum].Module_Rev == 0xB) || (Module_Information[ModNum].Module_Rev == 0xC) || (Module_Information[ModNum].Module_Rev == 0xD))
		{
			if(externdelaylen > EXTDELAYLEN_MAX_REVBCD)
			{
				externdelaylen = EXTDELAYLEN_MAX_REVBCD;
			}

		}
		else if (Module_Information[ModNum].Module_Rev == 0xF)
		{
			if(externdelaylen > EXTDELAYLEN_MAX_REVF)
			{
				externdelaylen = EXTDELAYLEN_MAX_REVF;
			}
		}

		// Update DSP parameter ExternDelayLen
		Pixie_Devices[ModNum].DSP_Parameter_Values[ExternDelayLen_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS] = externdelaylen;
		// Download to the selected Pixie module
		//Pixie16IMbufferIO(&externdelaylen, 1, (unsigned int)(ExternDelayLen_Address[ModNum] + ChanNum), MOD_WRITE, ModNum);

		// Program FiPPI to apply ExternDelayLen settings to the FPGA
		//retval = Pixie16ProgramFippi(ModNum);
		//if(retval < 0)
		//{
			//sprintf(ErrMSG, "*ERROR* (Pixie16WriteSglChanPar): ProgramFippi failed in module %d channel %d after downloading ExternDelayLen, retval=%d", ModNum, ChanNum, retval);
			//Pixie_Print_MSG(ErrMSG);
			//return(-4);
		//}
	}
	else if(strcmp(ChanParName,"FtrigoutDelay") == 0)
	{
		// Get the new FtrigoutDelay
		if(Module_Information[ModNum].Module_ADCMSPS == 100)
			ftrigoutdelay = (unsigned int)ROUND(ChanParData * (double)Module_Information[ModNum].Module_ADCMSPS);
		else if(Module_Information[ModNum].Module_ADCMSPS == 250)
			ftrigoutdelay = (unsigned int)ROUND(ChanParData * (double)(Module_Information[ModNum].Module_ADCMSPS / 2));
		else if(Module_Information[ModNum].Module_ADCMSPS == 500)
			ftrigoutdelay = (unsigned int)ROUND(ChanParData * (double)(Module_Information[ModNum].Module_ADCMSPS / 5));

		// Range check for FtrigoutDelay
		if(ftrigoutdelay < FASTTRIGBACKDELAY_MIN)
		{
			ftrigoutdelay = FASTTRIGBACKDELAY_MIN;
		}
		
		if ((Module_Information[ModNum].Module_Rev == 0xB) || (Module_Information[ModNum].Module_Rev == 0xC) || (Module_Information[ModNum].Module_Rev == 0xD))
		{
			if(ftrigoutdelay > FASTTRIGBACKDELAY_MAX_REVBCD)
			{
				ftrigoutdelay = FASTTRIGBACKDELAY_MAX_REVBCD;
			}
		}
		else if (Module_Information[ModNum].Module_Rev == 0xF)
		{
			if(ftrigoutdelay > FASTTRIGBACKDELAY_MAX_REVF)
			{
				ftrigoutdelay = FASTTRIGBACKDELAY_MAX_REVF;
			}
		}

		// Update DSP parameter FtrigoutDelay
		Pixie_Devices[ModNum].DSP_Parameter_Values[FtrigoutDelay_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS] = ftrigoutdelay;
		// Download to the selected Pixie module
		//Pixie16IMbufferIO(&ftrigoutdelay, 1, (unsigned int)(FtrigoutDelay_Address[ModNum] + ChanNum), MOD_WRITE, ModNum);

		// Program FiPPI to apply FtrigoutDelay settings to the FPGA
		//retval = Pixie16ProgramFippi(ModNum);
		//if(retval < 0)
		//{
			//sprintf(ErrMSG, "*ERROR* (Pixie16WriteSglChanPar): ProgramFippi failed in module %d channel %d after downloading FtrigoutDelay, retval=%d", ModNum, ChanNum, retval);
			//Pixie_Print_MSG(ErrMSG);
			//return(-4);
		//}
	}
	else if(strcmp(ChanParName,"ChanTrigStretch") == 0)
	{
		// Get the new ChanTrigStretch
		if(Module_Information[ModNum].Module_ADCMSPS == 100)
			chantrigstretch = (unsigned int)ROUND(ChanParData * (double)Module_Information[ModNum].Module_ADCMSPS);
		else if(Module_Information[ModNum].Module_ADCMSPS == 250)
			chantrigstretch = (unsigned int)ROUND(ChanParData * (double)(Module_Information[ModNum].Module_ADCMSPS / 2));
		else if(Module_Information[ModNum].Module_ADCMSPS == 500)
			chantrigstretch = (unsigned int)ROUND(ChanParData * (double)(Module_Information[ModNum].Module_ADCMSPS / 5));

		// Range check for ChanTrigStretch
		if(chantrigstretch < CHANTRIGSTRETCH_MIN)
		{
			chantrigstretch = CHANTRIGSTRETCH_MIN;
		}
		if(chantrigstretch > CHANTRIGSTRETCH_MAX)
		{
			chantrigstretch = CHANTRIGSTRETCH_MAX;
		}

		// Update DSP parameter ChanTrigStretch
		Pixie_Devices[ModNum].DSP_Parameter_Values[ChanTrigStretch_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS] = chantrigstretch;
		// Download to the selected Pixie module
		//Pixie16IMbufferIO(&chantrigstretch, 1, (unsigned int)(ChanTrigStretch_Address[ModNum] + ChanNum), MOD_WRITE, ModNum);

		// Program FiPPI to apply ChanTrigStretch settings to the FPGA
		//retval = Pixie16ProgramFippi(ModNum);
		//if(retval < 0)
		//{
			//sprintf(ErrMSG, "*ERROR* (Pixie16WriteSglChanPar): ProgramFippi failed in module %d channel %d after downloading ChanTrigStretch, retval=%d", ModNum, ChanNum, retval);
			//Pixie_Print_MSG(ErrMSG);
			//return(-4);
		//}
	}
	else
	{
		//sprintf(ErrMSG, "*ERROR* (Pixie16WriteSglChanPar): invalid channel parameter name %s", ChanParName);
		//Pixie_Print_MSG(ErrMSG);
		return(-E_PIXIE_UNKNOW_REG);
	}
	
	return(0);
}

/* code copied from Pixie_ComputeFIFO */
int pixie16_ctl::myPixie_ComputeFIFO(unsigned int TraceDelay, int ModNum,
		int ChanNum)
{
	unsigned int PAFLength, TriggerDelay, FifoLength;
	unsigned int SlowFilterRange, FastFilterRange, PeakSep;
	
	// Get the DSP parameter SlowFilterRange
	SlowFilterRange = Pixie_Devices[ModNum].DSP_Parameter_Values[SlowFilterRange_Address[ModNum] - DATA_MEMORY_ADDRESS];
	FastFilterRange = Pixie_Devices[ModNum].DSP_Parameter_Values[FastFilterRange_Address[ModNum] - DATA_MEMORY_ADDRESS];
	
	// Get the DSP parameter PeakSep
	PeakSep = Pixie_Devices[ModNum].DSP_Parameter_Values[PeakSep_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS];
	
	// Re-calculate TriggerDelay
	TriggerDelay = (unsigned int)(((double)PeakSep - 1.0) * pow(2.0, (double)SlowFilterRange));
	
	// Re-calculate PAFLength
	PAFLength = (unsigned int)((double)TriggerDelay / pow(2.0, (double)FastFilterRange))  + TraceDelay;
	
	// Get the current FIFO Length
	FifoLength = Pixie_Devices[ModNum].DSP_Parameter_Values[FIFOLength_Address[ModNum] - DATA_MEMORY_ADDRESS];
	
	if(PAFLength > FifoLength)  // PAFLength must be not larger than FifoLength
	{
		PAFLength = FifoLength - 1; // Keep TraceDelay while reducing TriggerDelay
		TriggerDelay = (PAFLength - TraceDelay) * (unsigned int)pow(2.0, (double)FastFilterRange);
	}
	
	// Update the DSP parameter TriggerDelay
	Pixie_Devices[ModNum].DSP_Parameter_Values[TriggerDelay_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS] = TriggerDelay;
	// Download to the selected Pixie module
	//Pixie16IMbufferIO(&TriggerDelay, 1, (unsigned int)(TriggerDelay_Address[ModNum] + ChanNum), MOD_WRITE, ModNum);
	
	// Update the DSP parameter PAFLength
	Pixie_Devices[ModNum].DSP_Parameter_Values[PAFlength_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS] = PAFLength;
	// Download to the selected Pixie module
	//Pixie16IMbufferIO(&PAFLength, 1, (unsigned int)(PAFlength_Address[ModNum] + ChanNum), MOD_WRITE, ModNum);
	
	return(0);
}

int pixie16_ctl::myPixie16ReadSglChanPar(const char* ChanParName, double*
		ChanParData, int ModNum, int ChanNum)
{
	unsigned int FL, FG, SL, SG, FastFilterRange, SlowFilterRange;
	unsigned int fastthresh, preamptau, tracelength;
	unsigned int paflength, triggerdelay, offsetdac;
	unsigned int xwait;
	unsigned int baselinepercent, energylow, log2ebin, chancsra, chancsrb;
	unsigned int baselinecut, fasttrigbacklen, baselineaverage;
	unsigned int cfddelay, cfdscale, qdclen, exttrigstretch, vetostretch, externdelaylen, multiplicitymaskl, multiplicitymaskh, ftrigoutdelay;
	unsigned int chantrigstretch, cfdthresh;

	// Read channel parameter
	if(strcmp(ChanParName,"TRIGGER_RISETIME") == 0)
	{
		
		// Read from the selected Pixie module
		//Pixie16IMbufferIO(&FL, 1, (unsigned int)(FastLength_Address[ModNum] + ChanNum), MOD_READ, ModNum);
		FL = Pixie_Devices[ModNum].DSP_Parameter_Values[FastLength_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS];
		
		//Pixie16IMbufferIO(&FastFilterRange, 1, FastFilterRange_Address[ModNum], MOD_READ, ModNum);
		FastFilterRange = Pixie_Devices[ModNum].DSP_Parameter_Values[FastFilterRange_Address[ModNum] - DATA_MEMORY_ADDRESS];
		
		// Update channel parameter TriggerRiseTime
		if(Module_Information[ModNum].Module_ADCMSPS == 100)
			*ChanParData = (double)FL * pow(2.0, (double)FastFilterRange) / (double)Module_Information[ModNum].Module_ADCMSPS;
		else if(Module_Information[ModNum].Module_ADCMSPS == 250)
			*ChanParData = (double)FL * pow(2.0, (double)FastFilterRange) / (double)(Module_Information[ModNum].Module_ADCMSPS / 2);
		else if(Module_Information[ModNum].Module_ADCMSPS == 500)
			*ChanParData = (double)FL * pow(2.0, (double)FastFilterRange) / (double)(Module_Information[ModNum].Module_ADCMSPS / 5);
	
	}
	else if(strcmp(ChanParName,"TRIGGER_FLATTOP") == 0)
	{
		
		// Read from the selected Pixie module
		//Pixie16IMbufferIO(&FG, 1, (unsigned int)(FastGap_Address[ModNum] + ChanNum), MOD_READ, ModNum);
		FG = Pixie_Devices[ModNum].DSP_Parameter_Values[FastGap_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS];
		
		//Pixie16IMbufferIO(&FastFilterRange, 1, FastFilterRange_Address[ModNum], MOD_READ, ModNum);
		FastFilterRange = Pixie_Devices[ModNum].DSP_Parameter_Values[FastFilterRange_Address[ModNum] - DATA_MEMORY_ADDRESS];
		
		// Update channel parameter TriggerFlatTop
		if(Module_Information[ModNum].Module_ADCMSPS == 100)
			*ChanParData = (double)FG * pow(2.0, (double)FastFilterRange) / (double)Module_Information[ModNum].Module_ADCMSPS;
		else if(Module_Information[ModNum].Module_ADCMSPS == 250)
			*ChanParData = (double)FG * pow(2.0, (double)FastFilterRange) / (double)(Module_Information[ModNum].Module_ADCMSPS / 2);
		else if(Module_Information[ModNum].Module_ADCMSPS == 500)
			*ChanParData = (double)FG * pow(2.0, (double)FastFilterRange) / (double)(Module_Information[ModNum].Module_ADCMSPS / 5);
		
	}
	else if(strcmp(ChanParName,"TRIGGER_THRESHOLD") == 0)
	{
		
		// Read from the selected Pixie module
		//Pixie16IMbufferIO(&fastthresh, 1, (unsigned int)(FastThresh_Address[ModNum] + ChanNum), MOD_READ, ModNum);
		fastthresh = Pixie_Devices[ModNum].DSP_Parameter_Values[FastThresh_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS];
		
		//Pixie16IMbufferIO(&FL, 1, (unsigned int)(FastLength_Address[ModNum] + ChanNum), MOD_READ, ModNum);
		FL = Pixie_Devices[ModNum].DSP_Parameter_Values[FastLength_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS];
		
		// Update channel parameter TriggerThreshold
		if(Module_Information[ModNum].Module_ADCMSPS == 100)
			*ChanParData = (double)fastthresh / (double)FL;
		else if(Module_Information[ModNum].Module_ADCMSPS == 250)
			*ChanParData = (double)fastthresh / ((double)FL * 2.0);
		else if(Module_Information[ModNum].Module_ADCMSPS == 500)
			*ChanParData = (double)fastthresh / ((double)FL * 5.0);

	}
	else if(strcmp(ChanParName,"ENERGY_RISETIME") == 0)
	{
		
		// Read from the selected Pixie module
		//Pixie16IMbufferIO(&SL, 1, (unsigned int)(SlowLength_Address[ModNum] + ChanNum), MOD_READ, ModNum);
		SL = Pixie_Devices[ModNum].DSP_Parameter_Values[SlowLength_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS];
		
		//Pixie16IMbufferIO(&SlowFilterRange, 1, SlowFilterRange_Address[ModNum], MOD_READ, ModNum);
		SlowFilterRange = Pixie_Devices[ModNum].DSP_Parameter_Values[SlowFilterRange_Address[ModNum] - DATA_MEMORY_ADDRESS];
		
		// Update channel parameter EnergyRiseTime
		if(Module_Information[ModNum].Module_ADCMSPS == 100)
			*ChanParData = (double)SL * pow(2.0, (double)SlowFilterRange) / (double)Module_Information[ModNum].Module_ADCMSPS;
		else if(Module_Information[ModNum].Module_ADCMSPS == 250)
			*ChanParData = (double)SL * pow(2.0, (double)SlowFilterRange) / (double)(Module_Information[ModNum].Module_ADCMSPS / 2);
		else if(Module_Information[ModNum].Module_ADCMSPS == 500)
			*ChanParData = (double)SL * pow(2.0, (double)SlowFilterRange) / (double)(Module_Information[ModNum].Module_ADCMSPS / 5);
		
	}
	else if(strcmp(ChanParName,"ENERGY_FLATTOP") == 0)
	{
		
		// Read from the selected Pixie module
		//Pixie16IMbufferIO(&SG, 1, (unsigned int)(SlowGap_Address[ModNum] + ChanNum), MOD_READ, ModNum);
		SG = Pixie_Devices[ModNum].DSP_Parameter_Values[SlowGap_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS];
		
		//Pixie16IMbufferIO(&SlowFilterRange, 1, SlowFilterRange_Address[ModNum], MOD_READ, ModNum);
		SlowFilterRange = Pixie_Devices[ModNum].DSP_Parameter_Values[SlowFilterRange_Address[ModNum] - DATA_MEMORY_ADDRESS];
		
		// Update channel parameter EnergyFlatTop
		if(Module_Information[ModNum].Module_ADCMSPS == 100)
			*ChanParData = (double)SG * pow(2.0, (double)SlowFilterRange) / (double)Module_Information[ModNum].Module_ADCMSPS;
		else if(Module_Information[ModNum].Module_ADCMSPS == 250)
			*ChanParData = (double)SG * pow(2.0, (double)SlowFilterRange) / (double)(Module_Information[ModNum].Module_ADCMSPS / 2);
		else if(Module_Information[ModNum].Module_ADCMSPS == 500)
			*ChanParData = (double)SG * pow(2.0, (double)SlowFilterRange) / (double)(Module_Information[ModNum].Module_ADCMSPS / 5);

	}
	else if(strcmp(ChanParName,"TAU") == 0)
	{
		
		// Read from the selected Pixie module
		//Pixie16IMbufferIO(&preamptau, 1, (unsigned int)(PreampTau_Address[ModNum] + ChanNum), MOD_READ, ModNum);
		preamptau = Pixie_Devices[ModNum].DSP_Parameter_Values[PreampTau_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS];
		
		// Update channel parameter PreampTau
		*ChanParData = IEEEFloating2Decimal(preamptau);
		
	}
	else if(strcmp(ChanParName,"TRACE_LENGTH") == 0)
	{
		
		// Read from the selected Pixie module
		//Pixie16IMbufferIO(&tracelength, 1, (unsigned int)(TraceLength_Address[ModNum] + ChanNum), MOD_READ, ModNum);
		tracelength = Pixie_Devices[ModNum].DSP_Parameter_Values[TraceLength_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS];
		//Pixie16IMbufferIO(&FastFilterRange, 1, FastFilterRange_Address[ModNum], MOD_READ, ModNum);
		FastFilterRange = Pixie_Devices[ModNum].DSP_Parameter_Values[FastFilterRange_Address[ModNum] - DATA_MEMORY_ADDRESS];
		
		// Update channel parameter TraceLength
		*ChanParData = (double)tracelength / (double)Module_Information[ModNum].Module_ADCMSPS * pow(2.0, (double)FastFilterRange);
		
	}
	else if(strcmp(ChanParName,"TRACE_DELAY") == 0)
	{
		
		// Read from the selected Pixie module
		//Pixie16IMbufferIO(&paflength, 1, (unsigned int)(PAFlength_Address[ModNum] + ChanNum), MOD_READ, ModNum);
		paflength = Pixie_Devices[ModNum].DSP_Parameter_Values[PAFlength_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS];
		//Pixie16IMbufferIO(&triggerdelay, 1, (unsigned int)(TriggerDelay_Address[ModNum] + ChanNum), MOD_READ, ModNum);
		triggerdelay = Pixie_Devices[ModNum].DSP_Parameter_Values[TriggerDelay_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS];
		//Pixie16IMbufferIO(&FastFilterRange, 1, FastFilterRange_Address[ModNum], MOD_READ, ModNum);
		FastFilterRange = Pixie_Devices[ModNum].DSP_Parameter_Values[FastFilterRange_Address[ModNum] - DATA_MEMORY_ADDRESS];
		
		// Update channel parameter TraceDelay
		if(Module_Information[ModNum].Module_ADCMSPS == 100)
			*ChanParData = (double)(paflength - (unsigned int)((double)triggerdelay / pow(2.0, (double)FastFilterRange))) / (double)Module_Information[ModNum].Module_ADCMSPS * pow(2.0, (double)FastFilterRange);
		else if(Module_Information[ModNum].Module_ADCMSPS == 250)
			*ChanParData = (double)(paflength - (unsigned int)((double)triggerdelay / pow(2.0, (double)FastFilterRange))) / (double)(Module_Information[ModNum].Module_ADCMSPS / 2) * pow(2.0, (double)FastFilterRange);
		else if(Module_Information[ModNum].Module_ADCMSPS == 500)
			*ChanParData = (double)(paflength - (unsigned int)((double)triggerdelay / pow(2.0, (double)FastFilterRange))) / (double)(Module_Information[ModNum].Module_ADCMSPS / 5) * pow(2.0, (double)FastFilterRange);
		
	}
	else if(strcmp(ChanParName,"VOFFSET") == 0)
	{
		
		// Read from the selected Pixie module
		//Pixie16IMbufferIO(&offsetdac, 1, (unsigned int)(OffsetDAC_Address[ModNum] + ChanNum), MOD_READ, ModNum);
		offsetdac = Pixie_Devices[ModNum].DSP_Parameter_Values[OffsetDAC_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS];
		
		// Update channel parameter DCoffset
		*ChanParData = (double)offsetdac / 65536.0 * DAC_VOLTAGE_RANGE - DAC_VOLTAGE_RANGE / 2.0;
		
	}
	else if(strcmp(ChanParName,"XDT") == 0)
	{
		// Read from the selected Pixie module
		//Pixie16IMbufferIO(&xwait, 1, (unsigned int)(Xwait_Address[ModNum] + ChanNum), MOD_READ, ModNum);
		xwait = Pixie_Devices[ModNum].DSP_Parameter_Values[Xwait_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS];
		
		// Update channel parameter Xdt
		*ChanParData = (double)xwait / (double)DSP_CLOCK_MHZ;		
	}
	else if(strcmp(ChanParName,"BASELINE_PERCENT") == 0)
	{
		
		// Read from the selected Pixie module
		//Pixie16IMbufferIO(&baselinepercent, 1, (unsigned int)(BaselinePercent_Address[ModNum] + ChanNum), MOD_READ, ModNum);
		baselinepercent = Pixie_Devices[ModNum].DSP_Parameter_Values[BaselinePercent_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS];
		
		// Update channel parameter BaselinePercent
		*ChanParData = (double)baselinepercent;
		
	}
	else if(strcmp(ChanParName,"EMIN") == 0)
	{
		
		// Read from the selected Pixie module
		//Pixie16IMbufferIO(&energylow, 1, (unsigned int)(EnergyLow_Address[ModNum] + ChanNum), MOD_READ, ModNum);
		energylow = Pixie_Devices[ModNum].DSP_Parameter_Values[EnergyLow_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS];
		
		// Update channel parameter EnergyLow
		*ChanParData = (double)energylow;
		
	}
	else if(strcmp(ChanParName,"BINFACTOR") == 0)
	{
		
		// Read from the selected Pixie module
		//Pixie16IMbufferIO(&log2ebin, 1, (unsigned int)(Log2Ebin_Address[ModNum] + ChanNum), MOD_READ, ModNum);
		log2ebin = Pixie_Devices[ModNum].DSP_Parameter_Values[Log2Ebin_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS];
		
		// Update channel parameter BinFactor
		*ChanParData = (double)(pow(2.0, 32.0) - log2ebin);
		
	}
	else if(strcmp(ChanParName,"BASELINE_AVERAGE") == 0)
	{
		
		// Read from the selected Pixie module
		//Pixie16IMbufferIO(&baselineaverage, 1, (unsigned int)(Log2Bweight_Address[ModNum] + ChanNum), MOD_READ, ModNum);
		baselineaverage = Pixie_Devices[ModNum].DSP_Parameter_Values[Log2Bweight_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS];
		
		// Update channel parameter Baseline Average
		if(baselineaverage == 0)
		{
			*ChanParData = 0.0;
		}
		else
		{
			*ChanParData = (double)(pow(2.0, 32.0) - baselineaverage);
		}
		
	}
	else if(strcmp(ChanParName,"CHANNEL_CSRA") == 0)
	{
		
		// Read from the selected Pixie module
		//Pixie16IMbufferIO(&chancsra, 1, (unsigned int)(ChanCSRa_Address[ModNum] + ChanNum), MOD_READ, ModNum);
		chancsra = Pixie_Devices[ModNum].DSP_Parameter_Values[ChanCSRa_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS];
		
		// Update channel parameter ChanCSRA
		*ChanParData = (double)chancsra;
		
	}
	else if(strcmp(ChanParName,"CHANNEL_CSRB") == 0)
	{
		
		// Read from the selected Pixie module
		//Pixie16IMbufferIO(&chancsrb, 1, (unsigned int)(ChanCSRb_Address[ModNum] + ChanNum), MOD_READ, ModNum);
		chancsrb = Pixie_Devices[ModNum].DSP_Parameter_Values[ChanCSRb_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS];
		
		// Update channel parameter ChanCSRB
		*ChanParData = (double)chancsrb;
		
	}
	else if(strcmp(ChanParName,"BLCUT") == 0)
	{
		
		// Read from the selected Pixie module
		//Pixie16IMbufferIO(&baselinecut, 1, (unsigned int)(BLcut_Address[ModNum] + ChanNum), MOD_READ, ModNum);
		baselinecut = Pixie_Devices[ModNum].DSP_Parameter_Values[BLcut_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS];
		
		// Update channel parameter BaselineCut
		*ChanParData = (double)baselinecut;
		
	}
	else if(strcmp(ChanParName,"FASTTRIGBACKLEN") == 0)
	{
		
		// Read from the selected Pixie module
		//Pixie16IMbufferIO(&fasttrigbacklen, 1, (unsigned int)(FastTrigBackLen_Address[ModNum] + ChanNum), MOD_READ, ModNum);
		fasttrigbacklen = Pixie_Devices[ModNum].DSP_Parameter_Values[FastTrigBackLen_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS];
		
		// Update channel parameter FastTrigBackLen
		if(Module_Information[ModNum].Module_ADCMSPS == 100)
			*ChanParData = (double)fasttrigbacklen / (double)Module_Information[ModNum].Module_ADCMSPS;
		else if(Module_Information[ModNum].Module_ADCMSPS == 250)
			*ChanParData = (double)fasttrigbacklen / (double)(Module_Information[ModNum].Module_ADCMSPS / 2);
		else if(Module_Information[ModNum].Module_ADCMSPS == 500)
			*ChanParData = (double)fasttrigbacklen / (double)(Module_Information[ModNum].Module_ADCMSPS / 5);
		
	}
	else if(strcmp(ChanParName,"CFDDelay") == 0)
	{
		// Read from the selected Pixie module
		//Pixie16IMbufferIO(&cfddelay, 1, (unsigned int)(CFDDelay_Address[ModNum] + ChanNum), MOD_READ, ModNum);
		cfddelay = Pixie_Devices[ModNum].DSP_Parameter_Values[CFDDelay_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS];
		
		// Update channel parameter CFDDelay
		if(Module_Information[ModNum].Module_ADCMSPS == 100)
			*ChanParData = (double)cfddelay / (double)Module_Information[ModNum].Module_ADCMSPS;
		else if(Module_Information[ModNum].Module_ADCMSPS == 250)
			*ChanParData = (double)cfddelay / (double)(Module_Information[ModNum].Module_ADCMSPS / 2);
		else if(Module_Information[ModNum].Module_ADCMSPS == 500)
			*ChanParData = (double)cfddelay / (double)(Module_Information[ModNum].Module_ADCMSPS / 5);

	}
	else if(strcmp(ChanParName,"CFDScale") == 0)
	{
		// Read from the selected Pixie module
		//Pixie16IMbufferIO(&cfdscale, 1, (unsigned int)(CFDScale_Address[ModNum] + ChanNum), MOD_READ, ModNum);
		cfdscale = Pixie_Devices[ModNum].DSP_Parameter_Values[CFDScale_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS];
		
		// Update channel parameter CFDScale
		*ChanParData = (double)cfdscale;
	}
	else if(strcmp(ChanParName,"CFDThresh") == 0)
	{
		// Read from the selected Pixie module
		//Pixie16IMbufferIO(&cfdthresh, 1, (unsigned int)(CFDThresh_Address[ModNum] + ChanNum), MOD_READ, ModNum);
		cfdthresh = Pixie_Devices[ModNum].DSP_Parameter_Values[CFDThresh_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS];
		
		// Update channel parameter CFDThresh
		*ChanParData = (double)cfdthresh;
	}
	else if(strcmp(ChanParName,"QDCLen0") == 0)
	{
		// Read from the selected Pixie module
		//Pixie16IMbufferIO(&qdclen, 1, (unsigned int)(QDCLen0_Address[ModNum] + ChanNum), MOD_READ, ModNum);
		qdclen = Pixie_Devices[ModNum].DSP_Parameter_Values[QDCLen0_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS];
		
		// Update channel parameter QDCLen0
		if(Module_Information[ModNum].Module_ADCMSPS == 500)
			*ChanParData = (double)qdclen / (double)(Module_Information[ModNum].Module_ADCMSPS / 5);
		else
			*ChanParData = (double)qdclen / (double)Module_Information[ModNum].Module_ADCMSPS;
	}
	else if(strcmp(ChanParName,"QDCLen1") == 0)
	{
		// Read from the selected Pixie module
		//Pixie16IMbufferIO(&qdclen, 1, (unsigned int)(QDCLen1_Address[ModNum] + ChanNum), MOD_READ, ModNum);
		qdclen = Pixie_Devices[ModNum].DSP_Parameter_Values[QDCLen1_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS];
		
		if(Module_Information[ModNum].Module_ADCMSPS == 500)
			*ChanParData = (double)qdclen / (double)(Module_Information[ModNum].Module_ADCMSPS / 5);
		else
			*ChanParData = (double)qdclen / (double)Module_Information[ModNum].Module_ADCMSPS;
	}
	else if(strcmp(ChanParName,"QDCLen2") == 0)
	{
		// Read from the selected Pixie module
		//Pixie16IMbufferIO(&qdclen, 1, (unsigned int)(QDCLen2_Address[ModNum] + ChanNum), MOD_READ, ModNum);
		qdclen = Pixie_Devices[ModNum].DSP_Parameter_Values[QDCLen2_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS];
		
		if(Module_Information[ModNum].Module_ADCMSPS == 500)
			*ChanParData = (double)qdclen / (double)(Module_Information[ModNum].Module_ADCMSPS / 5);
		else
			*ChanParData = (double)qdclen / (double)Module_Information[ModNum].Module_ADCMSPS;
	}
	else if(strcmp(ChanParName,"QDCLen3") == 0)
	{
		// Read from the selected Pixie module
		//Pixie16IMbufferIO(&qdclen, 1, (unsigned int)(QDCLen3_Address[ModNum] + ChanNum), MOD_READ, ModNum);
		qdclen = Pixie_Devices[ModNum].DSP_Parameter_Values[QDCLen3_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS];
		
		if(Module_Information[ModNum].Module_ADCMSPS == 500)
			*ChanParData = (double)qdclen / (double)(Module_Information[ModNum].Module_ADCMSPS / 5);
		else
			*ChanParData = (double)qdclen / (double)Module_Information[ModNum].Module_ADCMSPS;
	}
	else if(strcmp(ChanParName,"QDCLen4") == 0)
	{
		// Read from the selected Pixie module
		//Pixie16IMbufferIO(&qdclen, 1, (unsigned int)(QDCLen4_Address[ModNum] + ChanNum), MOD_READ, ModNum);
		qdclen = Pixie_Devices[ModNum].DSP_Parameter_Values[QDCLen4_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS];
		
		if(Module_Information[ModNum].Module_ADCMSPS == 500)
			*ChanParData = (double)qdclen / (double)(Module_Information[ModNum].Module_ADCMSPS / 5);
		else
			*ChanParData = (double)qdclen / (double)Module_Information[ModNum].Module_ADCMSPS;
	}
	else if(strcmp(ChanParName,"QDCLen5") == 0)
	{
		// Read from the selected Pixie module
		//Pixie16IMbufferIO(&qdclen, 1, (unsigned int)(QDCLen5_Address[ModNum] + ChanNum), MOD_READ, ModNum);
		qdclen = Pixie_Devices[ModNum].DSP_Parameter_Values[QDCLen5_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS];
		
		if(Module_Information[ModNum].Module_ADCMSPS == 500)
			*ChanParData = (double)qdclen / (double)(Module_Information[ModNum].Module_ADCMSPS / 5);
		else
			*ChanParData = (double)qdclen / (double)Module_Information[ModNum].Module_ADCMSPS;
	}
	else if(strcmp(ChanParName,"QDCLen6") == 0)
	{
		// Read from the selected Pixie module
		//Pixie16IMbufferIO(&qdclen, 1, (unsigned int)(QDCLen6_Address[ModNum] + ChanNum), MOD_READ, ModNum);
		qdclen = Pixie_Devices[ModNum].DSP_Parameter_Values[QDCLen6_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS];
		
		if(Module_Information[ModNum].Module_ADCMSPS == 500)
			*ChanParData = (double)qdclen / (double)(Module_Information[ModNum].Module_ADCMSPS / 5);
		else
			*ChanParData = (double)qdclen / (double)Module_Information[ModNum].Module_ADCMSPS;
	}
	else if(strcmp(ChanParName,"QDCLen7") == 0)
	{
		// Read from the selected Pixie module
		//Pixie16IMbufferIO(&qdclen, 1, (unsigned int)(QDCLen7_Address[ModNum] + ChanNum), MOD_READ, ModNum);
		qdclen = Pixie_Devices[ModNum].DSP_Parameter_Values[QDCLen7_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS];
		
		if(Module_Information[ModNum].Module_ADCMSPS == 500)
			*ChanParData = (double)qdclen / (double)(Module_Information[ModNum].Module_ADCMSPS / 5);
		else
			*ChanParData = (double)qdclen / (double)Module_Information[ModNum].Module_ADCMSPS;
	}
	else if(strcmp(ChanParName,"ExtTrigStretch") == 0)
	{
		// Read from the selected Pixie module
		//Pixie16IMbufferIO(&exttrigstretch, 1, (unsigned int)(ExtTrigStretch_Address[ModNum] + ChanNum), MOD_READ, ModNum);
		exttrigstretch = Pixie_Devices[ModNum].DSP_Parameter_Values[ExtTrigStretch_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS];
		
		// Update channel parameter ExtTrigStretch
		if(Module_Information[ModNum].Module_ADCMSPS == 100)
			*ChanParData = (double)exttrigstretch / (double)Module_Information[ModNum].Module_ADCMSPS;
		else if(Module_Information[ModNum].Module_ADCMSPS == 250)
			*ChanParData = (double)exttrigstretch / (double)(Module_Information[ModNum].Module_ADCMSPS / 2);
		else if(Module_Information[ModNum].Module_ADCMSPS == 500)
			*ChanParData = (double)exttrigstretch / (double)(Module_Information[ModNum].Module_ADCMSPS / 5);
	}
	else if(strcmp(ChanParName,"VetoStretch") == 0)
	{
		// Read from the selected Pixie module
		//Pixie16IMbufferIO(&vetostretch, 1, (unsigned int)(VetoStretch_Address[ModNum] + ChanNum), MOD_READ, ModNum);
		vetostretch = Pixie_Devices[ModNum].DSP_Parameter_Values[VetoStretch_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS];
		
		// Update channel parameter VetoStretch
		if(Module_Information[ModNum].Module_ADCMSPS == 100)
			*ChanParData = (double)vetostretch / (double)Module_Information[ModNum].Module_ADCMSPS;
		else if(Module_Information[ModNum].Module_ADCMSPS == 250)
			*ChanParData = (double)vetostretch / (double)(Module_Information[ModNum].Module_ADCMSPS / 2);
		else if(Module_Information[ModNum].Module_ADCMSPS == 500)
			*ChanParData = (double)vetostretch / (double)(Module_Information[ModNum].Module_ADCMSPS / 5);
	}
	else if(strcmp(ChanParName,"MultiplicityMaskL") == 0)
	{
		// Read from the selected Pixie module
		//Pixie16IMbufferIO(&multiplicitymaskl, 1, (unsigned int)(MultiplicityMaskL_Address[ModNum] + ChanNum), MOD_READ, ModNum);
		multiplicitymaskl = Pixie_Devices[ModNum].DSP_Parameter_Values[MultiplicityMaskL_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS];
		
		// Update channel parameter MultiplicityMaskL
		*ChanParData = (double)multiplicitymaskl;
	}
	else if(strcmp(ChanParName,"MultiplicityMaskH") == 0)
	{
		// Read from the selected Pixie module
		//Pixie16IMbufferIO(&multiplicitymaskh, 1, (unsigned int)(MultiplicityMaskH_Address[ModNum] + ChanNum), MOD_READ, ModNum);
		multiplicitymaskh = Pixie_Devices[ModNum].DSP_Parameter_Values[MultiplicityMaskH_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS];
		
		// Update channel parameter MultiplicityMaskH
		*ChanParData = (double)multiplicitymaskh;
	}
	else if(strcmp(ChanParName,"ExternDelayLen") == 0)
	{
		// Read from the selected Pixie module
		//Pixie16IMbufferIO(&externdelaylen, 1, (unsigned int)(ExternDelayLen_Address[ModNum] + ChanNum), MOD_READ, ModNum);
		externdelaylen = Pixie_Devices[ModNum].DSP_Parameter_Values[ExternDelayLen_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS];
		
		// Update channel parameter ExternDelayLen
		if(Module_Information[ModNum].Module_ADCMSPS == 100)
			*ChanParData = (double)externdelaylen / (double)Module_Information[ModNum].Module_ADCMSPS;
		else if(Module_Information[ModNum].Module_ADCMSPS == 250)
			*ChanParData = (double)externdelaylen / (double)(Module_Information[ModNum].Module_ADCMSPS / 2);
		else if(Module_Information[ModNum].Module_ADCMSPS == 500)
			*ChanParData = (double)externdelaylen / (double)(Module_Information[ModNum].Module_ADCMSPS / 5);
	}
	else if(strcmp(ChanParName,"FtrigoutDelay") == 0)
	{
		// Read from the selected Pixie module
		//Pixie16IMbufferIO(&ftrigoutdelay, 1, (unsigned int)(FtrigoutDelay_Address[ModNum] + ChanNum), MOD_READ, ModNum);
		ftrigoutdelay = Pixie_Devices[ModNum].DSP_Parameter_Values[FtrigoutDelay_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS];
		
		// Update channel parameter FtrigoutDelay
		if(Module_Information[ModNum].Module_ADCMSPS == 100)
			*ChanParData = (double)ftrigoutdelay / (double)Module_Information[ModNum].Module_ADCMSPS;
		else if(Module_Information[ModNum].Module_ADCMSPS == 250)
			*ChanParData = (double)ftrigoutdelay / (double)(Module_Information[ModNum].Module_ADCMSPS / 2);
		else if(Module_Information[ModNum].Module_ADCMSPS == 500)
			*ChanParData = (double)ftrigoutdelay / (double)(Module_Information[ModNum].Module_ADCMSPS / 5);
	}
	else if(strcmp(ChanParName,"ChanTrigStretch") == 0)
	{
		// Read from the selected Pixie module
		//Pixie16IMbufferIO(&chantrigstretch, 1, (unsigned int)(ChanTrigStretch_Address[ModNum] + ChanNum), MOD_READ, ModNum);
		chantrigstretch = Pixie_Devices[ModNum].DSP_Parameter_Values[ChanTrigStretch_Address[ModNum] + ChanNum - DATA_MEMORY_ADDRESS];
		
		// Update channel parameter ChanTrigStretch
		if(Module_Information[ModNum].Module_ADCMSPS == 100)
			*ChanParData = (double)chantrigstretch / (double)Module_Information[ModNum].Module_ADCMSPS;
		else if(Module_Information[ModNum].Module_ADCMSPS == 250)
			*ChanParData = (double)chantrigstretch / (double)(Module_Information[ModNum].Module_ADCMSPS / 2);
		else if(Module_Information[ModNum].Module_ADCMSPS == 500)
			*ChanParData = (double)chantrigstretch / (double)(Module_Information[ModNum].Module_ADCMSPS / 5);
	}
	else
	{
		return -E_PIXIE_UNKNOW_REG;
	}
	
	return(0);
}

int pixie16_ctl::myPixie16WriteSglModPar(const char* ModParName, uint32_t
		ModParData, int ModNum)
{
	unsigned short k;
	unsigned int paflength, triggerdelay, tracedelay, fastfilterrange, LastFastFilterRange, baselinecut;
	unsigned int CSR, slowfilterrange;
	int retval;
	unsigned int CPLD_CSR;
	
	
	// Write module parameter
	if(strcmp(ModParName,"MODULE_CSRA") == 0)
	{
		// Update the DSP parameter ModCSRA
		Pixie_Devices[ModNum].DSP_Parameter_Values[ModCSRA_Address[ModNum] - DATA_MEMORY_ADDRESS] = ModParData;
		
		// Download to the selected Pixie module
		//Pixie16IMbufferIO(&ModParData, 1, ModCSRA_Address[ModNum], MOD_WRITE, ModNum);
		
		// Program FiPPI to apply ModCSRA settings to the FPGA
		//retval = Pixie16ProgramFippi(ModNum);
		//if(retval < 0)
		//{
			//sprintf(ErrMSG, "*ERROR* (Pixie16WriteSglModPar): ProgramFippi failed after downloading ModCSRA, retval=%d", retval);
			//Pixie_Print_MSG(ErrMSG);
			//return(-3);
		//}
	}
	else if(strcmp(ModParName,"MODULE_CSRB") == 0)
	{
		// Update the DSP parameter ModCSRB
		Pixie_Devices[ModNum].DSP_Parameter_Values[ModCSRB_Address[ModNum] - DATA_MEMORY_ADDRESS] = ModParData;
		
		// Download to the selected Pixie module
		//Pixie16IMbufferIO(&ModParData, 1, ModCSRB_Address[ModNum], MOD_WRITE, ModNum);

		// Program FiPPI to apply ModCSRB settings to the FPGA
		//retval = Pixie16ProgramFippi(ModNum);
		//if(retval < 0)
		//{
			//sprintf(ErrMSG, "*ERROR* (Pixie16WriteSglModPar): ProgramFippi failed after downloading ModCSRB, retval=%d", retval);
			//Pixie_Print_MSG(ErrMSG);
			//return(-3);
		//}

		//**************************************************
		// Set up Pull-up resistors
		//**************************************************
		
		//CPLD_CSR = 0xAAA;

		// Pull-up
		//if( APP32_TstBit(MODCSRB_CPLDPULLUP, ModParData) )
		//{
			//CPLD_CSR = APP32_SetBit(CPLDCSR_PULLUP, CPLD_CSR);
		//}
		//else
		//{
			//CPLD_CSR = APP32_ClrBit(CPLDCSR_PULLUP, CPLD_CSR);
		//}
//
		// Enable connections of PXI nearest neighbor lines (J2) onto the
		// backplane if the module is a Rev-B or C module
		//if ((Module_Information[ModNum].Module_Rev == 0xB) || (Module_Information[ModNum].Module_Rev == 0xC))
		//{
			//CPLD_CSR = APP32_SetBit(CPLDCSR_BPCONNECT, CPLD_CSR);
		//}
//
		//Pixie_Register_IO(ModNum, CFG_CTRLCS, MOD_WRITE, &CPLD_CSR);
//
		////**************************************************
		//// Set pullups for the SYNCH lines on the backplane
		////**************************************************
	//
		//Pixie_ReadCSR(ModNum, &CSR);
		//if( APP32_TstBit(MODCSRB_CHASSISMASTER, ModParData) )
		//{
			//CSR = APP32_SetBit(PULLUP_CTRL, CSR);
		//}
		//else
		//{
			//CSR = APP32_ClrBit(PULLUP_CTRL, CSR);
		//}
		//Pixie_WrtCSR(ModNum, CSR);

	}
	else if(strcmp(ModParName,"MODULE_FORMAT") == 0)
	{
		// Update the DSP parameter ModFormat
		Pixie_Devices[ModNum].DSP_Parameter_Values[ModFormat_Address[ModNum] - DATA_MEMORY_ADDRESS] = ModParData;
		
		// Download to the selected Pixie module
		//Pixie16IMbufferIO(&ModParData, 1, ModFormat_Address[ModNum], MOD_WRITE, ModNum);
	}
	else if(strcmp(ModParName,"MAX_EVENTS") == 0)
	{
		// Update the DSP parameter MaxEvents
		Pixie_Devices[ModNum].DSP_Parameter_Values[MaxEvents_Address[ModNum] - DATA_MEMORY_ADDRESS] = ModParData;
		
		// Download to the selected Pixie module
		//Pixie16IMbufferIO(&ModParData, 1, MaxEvents_Address[ModNum], MOD_WRITE, ModNum);
	}
	else if(strcmp(ModParName,"SYNCH_WAIT") == 0)
	{
		// Update the DSP parameter SynchWait
		Pixie_Devices[ModNum].DSP_Parameter_Values[SynchWait_Address[ModNum] - DATA_MEMORY_ADDRESS] = ModParData;
		
		// Download to the selected Pixie module
		//Pixie16IMbufferIO(&ModParData, 1, SynchWait_Address[ModNum], MOD_WRITE, ModNum);
		
		// When changing SynchWait in one Pixie module, we need to broadcast it to all other modules as well
		//Pixie_Broadcast("SYNCH_WAIT", ModNum);
	}
	else if(strcmp(ModParName,"IN_SYNCH") == 0)
	{
		// Update the DSP parameter InSynch
		Pixie_Devices[ModNum].DSP_Parameter_Values[InSynch_Address[ModNum] - DATA_MEMORY_ADDRESS] = ModParData;
		
		// Download to the selected Pixie module
		//Pixie16IMbufferIO(&ModParData, 1, InSynch_Address[ModNum], MOD_WRITE, ModNum);
		
		// When changing InSynch in one Pixie module, we need to broadcast it to all other modules as well
		//Pixie_Broadcast("IN_SYNCH", ModNum);
	}
	else if(strcmp(ModParName,"SLOW_FILTER_RANGE") == 0)
	{
		// Check slow filter range limit
		slowfilterrange = ModParData;

		if(slowfilterrange > SLOWFILTERRANGE_MAX)
		{
			slowfilterrange = SLOWFILTERRANGE_MAX;

			//sprintf(ErrMSG, "*WARNING* (Pixie16WriteSglModPar): Maximum SlowFilterRange that is currently supported is %d", slowfilterrange);
			//Pixie_Print_MSG(ErrMSG);
		}
		if(slowfilterrange < SLOWFILTERRANGE_MIN)
		{
			slowfilterrange = SLOWFILTERRANGE_MIN;

			//sprintf(ErrMSG, "*WARNING* (Pixie16WriteSglModPar): Minimum SlowFilterRange that is currently supported is %d", slowfilterrange);
			//Pixie_Print_MSG(ErrMSG);
		}
		
		// Update the DSP parameter SlowFilterRange
		Pixie_Devices[ModNum].DSP_Parameter_Values[SlowFilterRange_Address[ModNum] - DATA_MEMORY_ADDRESS] = slowfilterrange;
		
		// Download to the selected Pixie module
		//Pixie16IMbufferIO(&slowfilterrange, 1, SlowFilterRange_Address[ModNum], MOD_WRITE, ModNum);
		
		/*******************************************/
		//	We need to recompute FIFO settings
		/*******************************************/
		fastfilterrange = Pixie_Devices[ModNum].DSP_Parameter_Values[FastFilterRange_Address[ModNum] - DATA_MEMORY_ADDRESS];
		for(k=0; k<NUMBER_OF_CHANNELS; k++)
		{
			// Get the current TraceDelay
			paflength = Pixie_Devices[ModNum].DSP_Parameter_Values[PAFlength_Address[ModNum] + k - DATA_MEMORY_ADDRESS];
			triggerdelay = Pixie_Devices[ModNum].DSP_Parameter_Values[TriggerDelay_Address[ModNum] + k - DATA_MEMORY_ADDRESS];
			tracedelay = paflength - (unsigned int)((double)triggerdelay / pow(2.0, (double)fastfilterrange));
			
			// Update FIFO settings (TriggerDelay and PAFLength)
			myPixie_ComputeFIFO(tracedelay, ModNum, k);
		}
		
		// Program FiPPI to apply settings to the FPGA
		//retval = Pixie16ProgramFippi(ModNum);
		//if(retval < 0)
		//{
			//sprintf(ErrMSG, "*ERROR* (Pixie16WriteSglModPar): ProgramFippi failed after downloading SlowFilterRange, retval=%d", retval);
			//Pixie_Print_MSG(ErrMSG);
			//return(-3);
		//}
		
		/*******************************************/
		//	We also need to update baseline cut value
		/*******************************************/
		//for(k=0; k<NUMBER_OF_CHANNELS; k++)
		//{
			//retval = Pixie16BLcutFinder(ModNum, k, &baselinecut);
			//if(retval < 0)
			//{
				//sprintf(ErrMSG, "*ERROR* (Pixie16WriteSglModPar): BLcutFinder failed in module %d after downloading SlowFilterRange, retval=%d", k, retval);
				//Pixie_Print_MSG(ErrMSG);
				//return(-4);
			//}
		//}
	}
	else if(strcmp(ModParName,"FAST_FILTER_RANGE") == 0)
	{
		// Get the last fast filter range
		LastFastFilterRange = Pixie_Devices[ModNum].DSP_Parameter_Values[FastFilterRange_Address[ModNum] - DATA_MEMORY_ADDRESS];
		
		// Check fast filter range limit
		fastfilterrange = ModParData;

		if(fastfilterrange > FASTFILTERRANGE_MAX)
		{
			fastfilterrange = FASTFILTERRANGE_MAX;

			//sprintf(ErrMSG, "*WARNING* (Pixie16WriteSglModPar): Maximum FastFilterRange that is currently supported is %d", fastfilterrange);
			//Pixie_Print_MSG(ErrMSG);
		}
		if(fastfilterrange < FASTFILTERRANGE_MIN)
		{
			fastfilterrange = FASTFILTERRANGE_MIN;

			//sprintf(ErrMSG, "*WARNING* (Pixie16WriteSglModPar): Minimum FastFilterRange that is currently supported is %d", fastfilterrange);
			//Pixie_Print_MSG(ErrMSG);
		}
		
		// Update the new DSP parameter FastFilterRange
		Pixie_Devices[ModNum].DSP_Parameter_Values[FastFilterRange_Address[ModNum] - DATA_MEMORY_ADDRESS] = fastfilterrange;
		
		// Download to the selected Pixie module
		//Pixie16IMbufferIO(&fastfilterrange, 1, FastFilterRange_Address[ModNum], MOD_WRITE, ModNum);
		
		/*******************************************/
		//	We need to recompute FIFO settings
		/*******************************************/
		for(k=0; k<NUMBER_OF_CHANNELS; k++)
		{
			// Get the current TraceDelay
			paflength = Pixie_Devices[ModNum].DSP_Parameter_Values[PAFlength_Address[ModNum] + k - DATA_MEMORY_ADDRESS];
			triggerdelay = Pixie_Devices[ModNum].DSP_Parameter_Values[TriggerDelay_Address[ModNum] + k - DATA_MEMORY_ADDRESS];
			tracedelay = paflength - (unsigned int)((double)triggerdelay / pow(2.0, (double)LastFastFilterRange));
			
			// Update FIFO settings (TriggerDelay and PAFLength)
			myPixie_ComputeFIFO(tracedelay, ModNum, k);
		}
		
		// Program FiPPI to apply settings to the FPGA
		//retval = Pixie16ProgramFippi(ModNum);
		//if(retval < 0)
		//{
			//sprintf(ErrMSG, "*ERROR* (Pixie16WriteSglModPar): ProgramFippi failed after downloading FastFilterRange, retval=%d", retval);
			//Pixie_Print_MSG(ErrMSG);
			//return(-3);
		//}
	}
	else if(strcmp(ModParName,"FastTrigBackplaneEna") == 0)
	{		
		// Update the new DSP parameter FastTrigBackplaneEna
		Pixie_Devices[ModNum].DSP_Parameter_Values[FastTrigBackplaneEna_Address[ModNum] - DATA_MEMORY_ADDRESS] = ModParData;
		
		// Download to the selected Pixie module
		//Pixie16IMbufferIO(&ModParData, 1, FastTrigBackplaneEna_Address[ModNum], MOD_WRITE, ModNum);
		
		// Program FiPPI to apply settings to the FPGA
		//retval = Pixie16ProgramFippi(ModNum);
		//if(retval < 0)
		//{
			//sprintf(ErrMSG, "*ERROR* (Pixie16WriteSglModPar): ProgramFippi failed after downloading FastTrigBackplaneEna, retval=%d", retval);
			//Pixie_Print_MSG(ErrMSG);
			//return(-3);
		//}			
	}
	else if(strcmp(ModParName,"CrateID") == 0)
	{		
		// Update the new DSP parameter CrateID
		Pixie_Devices[ModNum].DSP_Parameter_Values[CrateID_Address[ModNum] - DATA_MEMORY_ADDRESS] = ModParData;
		
		// Download to the selected Pixie module
		//Pixie16IMbufferIO(&ModParData, 1, CrateID_Address[ModNum], MOD_WRITE, ModNum);	

		// Program FiPPI to apply settings to the FPGA
		//retval = Pixie16ProgramFippi(ModNum);
		//if(retval < 0)
		//{
			//sprintf(ErrMSG, "*ERROR* (Pixie16WriteSglModPar): ProgramFippi failed after downloading CrateID, retval=%d", retval);
			//Pixie_Print_MSG(ErrMSG);
			//return(-3);
		//}
	}	
	else if(strcmp(ModParName,"SlotID") == 0)
	{		
		// Update the new DSP parameter SlotID
		Pixie_Devices[ModNum].DSP_Parameter_Values[SlotID_Address[ModNum] - DATA_MEMORY_ADDRESS] = ModParData;
		
		// Download to the selected Pixie module
		//Pixie16IMbufferIO(&ModParData, 1, SlotID_Address[ModNum], MOD_WRITE, ModNum);	

		// Program FiPPI to apply settings to the FPGA
		//retval = Pixie16ProgramFippi(ModNum);
		//if(retval < 0)
		//{
			//sprintf(ErrMSG, "*ERROR* (Pixie16WriteSglModPar): ProgramFippi failed after downloading SlotID, retval=%d", retval);
			//Pixie_Print_MSG(ErrMSG);
			//return(-3);
		//}
	}	
	else if(strcmp(ModParName,"ModID") == 0)
	{		
		// Update the new DSP parameter ModID
		Pixie_Devices[ModNum].DSP_Parameter_Values[ModID_Address[ModNum] - DATA_MEMORY_ADDRESS] = ModParData;
		
		// Download to the selected Pixie module
		//Pixie16IMbufferIO(&ModParData, 1, ModID_Address[ModNum], MOD_WRITE, ModNum);	

		// Program FiPPI to apply settings to the FPGA
		//retval = Pixie16ProgramFippi(ModNum);
		//if(retval < 0)
		//{
			//sprintf(ErrMSG, "*ERROR* (Pixie16WriteSglModPar): ProgramFippi failed after downloading ModID, retval=%d", retval);
			//Pixie_Print_MSG(ErrMSG);
			//return(-3);
		//}
	}
	else if(strcmp(ModParName,"TrigConfig0") == 0)
	{		
		// Update the new DSP parameter TrigConfig0
		Pixie_Devices[ModNum].DSP_Parameter_Values[TrigConfig_Address[ModNum] - DATA_MEMORY_ADDRESS] = ModParData;
		
		// Download to the selected Pixie module
		//Pixie16IMbufferIO(&ModParData, 1, TrigConfig_Address[ModNum], MOD_WRITE, ModNum);
		
		// Program FiPPI to apply settings to the FPGA
		//retval = Pixie16ProgramFippi(ModNum);
		//if(retval < 0)
		//{
			//sprintf(ErrMSG, "*ERROR* (Pixie16WriteSglModPar): ProgramFippi failed after downloading TrigConfig0, retval=%d", retval);
			//Pixie_Print_MSG(ErrMSG);
			//return(-3);
		//}			
	}
	else if(strcmp(ModParName,"TrigConfig1") == 0)
	{		
		// Update the new DSP parameter TrigConfig1
		Pixie_Devices[ModNum].DSP_Parameter_Values[TrigConfig_Address[ModNum] + 1 - DATA_MEMORY_ADDRESS] = ModParData;
		
		// Download to the selected Pixie module
		//Pixie16IMbufferIO(&ModParData, 1, TrigConfig_Address[ModNum] + 1, MOD_WRITE, ModNum);
		
		// Program FiPPI to apply settings to the FPGA
		//retval = Pixie16ProgramFippi(ModNum);
		//if(retval < 0)
		//{
			//sprintf(ErrMSG, "*ERROR* (Pixie16WriteSglModPar): ProgramFippi failed after downloading TrigConfig1, retval=%d", retval);
			//Pixie_Print_MSG(ErrMSG);
			//return(-3);
		//}			
	}
	else if(strcmp(ModParName,"TrigConfig2") == 0)
	{		
		// Update the new DSP parameter TrigConfig2
		Pixie_Devices[ModNum].DSP_Parameter_Values[TrigConfig_Address[ModNum] + 2 - DATA_MEMORY_ADDRESS] = ModParData;
		
		// Download to the selected Pixie module
		//Pixie16IMbufferIO(&ModParData, 1, TrigConfig_Address[ModNum] + 2, MOD_WRITE, ModNum);
		
		// Program FiPPI to apply settings to the FPGA
		//retval = Pixie16ProgramFippi(ModNum);
		//if(retval < 0)
		//{
			//sprintf(ErrMSG, "*ERROR* (Pixie16WriteSglModPar): ProgramFippi failed after downloading TrigConfig2, retval=%d", retval);
			//Pixie_Print_MSG(ErrMSG);
			//return(-3);
		//}			
	}
	else if(strcmp(ModParName,"TrigConfig3") == 0)
	{		
		// Update the new DSP parameter TrigConfig3
		Pixie_Devices[ModNum].DSP_Parameter_Values[TrigConfig_Address[ModNum] + 3 - DATA_MEMORY_ADDRESS] = ModParData;
		
		// Download to the selected Pixie module
		//Pixie16IMbufferIO(&ModParData, 1, TrigConfig_Address[ModNum] + 3, MOD_WRITE, ModNum);
		
		// Program FiPPI to apply settings to the FPGA
		//retval = Pixie16ProgramFippi(ModNum);
		//if(retval < 0)
		//{
			//sprintf(ErrMSG, "*ERROR* (Pixie16WriteSglModPar): ProgramFippi failed after downloading TrigConfig3, retval=%d", retval);
			//Pixie_Print_MSG(ErrMSG);
			//return(-3);
		//}			
	}
	else if(strcmp(ModParName,"HOST_RT_PRESET") == 0)
	{		
		// Update the new DSP parameter HRTP
		Pixie_Devices[ModNum].DSP_Parameter_Values[HRTP_Address[ModNum] - DATA_MEMORY_ADDRESS] = ModParData;
		
		// Download to the selected Pixie module
		//Pixie16IMbufferIO(&ModParData, 1, HRTP_Address[ModNum], MOD_WRITE, ModNum);

		// When changing HostRunTimePreset in one Pixie module, we need to broadcast it to all other modules as well
		//Pixie_Broadcast("HOST_RT_PRESET", ModNum);
	}
	else
	{
		//sprintf(ErrMSG, "*ERROR* (Pixie16WriteSglModPar): invalid module parameter name %s", ModParName);
		//Pixie_Print_MSG(ErrMSG);
		//return(-2);
		return -E_PIXIE_UNKNOW_REG;
	}
	
	return(0);
}

int pixie16_ctl::myPixie16ReadSglModPar(const char* ModParName, uint32_t*
		ModParData, int ModNum)
{
	// Read module parameter
	if(strcmp(ModParName,"MODULE_NUMBER") == 0)
	{
		// Read from the selected Pixie module
		//Pixie16IMbufferIO(ModParData, 1, ModNum_Address[ModNum], MOD_READ, ModNum);
		
		// Update module parameter ModNum
		*ModParData = Pixie_Devices[ModNum].DSP_Parameter_Values[ModNum_Address[ModNum] - DATA_MEMORY_ADDRESS];
	}
	else if(strcmp(ModParName,"MODULE_CSRA") == 0)
	{
		// Read from the selected Pixie module
		//Pixie16IMbufferIO(ModParData, 1, ModCSRA_Address[ModNum], MOD_READ, ModNum);
		
		// Update module parameter ModCSRA
		*ModParData = Pixie_Devices[ModNum].DSP_Parameter_Values[ModCSRA_Address[ModNum] - DATA_MEMORY_ADDRESS];
	}
	else if(strcmp(ModParName,"MODULE_CSRB") == 0)
	{
		// Read from the selected Pixie module
		//Pixie16IMbufferIO(ModParData, 1, ModCSRB_Address[ModNum], MOD_READ, ModNum);
		
		// Update module parameter ModCSRB
		*ModParData = Pixie_Devices[ModNum].DSP_Parameter_Values[ModCSRB_Address[ModNum] - DATA_MEMORY_ADDRESS];
	}
	else if(strcmp(ModParName,"MODULE_FORMAT") == 0)
	{
		// Read from the selected Pixie module
		//Pixie16IMbufferIO(ModParData, 1, ModFormat_Address[ModNum], MOD_READ, ModNum);
		
		// Update module parameter ModFormat
		*ModParData = Pixie_Devices[ModNum].DSP_Parameter_Values[ModFormat_Address[ModNum] - DATA_MEMORY_ADDRESS];
	}
	else if(strcmp(ModParName,"MAX_EVENTS") == 0)
	{
		// Read from the selected Pixie module
		//Pixie16IMbufferIO(ModParData, 1, MaxEvents_Address[ModNum], MOD_READ, ModNum);
		
		// Update module parameter MaxEvents
		*ModParData = Pixie_Devices[ModNum].DSP_Parameter_Values[MaxEvents_Address[ModNum] - DATA_MEMORY_ADDRESS];
	}
	else if(strcmp(ModParName,"SYNCH_WAIT") == 0)
	{
		// Read from the selected Pixie module
		//Pixie16IMbufferIO(ModParData, 1, SynchWait_Address[ModNum], MOD_READ, ModNum);
		
		// Update module parameter SynchWait
		*ModParData = Pixie_Devices[ModNum].DSP_Parameter_Values[SynchWait_Address[ModNum] - DATA_MEMORY_ADDRESS];
	}
	else if(strcmp(ModParName,"IN_SYNCH") == 0)
	{
		// Read from the selected Pixie module
		//Pixie16IMbufferIO(ModParData, 1, InSynch_Address[ModNum], MOD_READ, ModNum);
		
		// Update module parameter InSynch
		*ModParData = Pixie_Devices[ModNum].DSP_Parameter_Values[InSynch_Address[ModNum] - DATA_MEMORY_ADDRESS];
	}
	else if(strcmp(ModParName,"SLOW_FILTER_RANGE") == 0)
	{
		// Read from the selected Pixie module
		//Pixie16IMbufferIO(ModParData, 1, SlowFilterRange_Address[ModNum], MOD_READ, ModNum);
		
		// Update module parameter SlowFilterRange
		*ModParData = Pixie_Devices[ModNum].DSP_Parameter_Values[SlowFilterRange_Address[ModNum] - DATA_MEMORY_ADDRESS];
	}
	else if(strcmp(ModParName,"FAST_FILTER_RANGE") == 0)
	{
		// Read from the selected Pixie module
		//Pixie16IMbufferIO(ModParData, 1, FastFilterRange_Address[ModNum], MOD_READ, ModNum);
		
		// Update module parameter FastFilterRange
		*ModParData = Pixie_Devices[ModNum].DSP_Parameter_Values[FastFilterRange_Address[ModNum] - DATA_MEMORY_ADDRESS];
	}
	else if(strcmp(ModParName,"FastTrigBackplaneEna") == 0)
	{
		// Read from the selected Pixie module
		//Pixie16IMbufferIO(ModParData, 1, FastTrigBackplaneEna_Address[ModNum], MOD_READ, ModNum);
		
		// Update module parameter FastTrigBackplaneEna
		*ModParData = Pixie_Devices[ModNum].DSP_Parameter_Values[FastTrigBackplaneEna_Address[ModNum] - DATA_MEMORY_ADDRESS];
	}	
	else if(strcmp(ModParName,"CrateID") == 0)
	{
		// Read from the selected Pixie module
		//Pixie16IMbufferIO(ModParData, 1, CrateID_Address[ModNum], MOD_READ, ModNum);
		
		// Update module parameter CrateID
		*ModParData = Pixie_Devices[ModNum].DSP_Parameter_Values[CrateID_Address[ModNum] - DATA_MEMORY_ADDRESS];
	}	
	else if(strcmp(ModParName,"SlotID") == 0)
	{
		// Read from the selected Pixie module
		//Pixie16IMbufferIO(ModParData, 1, SlotID_Address[ModNum], MOD_READ, ModNum);
		
		// Update module parameter SlotID
		*ModParData = Pixie_Devices[ModNum].DSP_Parameter_Values[SlotID_Address[ModNum] - DATA_MEMORY_ADDRESS];
	}	
	else if(strcmp(ModParName,"ModID") == 0)
	{
		// Read from the selected Pixie module
		//Pixie16IMbufferIO(ModParData, 1, ModID_Address[ModNum], MOD_READ, ModNum);
		
		// Update module parameter ModID
		*ModParData = Pixie_Devices[ModNum].DSP_Parameter_Values[ModID_Address[ModNum] - DATA_MEMORY_ADDRESS];
	}
	else if(strcmp(ModParName,"TrigConfig0") == 0)
	{
		// Read from the selected Pixie module
		//Pixie16IMbufferIO(ModParData, 1, TrigConfig_Address[ModNum], MOD_READ, ModNum);
		
		// Update module parameter TrigConfig0
		*ModParData = Pixie_Devices[ModNum].DSP_Parameter_Values[TrigConfig_Address[ModNum] - DATA_MEMORY_ADDRESS];
	}	
	else if(strcmp(ModParName,"TrigConfig1") == 0)
	{
		// Read from the selected Pixie module
		//Pixie16IMbufferIO(ModParData, 1, TrigConfig_Address[ModNum] + 1, MOD_READ, ModNum);
		
		// Update module parameter TrigConfig1
		*ModParData = Pixie_Devices[ModNum].DSP_Parameter_Values[TrigConfig_Address[ModNum] + 1 - DATA_MEMORY_ADDRESS];
	}
	else if(strcmp(ModParName,"TrigConfig2") == 0)
	{
		// Read from the selected Pixie module
		//Pixie16IMbufferIO(ModParData, 1, TrigConfig_Address[ModNum] + 2, MOD_READ, ModNum);
		
		// Update module parameter TrigConfig2
		*ModParData = Pixie_Devices[ModNum].DSP_Parameter_Values[TrigConfig_Address[ModNum] + 2 - DATA_MEMORY_ADDRESS];
	}
	else if(strcmp(ModParName,"TrigConfig3") == 0)
	{
		// Read from the selected Pixie module
		//Pixie16IMbufferIO(ModParData, 1, TrigConfig_Address[ModNum] + 3, MOD_READ, ModNum);
		
		// Update module parameter TrigConfig3
		*ModParData = Pixie_Devices[ModNum].DSP_Parameter_Values[TrigConfig_Address[ModNum] + 3 - DATA_MEMORY_ADDRESS];
	}
	else if(strcmp(ModParName,"HOST_RT_PRESET") == 0)
	{
		// Read from the selected Pixie module
		//Pixie16IMbufferIO(ModParData, 1, HRTP_Address[ModNum], MOD_READ, ModNum);
		
		// Update module parameter HostRTPreset
		*ModParData = Pixie_Devices[ModNum].DSP_Parameter_Values[HRTP_Address[ModNum] - DATA_MEMORY_ADDRESS];
	}
	else
	{
		//sprintf(ErrMSG, "*ERROR* (Pixie16ReadSglModPar): invalid module parameter name %s", ModParName);
		//Pixie_Print_MSG(ErrMSG);
		//return(-2);
		return -E_PIXIE_UNKNOW_REG;
	}
	return(0);
	
}

int pixie16_ctl::myPixie_Init_DSPVarAddress(const char* DSPVarFile, int
		ModNum)
{
	char DSP_Parameter_Names[N_DSP_PAR][MAX_PAR_NAME_LENGTH];
	char *DSP_Parameter_AddrStr;
	unsigned int DSP_Parameter_Addr[N_DSP_PAR];
	char DSPParaName[MAX_PAR_NAME_LENGTH], str[MAX_PAR_NAME_LENGTH], ErrMSG[MAX_ERRMSG_LENGTH];
	unsigned short k;
	FILE* namesFile = NULL;
	
	// Open DSP .var file
	namesFile = fopen(DSPVarFile, "r");
	if(namesFile != NULL)
	{
		// Clear all names
		for(k=0; k<N_DSP_PAR; k++)
		{
			strcpy(DSP_Parameter_Names[k],"");
		}
		
		// DSP_Parameter_AddrStr is 11 characters long: 0x + 8 hex characters + NULL terminator
		DSP_Parameter_AddrStr = (char *)malloc(sizeof(char) * 10 + 1);
		
		// Read names
		for(k=0; k<N_DSP_PAR; k++)
		{
			if( fgets(DSPParaName, MAX_PAR_NAME_LENGTH, namesFile) != NULL )  // Read one line
			{
				sscanf(DSPParaName, "%s %s", DSP_Parameter_AddrStr, DSP_Parameter_Names[k]);
				DSP_Parameter_Addr[k] = strtol(DSP_Parameter_AddrStr, NULL, 0);
			}
			else  // Reached end of file
			{
				break;
			}
		}
		
		// Release memory
		free(DSP_Parameter_AddrStr);
		
		// Initialize addressee
		
		//--------------------
		//	Module parameters
		//--------------------
		
		ModNum_Address[ModNum] = 0;
		ModCSRA_Address[ModNum] = 0;
		ModCSRB_Address[ModNum] = 0;
		ModFormat_Address[ModNum] = 0;
		RunTask_Address[ModNum] = 0;
		ControlTask_Address[ModNum] = 0;
		MaxEvents_Address[ModNum] = 0;
		CoincPattern_Address[ModNum] = 0;
		CoincWait_Address[ModNum] = 0;
		SynchWait_Address[ModNum] = 0;
		InSynch_Address[ModNum] = 0;
		Resume_Address[ModNum] = 0;
		SlowFilterRange_Address[ModNum] = 0;
		FastFilterRange_Address[ModNum] = 0;
		ChanNum_Address[ModNum] = 0;
		HostIO_Address[ModNum] = 0;
		UserIn_Address[ModNum] = 0;
		U00_Address[ModNum] = 0;
		FastTrigBackplaneEna_Address[ModNum] = 0;
		CrateID_Address             [ModNum] = 0;
		SlotID_Address              [ModNum] = 0;
		ModID_Address               [ModNum] = 0;
		TrigConfig_Address          [ModNum] = 0;
		HRTP_Address[ModNum] = 0;
		
		//--------------------
		//	Channel parameters
		//--------------------
		
		ChanCSRa_Address[ModNum] = 0;
		ChanCSRb_Address[ModNum] = 0;
		GainDAC_Address[ModNum] = 0;
		OffsetDAC_Address[ModNum] = 0;
		DigGain_Address[ModNum] = 0;
		SlowLength_Address[ModNum] = 0;
		SlowGap_Address[ModNum] = 0;
		FastLength_Address[ModNum] = 0;
		FastGap_Address[ModNum] = 0;
		PeakSample_Address[ModNum] = 0;
		PeakSep_Address[ModNum] = 0;
		CFDThresh_Address[ModNum] = 0;
		FastThresh_Address[ModNum] = 0;
		ThreshWidth_Address[ModNum] = 0;
		PAFlength_Address[ModNum] = 0;
		TriggerDelay_Address[ModNum] = 0;
		ResetDelay_Address[ModNum] = 0;
		ChanTrigStretch_Address[ModNum] = 0;
		TraceLength_Address[ModNum] = 0;
		TrigOutLen_Address[ModNum] = 0;
		EnergyLow_Address[ModNum] = 0;
		Log2Ebin_Address[ModNum] = 0;
		Log2Bweight_Address[ModNum] = 0;
		MultiplicityMaskL_Address[ModNum] = 0;
		PSAoffset_Address[ModNum] = 0;
		PSAlength_Address[ModNum] = 0;
		Integrator_Address[ModNum] = 0;
		BLcut_Address[ModNum] = 0;
		BaselinePercent_Address[ModNum] = 0;
		FtrigoutDelay_Address[ModNum] = 0;
		Log2Bweight_Address[ModNum] = 0;
		PreampTau_Address[ModNum] = 0;
		MultiplicityMaskH_Address[ModNum] = 0;
		FastTrigBackLen_Address[ModNum] = 0;
		Xwait_Address[ModNum] = 0;
		CFDDelay_Address            [ModNum] = 0;
		CFDScale_Address            [ModNum] = 0;
		ExternDelayLen_Address      [ModNum] = 0;
		ExtTrigStretch_Address      [ModNum] = 0;
		VetoStretch_Address         [ModNum] = 0;
		QDCLen0_Address             [ModNum] = 0;
		QDCLen1_Address             [ModNum] = 0;
		QDCLen2_Address             [ModNum] = 0;
		QDCLen3_Address             [ModNum] = 0;
		QDCLen4_Address             [ModNum] = 0;
		QDCLen5_Address             [ModNum] = 0;
		QDCLen6_Address             [ModNum] = 0;
		QDCLen7_Address             [ModNum] = 0;

		//--------------------
		//	Results parameters
		//--------------------
		
		RealTimeA_Address[ModNum] = 0;
		RealTimeB_Address[ModNum] = 0;
		RunTimeA_Address[ModNum] = 0;
		RunTimeB_Address[ModNum] = 0;
		GSLTtime_Address[ModNum] = 0;
		NumEventsA_Address[ModNum] = 0;
		NumEventsB_Address[ModNum] = 0;
		DSPerror_Address[ModNum] = 0;
		SynchDone_Address[ModNum] = 0;
		BufHeadLen_Address[ModNum] = 0;
		EventHeadLen_Address[ModNum] = 0;
		ChanHeadLen_Address[ModNum] = 0;
		UserOut_Address[ModNum] = 0;
		AOutBuffer_Address[ModNum] = 0;
		LOutBuffer_Address[ModNum] = 0;
		AECorr_Address[ModNum] = 0;
		LECorr_Address[ModNum] = 0;
		HardwareID_Address[ModNum] = 0;
		HardVariant_Address[ModNum] = 0;
		FIFOLength_Address[ModNum] = 0;
		FippiID_Address[ModNum] = 0;
		FippiVariant_Address[ModNum] = 0;
		DSPrelease_Address[ModNum] = 0;
		DSPbuild_Address[ModNum] = 0;
		DSPVariant_Address[ModNum] = 0;
		U20_Address[ModNum] = 0;
		LiveTimeA_Address[ModNum] = 0;
		LiveTimeB_Address[ModNum] = 0;
		FastPeaksA_Address[ModNum] = 0;
		FastPeaksB_Address[ModNum] = 0;
		OverflowA_Address[ModNum] = 0;
		OverflowB_Address[ModNum] = 0;
		InSpecA_Address[ModNum] = 0;
		InSpecB_Address[ModNum] = 0;
		UnderflowA_Address[ModNum] = 0;
		UnderflowB_Address[ModNum] = 0;
		ChanEventsA_Address[ModNum] = 0;
		ChanEventsB_Address[ModNum] = 0;
		AutoTau_Address[ModNum] = 0;
		U30_Address[ModNum] = 0;
		
		// Find Address
		for(k=0; k<N_DSP_PAR; k++)
		{
			strcpy(str, DSP_Parameter_Names[k]);

			//--------------------
			//	Module parameters
			//--------------------
			
			if(strcmp(str, "ModNum") == 0)
				ModNum_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "ModCSRA") == 0)
				ModCSRA_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "ModCSRB") == 0)
				ModCSRB_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "ModFormat") == 0)
				ModFormat_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "RunTask") == 0)
				RunTask_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "ControlTask") == 0)
				ControlTask_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "MaxEvents") == 0)
				MaxEvents_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "CoincPattern") == 0)
				CoincPattern_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "CoincWait") == 0)
				CoincWait_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "SynchWait") == 0)
				SynchWait_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "InSynch") == 0)
				InSynch_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "Resume") == 0)
				Resume_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "SlowFilterRange") == 0)
				SlowFilterRange_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "FastFilterRange") == 0)
				FastFilterRange_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "ChanNum") == 0)
				ChanNum_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "HostIO") == 0)
				HostIO_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "UserIn") == 0)
				UserIn_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "U00") == 0)
				U00_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "FastTrigBackplaneEna") == 0)
				FastTrigBackplaneEna_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "CrateID") == 0)
				CrateID_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "SlotID") == 0)
				SlotID_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "ModID") == 0)
				ModID_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "TrigConfig") == 0)
				TrigConfig_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "HostRunTimePreset") == 0)
				HRTP_Address[ModNum] = DSP_Parameter_Addr[k];

			//--------------------
			//	Channel parameters
			//--------------------
			
			else if(strcmp(str, "ChanCSRa") == 0)
				ChanCSRa_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "ChanCSRb") == 0)
				ChanCSRb_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "GainDAC") == 0)
				GainDAC_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "OffsetDAC") == 0)
				OffsetDAC_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "DigGain") == 0)
				DigGain_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "SlowLength") == 0)
				SlowLength_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "SlowGap") == 0)
				SlowGap_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "FastLength") == 0)
				FastLength_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "FastGap") == 0)
				FastGap_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "PeakSample") == 0)
				PeakSample_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "PeakSep") == 0)
				PeakSep_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "CFDThresh") == 0)
				CFDThresh_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "FastThresh") == 0)
				FastThresh_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "ThreshWidth") == 0)
				ThreshWidth_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "PAFlength") == 0)
				PAFlength_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "TriggerDelay") == 0)
				TriggerDelay_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "ResetDelay") == 0)
				ResetDelay_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "ChanTrigStretch") == 0)
				ChanTrigStretch_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "TraceLength") == 0)
				TraceLength_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "TrigOutLen") == 0)
				TrigOutLen_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "EnergyLow") == 0)
				EnergyLow_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "Log2Ebin") == 0)
				Log2Ebin_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "Log2Bweight") == 0)
				Log2Bweight_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "MultiplicityMaskL") == 0)
				MultiplicityMaskL_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "PSAoffset") == 0)
				PSAoffset_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "PSAlength") == 0)
				PSAlength_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "Integrator") == 0)
				Integrator_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "BLcut") == 0)
				BLcut_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "BaselinePercent") == 0)
				BaselinePercent_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "FtrigoutDelay") == 0)
				FtrigoutDelay_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "Log2Bweight") == 0)
				Log2Bweight_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "PreampTau") == 0)
				PreampTau_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "MultiplicityMaskH") == 0)
				MultiplicityMaskH_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "FastTrigBackLen") == 0)
				FastTrigBackLen_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "Xwait") == 0)
				Xwait_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "CFDDelay") == 0)
				CFDDelay_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "CFDScale") == 0)
				CFDScale_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "ExternDelayLen") == 0)
				ExternDelayLen_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "ExtTrigStretch") == 0)
				ExtTrigStretch_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "VetoStretch") == 0)
				VetoStretch_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "QDCLen0") == 0)
				QDCLen0_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "QDCLen1") == 0)
				QDCLen1_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "QDCLen2") == 0)
				QDCLen2_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "QDCLen3") == 0)
				QDCLen3_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "QDCLen4") == 0)
				QDCLen4_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "QDCLen5") == 0)
				QDCLen5_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "QDCLen6") == 0)
				QDCLen6_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "QDCLen7") == 0)
				QDCLen7_Address[ModNum] = DSP_Parameter_Addr[k];
			
			//--------------------
			//	Results parameters
			//--------------------
			
			else if(strcmp(str, "RealTimeA") == 0)
				RealTimeA_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "RealTimeB") == 0)
				RealTimeB_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "RunTimeA") == 0)
				RunTimeA_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "RunTimeB") == 0)
				RunTimeB_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "GSLTtime") == 0)
				GSLTtime_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "NumEventsA") == 0)
				NumEventsA_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "NumEventsB") == 0)
				NumEventsB_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "DSPerror") == 0)
				DSPerror_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "SynchDone") == 0)
				SynchDone_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "BufHeadLen") == 0)
				BufHeadLen_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "EventHeadLen") == 0)
				EventHeadLen_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "ChanHeadLen") == 0)
				ChanHeadLen_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "UserOut") == 0)
				UserOut_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "AOutBuffer") == 0)
				AOutBuffer_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "LOutBuffer") == 0)
				LOutBuffer_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "AECorr") == 0)
				AECorr_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "LECorr") == 0)
				LECorr_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "HardwareID") == 0)
				HardwareID_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "HardVariant") == 0)
				HardVariant_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "FIFOLength") == 0)
				FIFOLength_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "FippiID") == 0)
				FippiID_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "FippiVariant") == 0)
				FippiVariant_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "DSPrelease") == 0)
				DSPrelease_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "DSPbuild") == 0)
				DSPbuild_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "DSPVariant") == 0)
				DSPVariant_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "U20") == 0)
				U20_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "LiveTimeA") == 0)
				LiveTimeA_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "LiveTimeB") == 0)
				LiveTimeB_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "FastPeaksA") == 0)
				FastPeaksA_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "FastPeaksB") == 0)
				FastPeaksB_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "OverflowA") == 0)
				OverflowA_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "OverflowB") == 0)
				OverflowB_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "InSpecA") == 0)
				InSpecA_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "InSpecB") == 0)
				InSpecB_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "UnderflowA") == 0)
				UnderflowA_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "UnderflowB") == 0)
				UnderflowB_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "ChanEventsA") == 0)
				ChanEventsA_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "ChanEventsB") == 0)
				ChanEventsB_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "AutoTau") == 0)
				AutoTau_Address[ModNum] = DSP_Parameter_Addr[k];
			else if(strcmp(str, "U30") == 0)
				U30_Address[ModNum] = DSP_Parameter_Addr[k];
		}
		
		// Check if all the DSP variable names have been found in the file
		
		//--------------------
		//	Module parameters
		//--------------------
		
	}
	else
	{
		return -E_DSPVarFile;
	}

	return(0);
}

int pixie16_ctl::myPixie_Copy_DSPVarAddress(int SourceModNum, int
		DestinationModNum)
{
	// Check if SourceModNum is valid
	
	// Copy Address
	
	//--------------------
	//	Module parameters
	//--------------------
	ModNum_Address[DestinationModNum] = ModNum_Address[SourceModNum];
	ModCSRA_Address[DestinationModNum] = ModCSRA_Address[SourceModNum];
	ModCSRB_Address[DestinationModNum] = ModCSRB_Address[SourceModNum];
	ModFormat_Address[DestinationModNum] = ModFormat_Address[SourceModNum];
	RunTask_Address[DestinationModNum] = RunTask_Address[SourceModNum];
	ControlTask_Address[DestinationModNum] = ControlTask_Address[SourceModNum];
	MaxEvents_Address[DestinationModNum] = MaxEvents_Address[SourceModNum];
	CoincPattern_Address[DestinationModNum] = CoincPattern_Address[SourceModNum];
	CoincWait_Address[DestinationModNum] = CoincWait_Address[SourceModNum];
	SynchWait_Address[DestinationModNum] = SynchWait_Address[SourceModNum];
	InSynch_Address[DestinationModNum] = InSynch_Address[SourceModNum];
	Resume_Address[DestinationModNum] = Resume_Address[SourceModNum];
	SlowFilterRange_Address[DestinationModNum] = SlowFilterRange_Address[SourceModNum];
	FastFilterRange_Address[DestinationModNum] = FastFilterRange_Address[SourceModNum];
	ChanNum_Address[DestinationModNum] = ChanNum_Address[SourceModNum];
	HostIO_Address[DestinationModNum] = HostIO_Address[SourceModNum];
	UserIn_Address[DestinationModNum] = UserIn_Address[SourceModNum];
	U00_Address[DestinationModNum] = U00_Address[SourceModNum];
	FastTrigBackplaneEna_Address[DestinationModNum] = FastTrigBackplaneEna_Address[SourceModNum];
	CrateID_Address             [DestinationModNum] = CrateID_Address             [SourceModNum];
	SlotID_Address              [DestinationModNum] = SlotID_Address              [SourceModNum];
	ModID_Address               [DestinationModNum] = ModID_Address               [SourceModNum];
	TrigConfig_Address          [DestinationModNum] = TrigConfig_Address          [SourceModNum];
	HRTP_Address[DestinationModNum] = HRTP_Address[SourceModNum];

	//--------------------
	//	Channel parameters
	//--------------------
	
	ChanCSRa_Address[DestinationModNum] = ChanCSRa_Address[SourceModNum];
	ChanCSRb_Address[DestinationModNum] = ChanCSRb_Address[SourceModNum];
	GainDAC_Address[DestinationModNum] = GainDAC_Address[SourceModNum];
	OffsetDAC_Address[DestinationModNum] = OffsetDAC_Address[SourceModNum];
	DigGain_Address[DestinationModNum] = DigGain_Address[SourceModNum];
	SlowLength_Address[DestinationModNum] = SlowLength_Address[SourceModNum];
	SlowGap_Address[DestinationModNum] = SlowGap_Address[SourceModNum];
	FastLength_Address[DestinationModNum] = FastLength_Address[SourceModNum];
	FastGap_Address[DestinationModNum] = FastGap_Address[SourceModNum];
	PeakSample_Address[DestinationModNum] = PeakSample_Address[SourceModNum];
	PeakSep_Address[DestinationModNum] = PeakSep_Address[SourceModNum];
	CFDThresh_Address[DestinationModNum] = CFDThresh_Address[SourceModNum];
	FastThresh_Address[DestinationModNum] = FastThresh_Address[SourceModNum];
	ThreshWidth_Address[DestinationModNum] = ThreshWidth_Address[SourceModNum];
	PAFlength_Address[DestinationModNum] = PAFlength_Address[SourceModNum];
	TriggerDelay_Address[DestinationModNum] = TriggerDelay_Address[SourceModNum];
	ResetDelay_Address[DestinationModNum] = ResetDelay_Address[SourceModNum];
	ChanTrigStretch_Address[DestinationModNum] = ChanTrigStretch_Address[SourceModNum];
	TraceLength_Address[DestinationModNum] = TraceLength_Address[SourceModNum];
	TrigOutLen_Address[DestinationModNum] = TrigOutLen_Address[SourceModNum];
	EnergyLow_Address[DestinationModNum] = EnergyLow_Address[SourceModNum];
	Log2Ebin_Address[DestinationModNum] = Log2Ebin_Address[SourceModNum];
	Log2Bweight_Address[DestinationModNum] = Log2Bweight_Address[SourceModNum];

	MultiplicityMaskL_Address[DestinationModNum] = MultiplicityMaskL_Address[SourceModNum];

	PSAoffset_Address[DestinationModNum] = PSAoffset_Address[SourceModNum];
	PSAlength_Address[DestinationModNum] = PSAlength_Address[SourceModNum];
	Integrator_Address[DestinationModNum] = Integrator_Address[SourceModNum];

	BLcut_Address[DestinationModNum] = BLcut_Address[SourceModNum];
	BaselinePercent_Address[DestinationModNum] = BaselinePercent_Address[SourceModNum];

	FtrigoutDelay_Address[DestinationModNum] = FtrigoutDelay_Address[SourceModNum];

	Log2Bweight_Address[DestinationModNum] = Log2Bweight_Address[SourceModNum];
	PreampTau_Address[DestinationModNum] = PreampTau_Address[SourceModNum];

	MultiplicityMaskH_Address[DestinationModNum] = MultiplicityMaskH_Address[SourceModNum];

	FastTrigBackLen_Address[DestinationModNum] = FastTrigBackLen_Address[SourceModNum];
	
	Xwait_Address[DestinationModNum] = Xwait_Address[SourceModNum];

	CFDDelay_Address            [DestinationModNum] = CFDDelay_Address            [SourceModNum];
	CFDScale_Address            [DestinationModNum] = CFDScale_Address            [SourceModNum];
	ExternDelayLen_Address      [DestinationModNum] = ExternDelayLen_Address      [SourceModNum];
	ExtTrigStretch_Address      [DestinationModNum] = ExtTrigStretch_Address      [SourceModNum];
	VetoStretch_Address         [DestinationModNum] = VetoStretch_Address         [SourceModNum];
	QDCLen0_Address             [DestinationModNum] = QDCLen0_Address             [SourceModNum];
	QDCLen1_Address             [DestinationModNum] = QDCLen1_Address             [SourceModNum];
	QDCLen2_Address             [DestinationModNum] = QDCLen2_Address             [SourceModNum];
	QDCLen3_Address             [DestinationModNum] = QDCLen3_Address             [SourceModNum];
	QDCLen4_Address             [DestinationModNum] = QDCLen4_Address             [SourceModNum];
	QDCLen5_Address             [DestinationModNum] = QDCLen5_Address             [SourceModNum];
	QDCLen6_Address             [DestinationModNum] = QDCLen6_Address             [SourceModNum];
	QDCLen7_Address             [DestinationModNum] = QDCLen7_Address             [SourceModNum];
	
	//--------------------
	//	Results parameters
	//--------------------
	
	RealTimeA_Address[DestinationModNum] = RealTimeA_Address[SourceModNum];
	RealTimeB_Address[DestinationModNum] = RealTimeB_Address[SourceModNum];
	RunTimeA_Address[DestinationModNum] = RunTimeA_Address[SourceModNum];
	RunTimeB_Address[DestinationModNum] = RunTimeB_Address[SourceModNum];
	GSLTtime_Address[DestinationModNum] = GSLTtime_Address[SourceModNum];
	NumEventsA_Address[DestinationModNum] = NumEventsA_Address[SourceModNum];
	NumEventsB_Address[DestinationModNum] = NumEventsB_Address[SourceModNum];
	DSPerror_Address[DestinationModNum] = DSPerror_Address[SourceModNum];
	SynchDone_Address[DestinationModNum] = SynchDone_Address[SourceModNum];
	BufHeadLen_Address[DestinationModNum] = BufHeadLen_Address[SourceModNum];
	EventHeadLen_Address[DestinationModNum] = EventHeadLen_Address[SourceModNum];
	ChanHeadLen_Address[DestinationModNum] = ChanHeadLen_Address[SourceModNum];
	UserOut_Address[DestinationModNum] = UserOut_Address[SourceModNum];
	AOutBuffer_Address[DestinationModNum] = AOutBuffer_Address[SourceModNum];
	LOutBuffer_Address[DestinationModNum] = LOutBuffer_Address[SourceModNum];
	AECorr_Address[DestinationModNum] = AECorr_Address[SourceModNum];
	LECorr_Address[DestinationModNum] = LECorr_Address[SourceModNum];
	HardwareID_Address[DestinationModNum] = HardwareID_Address[SourceModNum];
	HardVariant_Address[DestinationModNum] = HardVariant_Address[SourceModNum];
	FIFOLength_Address[DestinationModNum] = FIFOLength_Address[SourceModNum];
	FippiID_Address[DestinationModNum] = FippiID_Address[SourceModNum];
	FippiVariant_Address[DestinationModNum] = FippiVariant_Address[SourceModNum];
	DSPrelease_Address[DestinationModNum] = DSPrelease_Address[SourceModNum];
	DSPbuild_Address[DestinationModNum] = DSPbuild_Address[SourceModNum];
	DSPVariant_Address[DestinationModNum] = DSPVariant_Address[SourceModNum];
	U20_Address[DestinationModNum] = U20_Address[SourceModNum];
	LiveTimeA_Address[DestinationModNum] = LiveTimeA_Address[SourceModNum];
	LiveTimeB_Address[DestinationModNum] = LiveTimeB_Address[SourceModNum];
	FastPeaksA_Address[DestinationModNum] = FastPeaksA_Address[SourceModNum];
	FastPeaksB_Address[DestinationModNum] = FastPeaksB_Address[SourceModNum];
	OverflowA_Address[DestinationModNum] = OverflowA_Address[SourceModNum];
	OverflowB_Address[DestinationModNum] = OverflowB_Address[SourceModNum];
	InSpecA_Address[DestinationModNum] = InSpecA_Address[SourceModNum];
	InSpecB_Address[DestinationModNum] = InSpecB_Address[SourceModNum];
	UnderflowA_Address[DestinationModNum] = UnderflowA_Address[SourceModNum];
	UnderflowB_Address[DestinationModNum] = UnderflowB_Address[SourceModNum];
	ChanEventsA_Address[DestinationModNum] = ChanEventsA_Address[SourceModNum];
	ChanEventsB_Address[DestinationModNum] = ChanEventsB_Address[SourceModNum];
	AutoTau_Address[DestinationModNum] = AutoTau_Address[SourceModNum];
	U30_Address[DestinationModNum] = U30_Address[SourceModNum];
	
	return(0);
}
