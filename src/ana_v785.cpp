#include "ana_v785.h"
#include "err_code.h"

ana_v785::ana_v785()
{
	mod_id = 8;
}

ana_v785::~ana_v785()
{}

int ana_v785::parse_raw(uint32_t* raw_data)
{
	uint32_t sig, ch, entry;
	int i = 32;
	for (i = 0; i < 32; i++) 
		adc_val[i] = 0x0;
	
	/* check if the first word is header */
	sig = (raw_data[0] >> 24) & 0x7;
	if (sig != 2)
		return -E_DATA_V785;

	/* loop the event */
	i = 1;
	while (true) {
		entry = raw_data[i++];
		sig = (entry >> 24) & 0x7;
		if (sig == 4)
			/* end of event */
			break;
		if (sig == 0) {
			ch = (entry >> 16) & 0x1f;
			adc_val[ch] = entry & 0xfff;
		}
	}

	return 0;
}
