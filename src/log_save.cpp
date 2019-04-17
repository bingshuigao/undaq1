#include "log_save.h"
#include <stdio.h>
#include <unistd.h>
#include "err_code.h"

log_save::log_save()
{
	thread_id = 2;
	run_num = 0;
	save = false;
	fp_trig = NULL;
	fp_scal = NULL;
	data_buf = NULL;
	init_fun.push_back(&log_save_init);
}

log_save::~log_save()
{
}

int log_save::log_save_init(my_thread* This, initzer* the_initzer)
{
	log_save* ptr = reinterpret_cast<log_save*>(This);
	ptr->path = the_initzer->get_log_save_path();
	ptr->buf_len = the_initzer->get_log_save_buf_len();
	ptr->data_buf = new unsigned char[ptr->buf_len];

	return 0;
}

int log_save::handle_msg(uint32_t* msg_body)
{
	/* The message type of the current thread are defined as following 
	 * 1 --> run status transition
	 *       In this case, the new status is in msg_body[1], the run number
	 *       is in msg_body[2], the if_save flag is in msg_body[3];
	 * 2 --> To be defined yet 
	 * */
	uint32_t msg_type = msg_body[0] & 0xFFFFFF;
	switch (msg_type) {
	case 1:
		/* run status transition
		 * */
		if (msg_body[1] == DAQ_RUN) {
			run_num = msg_body[2];
			save = msg_body[3];
		}
		return switch_run(msg_body[1]);
	default:
		return -E_MSG_TYPE;
	}

}

int log_save::start()
{
	acq_stat = DAQ_RUN;

	/* open a file for write if the save flag is set */
	if (save) {
		char buf[100];
		std::string f_name;
		sprintf(buf, "run%05d.trig", run_num);
		f_name = path + '/' + buf;
		fp_trig = fopen(f_name.c_str(), "wb");
		sprintf(buf, "run%05d.scal", run_num);
		f_name = path + '/' + buf;
		fp_scal = fopen(f_name.c_str(), "wb");
		if ((!fp_trig) || (!fp_scal))
			return -E_OPEN_FILE;
	}

	/* proporgate the start to next thread */
	return send_msg(1, 1, &acq_stat, 4);
}

int log_save::stop()
{
	acq_stat = DAQ_STOP;

	/* close the data files */
	if (save) {
		if (fclose(fp_trig) || fclose(fp_scal))
			return -E_CLOSE_FILE;
	}

	/* proporgate the stop message to the next thread */
	return send_msg(3, 1, &acq_stat, 4);
}

int log_save::quit()
{
	acq_stat = DAQ_EXIT;
	return 0;
}

int log_save::save_data(ring_buf* rb, bool& flag)
{
	int sz, ret;

	if (rb->get_lock())
		return -E_SYSCALL;
	sz = rb->get_used1();
	if (sz) {
		/* data available */
		FILE* fp = NULL;
		flag = true;
		sz = rb->read1(data_buf, buf_len);
		rb->rel_lock();
		if (sz < 0) 
			return -E_RING_BUF_DATA;
		if (rb == rb_trig)
			fp = fp_trig;
		else if (rb == rb_scal)
			fp = fp_scal;
		ret = fwrite(data_buf, 1, sz, fp);
		if (ret != sz) 
			return -E_SYSCALL;
	}
	else {
		rb->rel_lock();
		flag = false;
	}

	return 0;
}

int log_save::main_proc()
{
	int sz, ret;
	bool flag1, flag2;

	/* check the ring buffer to see if any data available */
	/* the scaler ring buffer */
	ret = save_data(rb_scal, flag1);
	RET_IF_NONZERO(ret);
	ret = save_data(rb_trig, flag2);
	RET_IF_NONZERO(ret);
	if ((!flag1) && (!flag2)) 
		/* no data at all, sleep */
		usleep(120);
	return 0;

}
