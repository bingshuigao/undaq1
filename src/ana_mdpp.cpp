#include "ana_mdpp.h"
#include "err_code.h"

ana_mdpp::ana_mdpp(int fw_ver, int max_hits)
{
	mod_id = 15;
	this->fw_ver = fw_ver;
	this->max_hits = max_hits;

	adc_val = new uint32_t[16*max_hits];
	tdc_val = new uint32_t[16*max_hits];
	qdc_val = new uint32_t[16*max_hits];
	trig_t  = new uint32_t[2 *max_hits];
}

ana_mdpp::~ana_mdpp()
{
	delete adc_val;
	delete tdc_val;
	delete qdc_val;
	delete trig_t;
}

int ana_mdpp::parse_raw(uint32_t* raw_data)
{
	uint32_t sig, ch, sub_sig, entry, val;
	int i, j, hit_num;
	int n_hit[34];

	for (j = 0; j < max_hits; j++) {
		for (i = 0; i < 16; i++) {
			adc_val[j*16+i] = 0;
			tdc_val[j*16+i] = 0xffffffff;
			qdc_val[j*16+i] = 0;
		}
		trig_t[j*2] = 0xffffffff;
		trig_t[j*2+1] = 0xffffffff;
	}
	for (i = 0; i < 34; i++) 
		n_hit[i] = 0;
	
	/* only scp firmware is currently supported */
	if (fw_ver != 2) 
		return -E_NOT_SUPPORT;
	
	/* check if the first word is header */
	sig = raw_data[0] >> 30;
	if (sig != 1)
		return -E_DATA_MDPP;

	/* loop the event */
	i = 1;
	while (true) {
		entry = raw_data[i++];
		sig = entry >> 30;
		if (sig == 3) {
			/* end of event */
			ts = entry & 0x3fffffff;
			evt_cnt = ts;
			break;
		}
		if (sig == 0) {
			sub_sig = entry >> 28;
			if (sub_sig != 1)
				continue;
			ch = (entry >> 16) & 0x3f;
			hit_num = (n_hit[ch]<max_hits) ? n_hit[ch] : max_hits;
			val = entry & 0xffff;
			if (ch <= 15) 
				adc_val[hit_num*16 + ch] = val;
			else if (ch <= 31)
				tdc_val[hit_num*16 + ch-16] = val;
			else if (ch <= 33)
				trig_t[hit_num*2 + ch-32] = val;
			if (ch <= 33)
				n_hit[ch]++;
		}
	}

	return 0;
}

