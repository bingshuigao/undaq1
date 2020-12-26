#include "ana_v1751.h"
#include "err_code.h"

ana_v1751::ana_v1751(int n)
{
	int i;
	n_samp = n;
	for (i = 0; i < 8; i++) 
		samp[i].reserve(n);
	mod_id = 11;
}

ana_v1751::~ana_v1751()
{
}


int ana_v1751::set_ch_msk(uint32_t hd_ch_msk)
{
	int i;
	int total = 0;
	for (i = 0; i < 8; i++) {
		if (hd_ch_msk & (1<<i)) {
			ch_msk[i] = true;
			total += 1;
		}
		else {
			ch_msk[i] = false;
		}
	}

	return total;
}


void ana_v1751::get_ch_samples(uint32_t* raw_data, int ch)
{
	/* the algrithm below may be not fast, but easy to implement. */
	int i;
	for (i = 0; i < n_samp; i++) 
		samp[ch][i] = (raw_data[i/3]>>((i%3)*10))&0x3ff;
}

int ana_v1751::parse_raw(uint32_t* raw_data)
{
	uint32_t sig, n_ch, n_samp1, evt_sz;
	int i, j, mod;

	sig = raw_data[0]>>28;
	if (sig != 0xa) 
		return -E_DATA_V1751;

	evt_cnt = raw_data[2] & 0xffffff;
	ts = raw_data[3];
	n_ch = set_ch_msk(raw_data[1]);
	evt_sz = raw_data[0] & 0xfffffff;
	n_samp1 = (evt_sz - 4 ) / (n_ch) * 3;
//	mod = (((n_samp1*100)/1024/175)%2)+1;
//	n_samp1 -= mod;
	if (n_samp1 != n_samp)
		return -E_DATA_V1751;

	raw_data += 4;
	for (i = 0; i < 8; i++) {
		if (!ch_msk[i])
			continue;
		get_ch_samples(raw_data, i);
		raw_data += n_samp/3+1;
	}

	return write_data_ptr();
}

int ana_v1751::write_data_ptr()
{
	int i, j;
	for (i = 0; i < 8; i++) {
		if (!data_ptr[i])
			continue;
		for (j = 0; j < n_samp; j++) {
			data_ptr[i][j] = samp[i][j];
		}
	}

	return 0;
}
