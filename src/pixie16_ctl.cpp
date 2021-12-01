#include "pixie16_ctl.h"
//#include "CAENVMElib.h"
#include "err_code.h"
#include <iostream>
#include <string.h>


/* open the device 
 * @par a pointer of parameters 
 * @return 0 if succeed, non-zero error codes are returned if error */
int pixie16_ctl::open(void* par)
{
	struct pixie16_ctl_open_par* tmp = (struct pixie16_ctl_open_par*)par;
	int ret;
	
	mod_num = tmp->mod_num;
	if (mod_num > MAX_CRATE)
		return -E_MAX_CRATE;
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

int pixie16_ctl::blt_read(unsigned long addr, void* buf, int sz_in, int* sz_out)
{
	int ret;
	unsigned short mod_n = am;
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
		if (Pixie16WriteSglChanPar("XDT", valf64, mod_n, ch_n))
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
		if (Pixie16WriteSglChanPar("BASELINE_PERCENT", valf64, mod_n, ch_n))
			return -E_PIXIE_GENERAL;
		if (Pixie16AdjustOffsets(mod_n))
			return -E_PIXIE_GENERAL;
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
		if (Pixie16WriteSglChanPar("TAU", valf64, mod_n, ch_n))
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
		if (Pixie16WriteSglChanPar("CFDScale", valf64, mod_n, ch_n))
			return -E_PIXIE_GENERAL;
		break;
	case 24:
		/* cfd delay used in the cfd algrithm */
		if (Pixie16WriteSglChanPar("CFDDelay", valf64, mod_n, ch_n))
			return -E_PIXIE_GENERAL;
		break;
	case 25:
		/* cfd threshold in the cfd algrithm */
		valf64 *= 1000;
		if (Pixie16WriteSglChanPar("CFDThresh", valf64, mod_n, ch_n))
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
		if (Pixie16WriteSglChanPar(buf, valf64, mod_n, ch_n))
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
		if (Pixie16ReadSglChanPar("MultiplicityMaskH", &valf64, mod_n, ch_n))
			return -E_PIXIE_GENERAL;
		if (val64 >= 2) {
			valf64 = set_bits_double(valf64, 31, 1, 1);
			uint32_t data1;
			uint16_t ChNum=ch_n;
			ChNum /= 4;
			if (Pixie16ReadSglModPar("TrigConfig2", &data1, mod_n))
				return -E_PIXIE_GENERAL;
			if (val64 == 2)
				data1 = set_bits_int(data1, 24+ChNum, 1, 1);
			else
				data1 = set_bits_int(data1, 24+ChNum, 1, 0);
			if (Pixie16WriteSglModPar("TrigConfig2", data1, mod_n))
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
		if (Pixie16WriteSglChanPar("MultiplicityMaskH", valf64, mod_n, ch_n))
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
		if (Pixie16WriteSglChanPar("FtrigoutDelay", valf64, mod_n, ch_n))
			return -E_PIXIE_GENERAL;
		break;
	case 67:
		/* LCFT Width  */
		if (Pixie16WriteSglChanPar("FASTTRIGBACKLEN", valf64, mod_n, ch_n))
			return -E_PIXIE_GENERAL;
		break;
	case 68:
		/* fifo delay */
		if (Pixie16WriteSglChanPar("ExternDelayLen", valf64, mod_n, ch_n))
			return -E_PIXIE_GENERAL;
		break;
	case 69:
		/* ChVT Width */
		if (Pixie16WriteSglChanPar("ChanTrigStretch", valf64, mod_n, ch_n))
			return -E_PIXIE_GENERAL;
		break;
	case 70:
		/* ModVT Width */
		if (Pixie16WriteSglChanPar("ExtTrigStretch", valf64, mod_n, ch_n))
			return -E_PIXIE_GENERAL;
		break;
	case 71:
		/* Veto Width */
		if (Pixie16WriteSglChanPar("VetoStretch", valf64, mod_n, ch_n))
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
	
		/* start runs synchronisely */
		mod_n = 0;
		val32 = 1;
		if (Pixie16WriteSglModPar("SYNCH_WAIT", val32, mod_n)) 
			return -E_PIXIE_GENERAL;
		
		/* reset clock */
		if (reset_clk) {
			mod_n = 0;
			val32 = 0;
			if (Pixie16WriteSglModPar("IN_SYNCH", val32, mod_n)) 
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
		if (Pixie16ReadSglModPar("IN_SYNCH", &val32, mod_n))
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
		if (Pixie16WriteSglModPar("SlotID", val64, mod_n))
			return -E_PIXIE_GENERAL;
		break;
	case 1009:
		/* dsp crate number */
		if (Pixie16WriteSglModPar("CrateID", val64, mod_n))
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
	if (Pixie16ReadSglModPar(par_name, &val32, mod_n))
		return -E_PIXIE_GENERAL;
	val32 = set_bits_int(val32, start_bit, n_bit, bit_val);
	if (Pixie16WriteSglModPar(par_name, val32, mod_n))
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
	if (Pixie16ReadSglChanPar(par_name, &valf64, mod_n, ch_n))
		return -E_PIXIE_GENERAL;
	valf64 = set_bits_double(valf64, start_bit, n_bit, bit_val);
	if (Pixie16WriteSglChanPar(par_name, valf64, mod_n, ch_n))
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
	if (Pixie16AdjustOffsets(mod_n))
		return -E_PIXIE_GENERAL;
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
	ret |= Pixie16WriteSglModPar("FAST_FILTER_RANGE" , t_range, mod_n      );
	ret |= Pixie16WriteSglModPar("SLOW_FILTER_RANGE" , e_range, mod_n      );
	ret |= Pixie16WriteSglChanPar("TRACE_LENGTH"     , tr_len , mod_n, ch_n);
	ret |= Pixie16WriteSglChanPar("TRACE_DELAY"      , tr_del , mod_n, ch_n);
	ret |= Pixie16WriteSglChanPar("TRIGGER_FLATTOP"  , t_gap  , mod_n, ch_n);
	ret |= Pixie16WriteSglChanPar("TRIGGER_RISETIME" , t_rise , mod_n, ch_n);
	ret |= Pixie16WriteSglChanPar("TRIGGER_FLATTOP"  , t_gap  , mod_n, ch_n);
	ret |= Pixie16WriteSglChanPar("TRIGGER_THRESHOLD", t_thre , mod_n, ch_n);
	ret |= Pixie16WriteSglChanPar("ENERGY_FLATTOP"   , e_gap  , mod_n, ch_n);
	ret |= Pixie16WriteSglChanPar("ENERGY_RISETIME"  , e_rise , mod_n, ch_n);
	ret |= Pixie16WriteSglChanPar("ENERGY_FLATTOP"   , e_gap  , mod_n, ch_n);
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
	if (if_auto < 0) return 0;
	if (cut_val < 0) return 0;

	if (if_auto) {
		unsigned int tmp;
		if (Pixie16BLcutFinder(mod_n, ch_n, &tmp))
			return -E_PIXIE_GENERAL;
	}
	else {
		if (Pixie16WriteSglChanPar("BLCUT", cut_val, mod_n, ch_n))
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

