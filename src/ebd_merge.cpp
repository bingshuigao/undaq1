#include "ebd_merge.h"
#include <unistd.h>
#include <string.h>



ebd_merge::ebd_merge()
{
	thread_id = 3;
	rb_data_ready = false;
	init_fun.push_back(&ebd_merge_init);
}

ebd_merge::~ebd_merge()
{
}


int ebd_merge::ebd_merge_init(my_thread* This, initzer* the_initzer) 
{
	ebd_merge* ptr = reinterpret_cast<ebd_merge*>(This);
	ptr->glom = the_initzer->get_ebd_merge_glom();
	ptr->merged_buf_sz = (the_initzer->get_ebd_merge_merged_buf_sz())/4;
	ptr->merged_buf = new uint32_t[ptr->merged_buf_sz];

	return 0;
}


int ebd_merge::handle_msg(uint32_t* msg_body)
{
	/* The message type of the current thread are defined as following 
	 * 1 --> run status transition
	 * 2 --> rb_data is ready
	 * */
	uint32_t msg_type = msg_body[0] & 0xFFFFFF;
	static int readyness = 0;
	switch (msg_type) {
	case 1:
		/* run status transition
		 * */
		return switch_run(msg_body[1]);
	case 2:
		/* rb_data is ready. */
		readyness++;
		if (readyness == msg_body[1])
			rb_data_ready = true;
		return 0;
	default:
		return -E_MSG_TYPE;
	}
}


int ebd_merge::start()
{
	acq_stat = DAQ_RUN;
	cur_rd = 0;
	for (auto it = rb_data.begin(); it != rb_data.end(); it++) {
		char* p_data = (*it)->get_usr_data();
		uint32_t *p_int32 = reinterpret_cast<uint32_t*>(p_data);
		p_data[2] = 0; /* the can_build marker */
		p_int32[1] = 0; /* the cur_rd_rb (see ebd_thread.h) */
	}

	/* proporgate the message to the next thread */
	return send_msg(2, 1, &acq_stat, 4);
	
}

int ebd_merge::stop()
{
	bool all_clr = false;
	acq_stat = DAQ_STOP;
	int ret;

	/* clear the individual ring buffers */
	while (!all_clr) {
		ret = do_build(true, all_clr);
		RET_IF_NONZERO(ret);
	}

	/* proporgate the stop message to the next thread */
	return send_msg(4, 1, &acq_stat, 4);
}

int ebd_merge::quit()
{
	acq_stat = DAQ_EXIT;
	return 0;
}

int ebd_merge::main_proc()
{
	/* The purpose of this thread is to build events. The first thing is to
	 * see if it is OK to start building (see comments in ebd_thread.h */
	int n_try = 0;
	int ret;
	bool y_n;

	/* we should make sure the rb_data is already initialized before doing
	 * anything. */
	if (!rb_data_ready)
		return 0;
begin:
	ret = can_build(y_n);
	if (y_n) {
		/* good, let's start building */
		bool all_clr;
		return do_build(0, all_clr);
	}
	else {
		/* try again */
		if (n_try++ >= 10)
			return 0;
		usleep(100);
		goto begin;
	}

	return 0;
}

static int set_clr_can_build(ring_buf* rb, char* usr_data, bool& set)
{
	int used = rb->get_used();
	if (used < 0)
		return -E_RING_BUF_DATA;
	if (used < 24) {
		usr_data[2] = 0;
		set = false;
	}
	else {
		usr_data[2] = 1;
		set = true;
	}
	return 0;
}



