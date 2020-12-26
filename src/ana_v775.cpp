#include "ana_v775.h"
#include "err_code.h"

ana_v775::ana_v775()
{
	mod_id = 6;
}

ana_v775::~ana_v775()
{}

int ana_v775::parse_raw(uint32_t* raw_data)
{
	uint32_t sig, ch, entry;
	int i = 32;
	for (i = 0; i < 32; i++) 
		tdc_val[i] = 0xffffffff;
	
	/* check if the first word is header */
	sig = (raw_data[0] >> 24) & 0x7;
	if (sig != 2)
		return -E_DATA_V775;

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
			tdc_val[ch] = entry & 0xfff;
		}
	}

	return 0;
}
