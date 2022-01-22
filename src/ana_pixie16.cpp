#include "ana_pixie16.h"
#include "err_code.h"

ana_pixie16::ana_pixie16(const char* f_dsp)
{
	int i,j;
	
	mod_id = 14;
	for (i = 0; i < MAX_CRATE; i++) {
		for (j = 0; j < 16; j++) {
			n_pt[i][j] = 0;
			ptr_qdc[i][j] = -1;
			ptr_energy_ga[i][j] = -1;
			ptr_energy_le[i][j] = -1;
			ptr_energy_tr[i][j] = -1;
			ptr_energy[i][j] = -1;
			ptr_cfd[i][j] = -1;
			ptr_ts_ext[i][j] = -1;
			ptr_ts[i][j] = -1;
		}
	}

	parse_dsp(f_dsp);
}

ana_pixie16::~ana_pixie16()
{}

void ana_pixie16::parse_dsp(const char* f_dsp)
{

}

int ana_pixie16::parse_raw(uint32_t* raw_data)
{
	crate = (raw_data[0]>>8) & 0xf;
	slot  = (raw_data[0]>>4) & 0xf;
	ch    = (raw_data[0]>>0) & 0xf;
	hd_ptr = raw_data;
	
	_get_ts();
	_get_cfd();
	_get_energy();
	_get_energy_tr();
	_get_energy_le();
	_get_energy_ga();
	_get_qdcs();
	return _get_wave();
}

void ana_pixie16::_get_ts()
{
	uint64_t ts_lo, ts_hi;
	int ptr = ptr_ts[slot][ch];

	if (ptr < 0) 
		return;
	ts_hi = hd_ptr[ptr+1] & 0xffff;
	ts_lo = hd_ptr[ptr];
	pixie16_data.ts = (ts_hi<<32)+ts_lo;

	/* ts is inhirated from the base class*/
	ts = pixie16_data.ts;
}
void ana_pixie16::_get_ts_ext()
{
	uint64_t ts_lo, ts_hi;
	int ptr = ptr_ts_ext[slot][ch];

	if (ptr < 0) 
		return;
	ts_hi = hd_ptr[ptr+1] & 0xffff;
	ts_lo = hd_ptr[ptr];
	pixie16_data.ts_ext = (ts_hi<<32)+ts_lo;
	
	/* ts is inhirated from the base class*/
	ts = pixie16_data.ts_ext;
}

void ana_pixie16::_get_cfd()
{
	int ptr = ptr_cfd[slot][ch];

	if (ptr < 0) 
		return;
	pixie16_data.cfd = hd_ptr[ptr] >> 16;
}

void ana_pixie16::_get_energy()
{
	int ptr = ptr_energy[slot][ch];

	if (ptr < 0) 
		return;
	pixie16_data.energy = hd_ptr[ptr] & 0xffff;
}

void ana_pixie16::_get_energy_tr()
{
	int ptr = ptr_energy_tr[slot][ch];

	if (ptr < 0) 
		return;
	pixie16_data.energy_tr = hd_ptr[ptr];
}
void ana_pixie16::_get_energy_le()
{
	int ptr = ptr_energy_le[slot][ch];

	if (ptr < 0) 
		return;
	pixie16_data.energy_le = hd_ptr[ptr];
}
void ana_pixie16::_get_energy_ga()
{
	int ptr = ptr_energy_ga[slot][ch];

	if (ptr < 0) 
		return;
	pixie16_data.energy_ga = hd_ptr[ptr];

}
void ana_pixie16::_get_qdcs()
{
	int ptr = ptr_qdc[slot][ch];

	if (ptr < 0) 
		return;
	pixie16_data.qdc0 = hd_ptr[ptr + 0];
	pixie16_data.qdc1 = hd_ptr[ptr + 1];
	pixie16_data.qdc2 = hd_ptr[ptr + 2];
	pixie16_data.qdc3 = hd_ptr[ptr + 3];
	pixie16_data.qdc4 = hd_ptr[ptr + 4];
	pixie16_data.qdc5 = hd_ptr[ptr + 5];
	pixie16_data.qdc6 = hd_ptr[ptr + 6];
	pixie16_data.qdc7 = hd_ptr[ptr + 7];
}
	
int ana_pixie16::_get_wave()
{
	int i, hd_len, wave_n;

	wave_n = n_pt[slot][ch];
	if (wave_n == 0) 
		return 0;
	if (wave_n >= MAX_PIXIE16_WAVE)
		return -E_PIXIE_WAVE_TOO_LONG;

	hd_len = (hd_ptr[0]>>12) & 0x1f;
	for (i = 0; i < wave_n; i++) 
		pixie16_data.wave[i] = (hd_ptr[hd_len + i/2] >> ((i%2)*16))
			& 0x3fff;
	return 0;
}

