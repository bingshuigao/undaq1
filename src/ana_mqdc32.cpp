#include "ana_mqdc32.h"
#include "err_code.h"

ana_mqdc32::ana_mqdc32()
{
	mod_id = 12;
}

ana_mqdc32::~ana_mqdc32()
{}

int ana_mqdc32::parse_raw(uint32_t* raw_data)
{
	uint32_t sig, ch, sub_sig, entry;
	int i = 32;
	for (i = 0; i < 32; i++) 
		qdc_val[i] = 0;
	
	/* check if the first word is header */
	sig = raw_data[0] >> 30;
	if (sig != 1)
		return -E_DATA_MQDC32;

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
			sub_sig = entry >> 21;
			if (sub_sig != 0x20)
				continue;
			ch = (entry >> 16) & 0x1f;
			qdc_val[ch] = entry & 0x1fff;
		}
	}

	return 0;
}

