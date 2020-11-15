#include "ana_v830.h"
#include "err_code.h"

ana_v830::ana_v830()
{
	mod_id = 3;
}

ana_v830::~ana_v830()
{}

int ana_v830::parse_raw(uint32_t* raw_data)
{
	uint32_t sig, n_ch;
	int i;
	for (i = 0; i < 32; i++) 
		scaler_val[i] = 0;
	
	/* check if the first word is header */
	sig = (raw_data[0] >> 26) & 1;
	if (sig != 1)
		return -E_DATA_V830;
	n_ch = (raw_data[0] >> 18) & 0x3f;

	/* loop the event */
	for (i = 0; i < n_ch; i++) {
		scaler_val[i] = raw_data[i+1];
	}
	return 0;
}
