/* this class analyze the raw data from v830 modules. 
 *
 * By B.Gao Nov. 15, 2020 
 * */

#ifndef ANA_V830_HHH
#define ANA_V830_HHH

#include <stdint.h>
#include "ana_module.h"

class ana_v830 : public ana_module
{
public:
	ana_v830();
	~ana_v830();

	/* parse the raw data from v830 modules. return 0 if succeed,
	 * otherwise return error code.
	 * */
	int parse_raw(uint32_t* raw_data);

	/* get scaler values */
	uint32_t* get_scaler_val() {return scaler_val;}

private:
	uint32_t scaler_val[32];
};




#endif 
