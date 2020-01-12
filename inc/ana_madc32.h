/* this class analyze the raw data from madc32 modules. 
 *
 * By B.Gao Jan. 12, 2020 
 * */

#ifndef ANA_MADC32_HHH
#define ANA_MADC32_HHH

#include <stdint.h>

class ana_madc32
{
public:
	ana_madc32();
	~ana_madc32();

	/* parse the raw data from madc32 modules, the channel values are
	 * returned in param val.  return 0 if succeed, otherwise return error
	 * code.
	 * */
	int parse_raw(uint32_t* raw_data, uint32_t* val);

};




#endif 
