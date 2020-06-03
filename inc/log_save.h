#ifndef LOG_SAVE_HHH
#define LOG_SAVE_HHH

/* This is the save thread of the logger . In this thread, it
 * reads data from ring buffer and save it to the disk.
 *
 * By B.Gao Jan. 2019 */

#include "log_thread.h"
#include <string>
#include <stdio.h>

class log_save : public log_thread
{
public:
	log_save();
	~log_save();

private:
	static int log_save_init(my_thread* This, initzer* the_initzer); 

	/* implementation of virtual functions declared in base classes. See
	 * comments in base classes for explainations */
	int handle_msg(uint32_t* msg_body);
	int start();
	int stop();
	int quit();
	int main_proc();

	/* save data to disk
	 * @param rb the ring buffer where the data is from
	 * @param flag set to true if the target ring buffer is not empty,
	 * otherwise set to false 
	 * return 0 if succeed, otherwise return error code.
	 * */
	int save_data(ring_buf* rb, bool& flag);


private:
	int run_num;
	bool save;
	std::string path;
	FILE* fp_trig;
	FILE* fp_scal;
	unsigned char* data_buf;
	int buf_len;
	char run_title[128];
};



#endif 

