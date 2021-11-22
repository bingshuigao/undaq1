/* This class analyze the event header 
 *
 * By B.Gao, Jan. 12, 2020 */

#include <stdint.h>
#include "imp_daq.h"

#ifndef ANA_EVT_HD_HHH
#define ANA_EVT_HD_HHH

class ana_evt_hd
{
public:
	ana_evt_hd(); 
	~ana_evt_hd();

	/* parse the header, extract the corresponding information into its
	 * class members ,
	 * @param hd The pointer to the header.
	 * @return return 0 is successfully parsed, otherwise return error
	 * code.
	 *
	 * */
	int parse_hd(uint32_t* hd);

	/* get length the header information */
	uint32_t get_len() {return len;}
	uint64_t get_ts() {return ts;}
	uint64_t get_unix_t() {return unix_t;}

private:
	uint32_t len;
	uint64_t ts;
	uint64_t unix_t;
};


#endif
