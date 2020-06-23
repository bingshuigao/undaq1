/* this class analyze the raw data from v1190 modules. 
 *
 * By B.Gao Jan. 15, 2020 
 * */

#ifndef ANA_V1190_HHH
#define ANA_V1190_HHH


#include <stdint.h>
#include <vector>
#include "ana_module.h"


#define MAX_V1190_HIT 1000

struct v1190_channel_data
{
	uint32_t n_hit;
	uint32_t tdc_val[MAX_V1190_HIT];
};

class ana_v1190 : public ana_module
{
public:
	ana_v1190();
	~ana_v1190();

	/* parse the raw data from v1190 modules.  return 0 if succeed,
	 * otherwise return error code.
	 * */
	int parse_raw(uint32_t* raw_data);

	/* get tdc values */
	std::vector<uint32_t>* get_val() {return val;}

	struct v1190_channel_data* get_data_ptr(int i) {return v1190_ch_data[i];}
	void set_data_ptr(int i, struct v1190_channel_data* ptr) {v1190_ch_data[i] = ptr;}

private:
	std::vector<uint32_t> val[128];

	/* this is for raw2root */
	struct v1190_channel_data* v1190_ch_data[128];

};




#endif 
