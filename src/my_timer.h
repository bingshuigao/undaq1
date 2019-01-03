/* This is a simple timer. For the sake of speed, the timer is (by default)
 * implemented using the clock CLOCK_MONOTONIC_COARSE which has lower
 * resolution.  
 * 
 *
 * By B.Gao Oct. 2018 */

#ifndef MY_TIMER_HHH
#define MY_TIMER_HHH
#include "err_code.h"
#include <stdint.h>
#include <time.h>

class my_timer
{
public:
	my_timer() {clk_id = CLOCK_MONOTONIC_COARSE;}
	~my_timer();

	/* reset the timer */
	int reset() 
	{
		if (clock_gettime(clk_id, &start_t))
			return -E_SYSCALL;
		return 0;
	}

	/* return true if timeout (since last reset), otherwise return false.
	 * The timeout is in unit of microsecond */
	bool time_out(uint64_t t_us, int* status = NULL)
	{
		struct timespec current_t;
		int ret;
		uint64_t t_ellipse;
		
		ret = clock_gettime(clk_id, &current_t);
		if (ret) {
			if (status)
				*status = -E_SYSCALL;
			return false;
		}
		if (status)
			*status = 0;
		t_ellipse = (current_t.tv_sec - start_t.tv_sec);
		t_ellipse = t_ellipse * 1000000 + 
			(current_t.tv_nsec - start_t.tv_nsec)/1000;
		if (t_ellipse >= t_us)
			return true;
		else
			return false;
	}
	
	void set_clockid(clockid_t id) {clk_id = id;}

private:
	clockid_t clk_id;
	timespec start_t;
};



#endif
