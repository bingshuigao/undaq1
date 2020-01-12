#include "ana_frag_hd.h"

ana_frag_hd::ana_frag_hd()
{}
ana_frag_hd::~ana_frag_hd()
{}

int ana_frag_hd::parse_hd(uint32_t* hd)
{
	slot = hd[0] & 0xFF;
	crate = (hd[0] >> 8) & 0xFF;
	daq = (hd[0] >> 16) & 0xFF;
	len = hd[1];
	ts = hd[3];
	ts <<= 32;
	ts += hd[4];

	return 0;
}

