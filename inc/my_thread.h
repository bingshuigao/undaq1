 /* This is a thin wrapper of the pthread multithreading.
  *
 * By B.Gao Nov. 2018 */

#ifndef MY_THREAD
#define MY_THREAD

#include "initzer.h"
#include "err_code.h"
#include "imp_daq.h"
#include <pthread.h>
#include <vector>

class my_thread
{
public:
	my_thread() {ret_val = new char[1024];}
	~my_thread() {delete ret_val;}

	/* initialize the thread. Everything that is needed can be found from
	 * the initzer object. 
	 * This function calls all the initialization routines found in the
	 * vector init_fun. So the base class need to define a static member
	 * function and push it into this vector if want it be called by the
	 * init() function.
	 * Return 0 if succeed, return error code in case of error */
	int init(initzer* the_initzer) 
	{
		for (auto &f : init_fun) {
			int ret = (*f)(this, the_initzer);
			RET_IF_NONZERO(ret);
		}
		return 0;
	}

	
	/* create  the thread and run it, return 0 if succeed, otherwise return
	 * error code.*/
	int run()
	{
		int ret = pthread_create(&thread, NULL, thread_entry, this);
		if (ret)
			return -E_SYSCALL;
		return 0;
	}

	/* join the thread */
	int join(void** retval) {return pthread_join(thread, retval);}


protected:
	/* the entry point of the thread */
	virtual void* main_loop(void* arg) = 0;
	static void* thread_entry(void* This) 
	{return ((my_thread*)This)->main_loop(NULL);}



protected:
	/* the thread */
	pthread_t thread;

	/* return value of the thread */
	char* ret_val;

	/* The initialization chain */
	std::vector<int (*)(my_thread*, initzer*)> init_fun;
};



#endif 
