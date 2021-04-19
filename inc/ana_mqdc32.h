/* this class analyze the raw data from mqdc32 modules. 
 *
 * By B.Gao Apr. 19, 2021
 * */

#ifndef ANA_QADC32_HHH
#define ANA_QADC32_HHH

#include <stdint.h>
#include "ana_module.h"

class ana_mqdc32 : public ana_module
{
public:
	ana_mqdc32();
	~ana_mqdc32();

	/* parse the raw data from mqdc32 modules. return 0 if succeed,
	 * otherwise return error code.
	 * */
	int parse_raw(uint32_t* raw_data);

	/* get qdc values */
	uint32_t* get_qdc_val() {return qdc_val;}



private:
	uint32_t qdc_val[32];
};




#endif 
