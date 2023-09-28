/* this class analyze the raw data from mtdc32 modules. 
 *
 * By B.Gao Sep., 2023
 * */

#ifndef ANA_MTDC32_HHH
#define ANA_MTDC32_HHH

#include <stdint.h>
#include <vector>
#include "ana_module.h"

#define MAX_MTDC32_HIT 100

struct mtdc32_channel_data
{
	uint32_t n_hit;
	uint32_t tdc_val[MAX_MTDC32_HIT];
};

class ana_mtdc32 : public ana_module
{
public:
	ana_mtdc32();
	~ana_mtdc32();

	/* parse the raw data from mtdc32 modules. return 0 if succeed,
	 * otherwise return error code.
	 * */
	int parse_raw(uint32_t* raw_data);

	/* get tdc values */
	std::vector<uint32_t>* get_val() {return val;}

	struct mtdc32_channel_data* get_data_ptr(int i) {return mtdc32_ch_data[i];}
	void set_data_ptr(int i, struct mtdc32_channel_data* ptr) {mtdc32_ch_data[i] = ptr;}

private:
	/* set data points for data_ptr */
	int write_data_ptr();

private:
	std::vector<uint32_t> val[32];

	/* this is for raw2root */
	struct mtdc32_channel_data* mtdc32_ch_data[32];

};


#endif 
