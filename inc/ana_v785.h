/* this class analyze the raw data from v785 modules. 
 *
 * By B.Gao Sep. 11, 2020 
 * */

#ifndef ANA_V785_HHH
#define ANA_V785_HHH

#include <stdint.h>
#include "ana_module.h"

class ana_v785 : public ana_module
{
public:
	ana_v785();
	~ana_v785();

	/* parse the raw data from v785 modules. return 0 if succeed,
	 * otherwise return error code.
	 * */
	int parse_raw(uint32_t* raw_data);

	/* get adc values */
	uint32_t* get_adc_val() {return adc_val;}

private:
	uint32_t adc_val[32];
};




#endif 
