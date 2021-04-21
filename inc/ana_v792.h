/* this class analyze the raw data from v792 modules. 
 *
 * By B.Gao Apr. 21, 2021
 * */

#ifndef ANA_V792_HHH
#define ANA_V792_HHH

#include <stdint.h>
#include "ana_module.h"

class ana_v792 : public ana_module
{
public:
	ana_v792();
	~ana_v792();

	/* parse the raw data from v792 modules. return 0 if succeed,
	 * otherwise return error code.
	 * */
	int parse_raw(uint32_t* raw_data);

	/* get tdc values */
	uint32_t* get_qdc_val() {return qdc_val;}

private:
	uint32_t qdc_val[32];
};




#endif 
