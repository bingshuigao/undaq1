/* this class analyze the raw data from v1751 modules. 
 * NOTE: it does not work in DES mode.
 *
 * By B.Gao Dec. 21, 2020 
 * */

#ifndef ANA_V1751_HHH
#define ANA_V1751_HHH

#include <stdint.h>
#include <vector>
#include "ana_module.h"


class ana_v1751 : public ana_module
{
public:
	/* n is the number of points per waveform */
	ana_v1751(int n);
	~ana_v1751();

	/* parse the raw data from v1751 modules. The results is saved in the
	 * internal member variable 'samp' The 'samp' is an array of vectors,
	 * which behaves like 2D arrays. To get the j'th sample of the i'th
	 * channel, use samp[i][j].  return 0 if succeed, otherwise return
	 * error code.                                                         
	 * */
	int parse_raw(uint32_t* raw_data);

	std::vector<uint16_t>* get_samp() {return samp;}

	uint16_t* get_data_ptr(int i){return data_ptr[i];}
	void set_data_ptr(int i, uint16_t* ptr) {data_ptr[i] = ptr;}
	int get_n_samp() {return n_samp;}

private:

	/* set the member variable ch_msk according to the event
	 * header, return total number of enabled channels  */
	int set_ch_msk(uint32_t hd_ch_msk);

	/* get samples for one channel  */
	void get_ch_samples(uint32_t* raw_data, int ch);
	
	/* set data points for data_ptr */
	int write_data_ptr();
private:

	/* indicates if a channel participates the data, 1 means yes, 0
	 * means no */
	bool ch_msk[8];

	/* number of samples of each waveform*/
	int n_samp;

	/* the internal parsed data */
	std::vector<uint16_t> samp[8];

	/* this is for raw2root */
	uint16_t* data_ptr[8];

};




#endif 
