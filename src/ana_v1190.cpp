#include "ana_v1190.h"
#include "err_code.h"

ana_v1190::ana_v1190()
{
	mod_id = 2;
}

ana_v1190::~ana_v1190()
{}

int ana_v1190::parse_raw(uint32_t* raw_data)
{
	uint32_t sig, ch, entry, ch_val;
	int i;

	for (i = 0; i < 128; i++) 
		val[i].clear();
	
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
			ch_val = entry & 0x7ffff;
			val[ch].push_back(ch_val);
		}
	}

	return write_data_ptr();
}

int ana_v1190::write_data_ptr()
{
	int i, j, n_hit;
	for (i = 0; i < 128; i++) {
		if (!v1190_ch_data[i])
			continue;
		n_hit = val[i].size();
		if (n_hit >= MAX_V1190_HIT) 
			n_hit = MAX_V1190_HIT;
		v1190_ch_data[i]->n_hit = n_hit;
		for (j = 0; j < n_hit; j++) 
			v1190_ch_data[i]->tdc_val[j] = val[i][j];
	}

	return 0;
}
