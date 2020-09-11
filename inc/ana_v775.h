/* this class analyze the raw data from v775 modules. 
 *
 * By B.Gao Sep. 11, 2020 
 * */

#ifndef ANA_V775_HHH
#define ANA_V775_HHH

#include <stdint.h>
#include "ana_module.h"

class ana_v775 : public ana_module
{
public:
	ana_v775();
	~ana_v775();

	/* parse the raw data from v775 modules. return 0 if succeed,
	 * otherwise return error code.
	 * */
	int parse_raw(uint32_t* raw_data);

	/* get tdc values */
	uint32_t* get_tdc_val() {return tdc_val;}

private:
	uint32_t tdc_val[32];
};




#endif 