int ebd_merge::do_build(bool force_merge, bool& all_clr)
{
	uint64_t ts_min = 0xFFFFFFFFFFFFFFFF;
	uint64_t ts_cur;
	uint32_t head[4];
	int ret, tot_len_wd;

	/* if the force_merge flag is set, we need to remove all the EOR if
	 * any. 
	 * NOTE: we should be careful here because some ring buffers might be
	 * empty. */
	all_clr = false;
	if (force_merge) {
		for (auto it = rb_data.begin(); it != rb_data.end(); it++) {
			int len;
start1:
			len = (*it)->get_used();
			if (len == 0)
				continue;
			(*it)->read1(head, 8, true);
			if (head[1] == 1) {
				/* this is an EOR, we should remove, also note
				 * that we don't need to get lock because the
				 * sort thread has stopped by the time when
				 * this thread received the stop command. */
				(*it)->skip(12);
				goto start1;
			}
		}
	}

	/* first, we need to find the minimum of the timestamps */
	for (auto it = rb_data.begin(); it != rb_data.end(); it++) {
		/* here we peek data, so no need to lock */
		bool rm, chg;
start:
		if ((*it)->read1(head, 16, true) == 0)
			/* this ring buffer is empty */
			continue;
		if (head[1] == 1) {
			/* this is an EOR */
			/* first let's see if we can remove the EOR */
			ret = try_rm_EOR(*it, rm, chg);
			RET_IF_NONZERO(ret);
			if (rm) {
				/* the EOR is removed */
				if (chg) 
					/* the cur_rd is changed, we need to
					 * restart the iteration. */
					it = rb_data.begin();
				goto start;
			}
			/* if the EOR is not removed, we ignore this ring
			 * buffer */
			continue;
		}
		/* now it must be a data item */
		ts_cur = head[2];
		ts_cur = (ts_cur << 32) + head[3];
		if (ts_cur < ts_min) 
			ts_min = ts_cur;
	}

	/* second, pull out events from ring buffers whose timestamps are close
	 * enough to the minimum, and then merge them into a complete event. */
	tot_len_wd = 3; /* the header */
//	merged_buf[0] = 4;
	merged_buf[1] = ts_min >> 32;
	merged_buf[2] = ts_min & 0xFFFFFFFF;
	for (auto it = rb_data.begin(); it != rb_data.end(); it++) {
		int len_wd;
		bool set;
		if ((*it)->read1(head, 16, true) == 0)
			/* this ring buffer is empty */
			continue;
		if (head[1] == 1) 
			/* this is an EOR */
			continue;
		/* now it must be a data item */
		ts_cur = head[2];
		ts_cur = (ts_cur << 32) + head[3];
		if (ts_cur - ts_min > glom) 
			/* don't merge, time different too big */
			continue;

		/* now we should pull the event out and merge*/
		len_wd = head[0];
		/* check the length to make sure our buffer is big enough */
		if ((tot_len_wd + len_wd + 1) > merged_buf_sz) 
			return -E_MERGE_BUF_SZ;
		/* we fill the daq#, crate# and slot# before filling the
		 * fragment data */
		merged_buf[tot_len_wd] = (*it)->get_usr_data()[0];
		merged_buf[tot_len_wd] <<= 8;
		merged_buf[tot_len_wd] += (*it)->get_usr_data()[1];
		ret = (*it)->get_lock();
		RET_IF_NONZERO(ret);
		(*it)->read1(merged_buf+tot_len_wd+1, len_wd*4);
		(*it)->rel_lock();
		set_clr_can_build(*it, (*it)->get_usr_data(), set);
		tot_len_wd += len_wd+1;
	}
	merged_buf[0] = tot_len_wd;

	/* if no fragments any more, we should return. */
	if (merged_buf[0] == 3) {
		all_clr = true;
		return 0;
	}

	/* save the event into the merged ring buffer */
	rb_evt->wait_buf_free(tot_len_wd*4);
	if (rb_evt->get_lock())
		return -E_SYSCALL;
	rb_evt->write1(merged_buf, tot_len_wd*4);
	rb_evt->rel_lock();

	return 0;
}



int ebd_merge::try_rm_EOR(ring_buf* rb, bool& rm, bool& chg)
{
	char* p_data = rb->get_usr_data();
	uint32_t *p_int32 = reinterpret_cast<uint32_t*>(p_data);
	uint32_t buf[3];
	bool set;
	int ret;

	if (p_int32[1] - cur_rd >= 1) {
		rm = false;
		chg = false;
		return 0;
	}

	/* now we should remove the EOR */
	if (rb->get_lock())
		return -E_SYSCALL;
	rb->read1(buf, 12);
	rb->rel_lock();
	ret = set_clr_can_build(rb, p_data, set);
	RET_IF_NONZERO(ret);
	p_int32[1]++;
	rm = true;

	/* now we need to re-calculate the cur_rd */
	return cal_cur_rd(chg);
}

int ebd_merge::cal_cur_rd(bool& chg)
{
	uint32_t cur_rd_min = 0xFFFFFFFF;
	for (auto it = rb_data.begin(); it != rb_data.end(); it++) {
		char* p_data = (*it)->get_usr_data();
		uint32_t *p_int32 = reinterpret_cast<uint32_t*>(p_data);
		if (p_int32[1] < cur_rd_min)
			cur_rd_min = p_int32[1];
	}

	if (cur_rd_min > cur_rd) {
		cur_rd = cur_rd_min;
		chg = true;
	}
	else {
		chg = false;
	}

	return 0;
}


int ebd_merge::can_build(bool& y_n)
{
	/* see comments in ebd_thread.h. To recap, all ring buffers should
	 * contain raw data or at least two EOR marks. This makes the length >=
	 * 6(word) */
	y_n = true;
	int ret;
	bool set;
	for (auto it = rb_data.begin(); it != rb_data.end(); it++) {
		char* p_data = (*it)->get_usr_data();
		if (p_data[2])
			/* a previous check has been done, no need to check
			 * agagin. */
			continue;

		ret = set_clr_can_build(*it, p_data, set);
		RET_IF_NONZERO(ret);
		if (!set) {
			y_n = false;
			break;
		}
	}
	return 0;
}

