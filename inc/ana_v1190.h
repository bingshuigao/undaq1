/* this class analyze the raw data from v1190 modules. 
 *
 * By B.Gao Jan. 15, 2020 
 * */

#ifndef ANA_V1190_HHH
#define ANA_V1190_HHH


#include <stdint.h>
#include <vector>

struct v1190_data
{
	std::vector<uint32_t> val[128];
};

class ana_v1190
{
public:
	ana_v1190();
	~ana_v1190();

	/* parse the raw data from v1190 modules, the channel values are
	 * returned in param v1190_data.  return 0 if succeed, otherwise return error
	 * code.
	 * */
	int parse_raw(uint32_t* raw_data, struct v1190_data& v1190_data);

};




#endif 
