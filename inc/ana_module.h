/* this class is the base class to parse vme raw data 
 *
 * By B.Gao Jun. 22, 2020 */

#ifndef ANA_MODULE_HHH
#define ANA_MODULE_HHH

#include <stdint.h>

class ana_module
{
public:
	ana_module(){};
	~ana_module(){};

	/* parse the raw data. Return 0 if sucess, otherwise return error code
	 * */
	virtual int parse_raw(uint32_t* raw_data) = 0;

	/* get timestamp or event counter. */
	uint64_t get_ts() {return ts;}
	uint64_t get_cnt() {return evt_cnt;}
	
	int get_mod_id(){return mod_id;}

protected:
	/* see definations in module.h */
	int mod_id;
	int64_t ts;
	int64_t evt_cnt;
};



#endif
