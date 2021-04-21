#include "ana_v792.h"
#include "err_code.h"

ana_v792::ana_v792()
{
	mod_id = 13;
}

ana_v792::~ana_v792()
{}

int ana_v792::parse_raw(uint32_t* raw_data)
{
	uint32_t sig, ch, entry;
	int i = 32;
	for (i = 0; i < 32; i++) 
		qdc_val[i] = 0xffffffff;
	
	/* check if the first word is header */
	sig = (raw_data[0] >> 24) & 0x7;
	if (sig != 2)
		return -E_DATA_V792;

	/* loop the event */
	i = 1;
	while (true) {
		entry = raw_data[i++];
		sig = (entry >> 24) & 0x7;
		if (sig == 4) {
			/* end of event */
			evt_cnt = entry & 0xffffff;
			break;
		}
		if (sig == 0) {
			ch = (entry >> 16) & 0x1f;
			qdc_val[ch] = entry & 0xfff;
		}
	}

	return 0;
}
