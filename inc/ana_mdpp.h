/* this class analyze the raw data from mdpp modules. 
 *
 * By B.Gao Apr. 28, 2022 
 * */

#ifndef ANA_MDPP_HHH
#define ANA_MDPP_HHH

#include <stdint.h>
#include "ana_module.h"

class ana_mdpp : public ana_module
{
public:
	/* firmware version: 
	 * rcp = 1
	 * scp = 2
	 * qdc = 3 */
	ana_mdpp(int fw_ver, int max_hits);
	~ana_mdpp();

	/* parse the raw data from mdpp modules. return 0 if succeed,
	 * otherwise return error code.
	 * */
	int parse_raw(uint32_t* raw_data);

	/* get adc/tdc/qdc values. Those pointers represents 2d arrays in fact.
	 * For example, to get the adc value of n'th hit of channel m:
	 * adc_val[16*n + m]. */
	uint32_t* get_adc_val() {return adc_val;}
	uint32_t* get_tdc_val() {return tdc_val;}
	uint32_t* get_qdc_val() {return qdc_val;}
	uint32_t* get_trig_t() {return trig_t;}
	int get_max_hit() {return max_hits;}



private:
	uint32_t* adc_val;
	uint32_t* tdc_val;
	uint32_t* qdc_val;
	uint32_t* trig_t;
	int fw_ver;
	int max_hits;
};




#endif 
