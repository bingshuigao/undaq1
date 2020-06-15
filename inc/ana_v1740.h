/* this class analyze the raw data from v1740 modules. 
 *
 * By B.Gao Jun. 14, 2020 
 * */

#ifndef ANA_V1740_HHH
#define ANA_V1740_HHH

#include <stdint.h>
#include <vector>

struct v1740_data
{
	std::vector<uint16_t> samp[64];
};

class ana_v1740
{
public:
	ana_v1740(int n);
	~ana_v1740();

	/* parse the raw data from v1740 modules. The results is saved in the
	 * internal member variable 'wave' The 'wave' is a structure
	 * containning an array of vectors, which behaves like 2D arrays. To
	 * get the j'th sample of the i'th channel, use wave->samp[i][j]. 
	 * return 0 if succeed, otherwise return error code.                                                         
	 * */
	int parse_raw(uint32_t* raw_data);

	struct v1740_data* get_wave() {return wave;}

private:

	/* set the member variable grp_msk according to the event
	 * header, return total number of enabled channels  */
	int set_grp_msk(uint32_t hd_grp_msk);

	/* get samples for one group */
	void get_ch_samples(uint32_t* raw_data, int grp)
private:

	/* indicates if a group participates the data, 1 means yes, 0
	 * means no */
	bool grp_msk[8];

	/* number of samples of each waveform*/
	int n_samp;

	/* the internal parsed data */
	struct v1740_data* wave;

};




#endif 
