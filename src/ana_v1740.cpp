#include "ana_v1740.h"
#include "err_code.h"

ana_v1740::ana_v1740(int n)
{
	int i;
	n_samp = n;
	wave = new struct v1740_data;
	for (i = 0; i < 64; i++) 
		wave->samp[i].reserve(n);
}

ana_v1740::~ana_v1740()
{
	delete wave;
}


int ana_v1740::set_grp_msk(uint32_t hd_grp_msk)
{
	int i;
	int total = 0;
	for (i = 0; i < 8; i++) {
		if (hd_grp_msk & (1<<i)) {
			grp_msk[i] = true;
			total += 1;
		}
		else {
			grp_msk[i] = false;
		}
	}

	return total;
}


static inline uint16_t do_get_ch_samples(uint32_t* raw_data, int ch, int samp)
{
	/* see v1740 manual for the data format */
	int sub_block = samp/3;
	int sub_samp = samp%3;
	int n_bit = (ch * 3 + sub_samp) * 12;
	int n_wd = n_bit / 32;
	int n_sub_bit = n_bit % 32;
	raw_data += sub_block * 9;

	if (n_sub_bit <= 20) {
		return (raw_data[n_wd]>>n_sub_bit) & 0xfff;
	}
	else {
		return ((raw_data[n_wd]>>n_sub_bit) |
				(raw_data[n_wd+1]<<(32-n_sub_bit))) & 0xfff;
	}
}

void ana_v1740::get_ch_samples(uint32_t* raw_data, int grp)
{
	/* the algrithm below may be not fast, but easy to implement. */
	int i, j;
	for (i = 0; i < 8; i++) 
		for (j = 0; j < n_samp; j++) 
			wave->samp[i+grp*8][j] = do_get_ch_samples(raw_data, i, j);
}

int ana_v1740::parse_raw(uint32_t* raw_data)
{
	uint32_t sig, n_ch, n_samp1, evt_sz;
	int i, j;

	sig = raw_data[0]>>28;
	if (sig != 0xa) 
		return -E_DATA_V1740;

	n_ch = set_grp_msk(raw_data[1]);
	evt_sz = raw_data[0] & 0xfffffff;
	n_samp1 = (evt_sz - 4 ) / (n_ch*8) * 32 / 12;
	if (n_samp1 != n_samp)
		return -E_DATA_V1740;

	raw_data += 4;
	for (i = 0; i < 8; i++) {
		if (!grp_msk[i])
			continue;
		get_ch_samples(raw_data, i);
		raw_data += 8*n_samp*12/32;
	}

	return 0;
}
