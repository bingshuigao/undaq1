#include "ana_mtdc32.h"
#include "err_code.h"

ana_mtdc32::ana_mtdc32()
{
	mod_id = 16;
}

ana_mtdc32::~ana_mtdc32()
{}

int ana_mtdc32::parse_raw(uint32_t* raw_data)
{
	uint32_t sig, ch, sub_sig, entry, ch_val;
	int i;

	for (i = 0; i < 32; i++) 
		val[i].clear();
	
	/* check if the first word is header */
	sig = raw_data[0] >> 30;
	if (sig != 1)
		return -E_DATA_MTDC32;

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
			ch_val = entry & 0xffff;
			val[ch].push_back(ch_val);
		}
	}

	return write_data_ptr();
}


int ana_mtdc32::write_data_ptr()
{
	int i, j, n_hit;
	for (i = 0; i < 32; i++) {
		if (!mtdc32_ch_data[i])
			continue;
		n_hit = val[i].size();
		if (n_hit >= MAX_MTDC32_HIT) 
			n_hit = MAX_MTDC32_HIT;
		mtdc32_ch_data[i]->n_hit = n_hit;
		for (j = 0; j < n_hit; j++) 
			mtdc32_ch_data[i]->tdc_val[j] = val[i][j];
	}

	return 0;
}
