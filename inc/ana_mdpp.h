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
	ana_mdpp(int fw_ver);
	~ana_mdpp();

	/* parse the raw data from mdpp modules. return 0 if succeed,
	 * otherwise return error code.
	 * */
	int parse_raw(uint32_t* raw_data);

	/* get adc/tdc/qdc values */
	uint32_t* get_adc_val() {return adc_val;}
	uint32_t* get_tdc_val() {return tdc_val;}
	uint32_t* get_qdc_val() {return qdc_val;}
	uint32_t* get_trig_t() {return trig_t;}



private:
	uint32_t adc_val[16];
	uint32_t tdc_val[16];
	uint32_t qdc_val[16];
	uint32_t trig_t[2];
	int fw_ver;
};




#endif 
