#ifndef SCAL_CNTER_HHH
#define SCAL_CNTER_HHH

#include <stdint.h>

/* A small class used by the analyzer to store counts, 30 counters max 
 * By B.Gao Sep. 2021
 * */

class scal_cnter
{
public:
	scal_cnter(){}
	~scal_cnter(){}
	void set_cnt(int cnt_id, uint32_t cnt) {
		if (cnt_id >= 30) 
			return;
		cnters[cnt_id] = cnt;
	}
	void reset_cnter() {
		int i;
		for (i = 0; i < 30; i++) 
			cnters[i] = 0;
	}
	uint32_t* get_cnter_ptr() {return cnters;}

private:
	/* counters */
	uint32_t cnters[30];
};

#endif
