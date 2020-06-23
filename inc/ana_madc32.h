/* this class analyze the raw data from madc32 modules. 
 *
 * By B.Gao Jan. 12, 2020 
 * */

#ifndef ANA_MADC32_HHH
#define ANA_MADC32_HHH

#include <stdint.h>
#include "ana_module.h"

class ana_madc32 : public ana_module
{
public:
	ana_madc32();
	~ana_madc32();

	/* parse the raw data from madc32 modules. return 0 if succeed,
	 * otherwise return error code.
	 * */
	int parse_raw(uint32_t* raw_data);

	/* get adc values */
	uint32_t* get_adc_val() {return adc_val;}

private:
	uint32_t adc_val[32];
};




#endif 
