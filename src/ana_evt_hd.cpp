#include "ana_evt_hd.h"
#include "err_code.h"


ana_evt_hd::ana_evt_hd()
{
	ts = 0xFFFFFFFFFFFFFFFF;
	unix_t = 0xFFFFFFFFFFFFFFFF;
	len = 0xFFFFFFFF;
}

ana_evt_hd::~ana_evt_hd()
{}

int ana_evt_hd::parse_hd(uint32_t* hd)
{
	len = hd[0];
	ts = hd[1];
	ts <<= 32;
	ts += hd[2];
	unix_t = hd[3];
	unix_t <<= 32;
	unix_t += hd[4];

	return 0;
}

