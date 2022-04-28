#include "ana_mdpp.h"
#include "err_code.h"

ana_mdpp::ana_mdpp(int fw_ver)
{
	mod_id = 15;
	this->fw_ver = fw_ver;
}

ana_mdpp::~ana_mdpp()
{}

int ana_mdpp::parse_raw(uint32_t* raw_data)
{
	uint32_t sig, ch, sub_sig, entry, val;
	int i = 32;

	for (i = 0; i < 16; i++) {
		adc_val[i] = 0;
		tdc_val[i] = 0xffffffff;
		qdc_val[i] = 0;
	}
	trig_t[0] = 0xffffffff;
	trig_t[1] = 0xffffffff;
	
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
			val = entry & 0xffff;
			if (ch <= 15) 
				adc_val[ch] = val;
			else if (ch <= 31)
				tdc_val[ch-16] = val;
			else
				trig_t[ch-32] = val;
		}
	}

	return 0;
}

