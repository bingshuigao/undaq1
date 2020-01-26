#include "ana_v1190.h"
#include "err_code.h"

ana_v1190::ana_v1190()
{}

ana_v1190::~ana_v1190()
{}

int ana_v1190::parse_raw(uint32_t* raw_data, struct v1190_data& v1190_data)
{
	uint32_t sig, ch, entry, val;
	int i;

	for (i = 0; i < 128; i++) 
		v1190_data.val[i].clear();
	
	/* check if the first word is header */
	sig = raw_data[0] >> 27;
	if (sig != 8)
		return -E_DATA_V1190;

	/* loop the event */
	i = 1;
	while (true) {
		entry = raw_data[i++];
		sig = entry >> 27;
		if (sig == 0x10)
			/* end of event */
			break;
		if (sig == 0) {
			ch = (entry >> 19) & 0x7f;
			val = entry & 0x7ffff;
			v1190_data.val[ch].push_back(val);
		}
	}

	return 0;
}
