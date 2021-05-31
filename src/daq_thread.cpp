#include "daq_thread.h"
#include "err_code.h"
#include <unistd.h>
#include <string.h>
#include <stdio.h>


daq_thread::daq_thread()
{
	rb_msg = NULL;
	acq_stat = 0;
	the_timer.reset();
	is_ctl_thread = false;
}

daq_thread::~daq_thread()
{
}

void* daq_thread::main_loop(void* arg)
{
	int* p_ret = reinterpret_cast<int*>(ret_val);
	int ret;
	char buf[100];
	int* p_buf = reinterpret_cast<int*>(buf);
	while (true) {
begin:
		if (ret = rd_msg()) {
			/* error in the (message) ring buffer communication */
			*p_ret = ret;
			goto finish;
		}
		if (acq_stat == 2) {
			/* exit normally */
			*p_ret = 0;
			goto finish;
		}
		if (acq_stat == 0) {
			/* DAQ is stopped */
			usleep(100000);
			goto begin;
		}

		/* If we reached here, the daq is in the running status, we
		 * should do what we are normally supposted to do 
		 * */
		if (*p_ret = main_proc())
			goto finish;
	}

finish:
	printf("thread (%d) exits with return code (%d)\n", thread_id, *p_ret);
	sprintf(buf, "thread (%d) exits with return code (%d)\n", 
			thread_id, *p_ret);
	send_text_mes(buf, MSG_LEV_FATAL);
	return ret_val;
}


int daq_thread::switch_run(uint32_t status)
{
	/* if the acq_stat == status, no action needed */
	/* we comment this part out because for the control thread, we should
	 * compare the values of real_stat and status, not qcq_stat...*/
//	if (acq_stat == status) 
//		return 0;
	
	switch (status) {
	case 0:
		/* stop the run */
		return stop();
	case 1:
		/* start the run */
		return start();
	case 2:
		/* exit the thread */
		return quit();
	}
	return 0;
}

int daq_thread::send_msg(int id, int type, void* msg, int len)
{
	uint32_t h[2];
	h[0] = id;
	h[0] <<= 24;
	h[0] |= len + 8;
	h[1] = thread_id;
	h[1] <<= 24;
	h[1] |= type;
	
	if (rb_msg->get_lock())
		return -E_SYSCALL;
	rb_msg->write1(h, 8);
	rb_msg->write1(msg, len);
	if (rb_msg->rel_lock())
		return -E_SYSCALL;

	return 0;
}

int daq_thread::send_text_mes(const char* msg, int level)
{
	char buf[100];
	int* p_buf = (int*)buf;
	strncpy(buf+4, (const char*)msg, 96);
	p_buf[0] = level;
	buf[95] = 0;
	return send_msg(THREAD_CTL, MSG_TEXT, buf, strlen(buf+4)+1+4);
}

int daq_thread::do_rd_msg()
{
	uint32_t msg_head;
	uint32_t msg[700];
	int cnt, msg_id;

	
	/* first we need to look at the ring buffer to see if the message is
	 * for the current thread (if any)*/	
	if (rb_msg->get_lock())
		return -E_SYSCALL;
	cnt = rb_msg->read(&msg_head, 4, true, false);
	if (cnt == -1) {
		rb_msg->rel_lock();
		return -E_RING_BUF_MSG;
	}
	if (cnt == 4) {
		/* yes, there is a message found */
		msg_id = msg_head>>24;
		if ((msg_id == thread_id) || 
				((msg_id == THREAD_CTL) && is_ctl_thread)) {
			/* the header shows that this message is for the
			 * current thread  */
			int32_t sz = (msg_head & 0xFFFFFF);
			if (sz > 100*7) {
				rb_msg->rel_lock();
				return -E_MSG_TOO_BIG;
			}
			cnt = rb_msg->read(msg, sz, false, false);
			rb_msg->rel_lock();
			if (cnt != sz) {
				return -E_RING_BUF_MSG;
			}
			/* Now we have the message, let's do something... */
			return handle_msg(msg+1);
		}
	}
	
	rb_msg->rel_lock();
	return 0;
}



