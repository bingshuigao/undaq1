/* This class analyze the fragment header 
 *
 * By B.Gao, Jan. 12, 2020 */

#include <stdint.h>
#include "imp_daq.h"

#ifndef ANA_FRAG_HD_HHH
#define ANA_FRAG_HD_HHH

class ana_frag_hd
{
public:
	ana_frag_hd(); 
	~ana_frag_hd();

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
	uint32_t get_slot() {return slot;}
	uint32_t get_crate() {return crate;}
	uint32_t get_daq() {return daq;}

private:
	uint32_t len;
	uint32_t slot;
	uint32_t daq;
	uint32_t crate;
	uint64_t ts;
};


#endif
