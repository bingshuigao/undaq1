#include "rd_fe.h"
#include "err_code.h"
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <iostream>


rd_fe::rd_fe()
{
	rb_data = NULL;
	rb_msg = NULL;
	acq_stat = 0;
	the_timer.reset();
	n_byte = 0;
	blt_buf = NULL;
	is_pipe_broken = false;

	init_fun.push_back(&rd_fe_init);
}

rd_fe::~rd_fe()
{
	delete blt_buf;

}


int rd_fe::start()
{
	n_byte = 0;
	acq_stat = 1;
	struct timespec ts1, ts2;
	int ret;


	/* get the time stamp before the on_start() functions */
	clock_gettime(CLOCK_MONOTONIC_COARSE, &ts1);

	for (auto it = mods.begin(); it != mods.end(); it++) {
		ret = (*it)->on_start();
		RET_IF_NONZERO(ret);
	}
	
	/* get the time stamp after the on_start() functions */
	clock_gettime(CLOCK_MONOTONIC_COARSE, &ts2);
	if (thread_id == 1) {
		if (ts2.tv_sec - ts1.tv_sec >= on_start_t_max) 
			/* why we need this? It is related to the global
			 * monotonic clock used by vent builder. If the modules
			 * took too long time for initialization, then the
			 * global monotonic clock is uncertain regarding the
			 * zero time (the starting time). */
			return -E_ON_START_TOUT;
		/* send a begin of run mark (to event builder) */
		do_rd_mods(NULL, 1);
	}

	/* send a begin of run signal (to event builder to reset the global
	 * monotonic clock, see comments in ebd_sort.h ) */


	/* clear ring buffer and proporgate the start message to next thread
	 * (if any)*/
	if (thread_id == 2) {
		rb_data->clear();
		return send_msg(1, 1, &acq_stat, 4);
	}

#ifdef USE_DIRECT_SAVE
	fp_direct_write = fopen("direct_save.dat", "wb");
	if (!fp_direct_write) {
		send_text_mes("cannot open file for direct save!", MSG_LEV_FATAL);
		return -E_SYSCALL;
	}
#endif
	/* lastly, send a pulse for whatever use (e.g. syncronize the
	 * timestamps)*/
	if (thread_id == 1) {
		ret = the_ctl->send_pulse(false);
		RET_IF_NONZERO(ret);
	}

	return 0;
}

int rd_fe::stop()
{
	int ret;
	
	/* first, send a pulse for whatever use (e.g. veto all subsequent
	 * triggers) */
	if (thread_id == 1) {
		ret = the_ctl->send_pulse1(false);
		RET_IF_NONZERO(ret);
	}

	/* then read the remaining data in the readout buffer */
	usleep(50000);
	ret = try_rd_fe(true);
	RET_IF_NONZERO(ret);

	/* This is tricky: in case of reading scaler data, which is done by
	 * sending messages to the rd_trig thread, we need to to make sure that
	 * the reading is finished before proporgating the stop message to the
	 * next thread. So we insert a 'dummy' message in the que, that means
	 * when the rd_trig thread sees the 'dummy' message, it must have
	 * finished all the reading scaler data, then the stop message in the
	 * que can be processed by other threads. */
	if (thread_id == 2) {
		ret = send_msg(1, 3, NULL, 0);
		RET_IF_NONZERO(ret);
	}
	
	acq_stat = 0;
	for (auto it = mods.begin(); it != mods.end(); it++) {
		ret = (*it)->on_stop();
		RET_IF_NONZERO(ret);
	}

	/* proporgate the stop message to next thread (if any)*/
	if (thread_id == 1) {
#ifdef USE_DIRECT_SAVE
		fclose(fp_direct_write);
#endif
		return send_msg(2, 1, &acq_stat, 4);
	}
	else if (thread_id == 2)
		return send_msg(3, 1, &acq_stat, 4);

	return 0;
}

int rd_fe::quit()
{
	acq_stat = 2;
	return 0;
}

int rd_fe::rd_fe_init(my_thread* ptr, initzer* the_initzer)
{
	int ret;
	char type;
	rd_fe* This = static_cast<rd_fe*>(ptr);


	/* initialize the modules which will be readout at each trigger (or
	 * each timeout of scaler-type modules) */
	if (This->thread_id == 1)
		type = 'T';
	else
		type = 'S';
	ret = the_initzer->get_modules(type, This->mods);
	/* debug...*/
//	std::cout<<"number of moduless "<<This->mods.size()<<std::endl;
	RET_IF_NONZERO(ret);
	This->rbs_ebd = the_initzer->get_rbs_ebd(ret);
	RET_IF_NONZERO(ret);
	/* debug */
//	std::cout<<This->rbs_ebd.size()<<std::endl;

	/* the blt_buf */
	This->blt_buf_sz = the_initzer->get_fe_blt_buf_sz();
	This->blt_buf = new unsigned char[This->blt_buf_sz];

	/* The on_start_t_max */
	This->on_start_t_max = the_initzer->get_fe_on_start_t_max();

	return This->my_init(the_initzer);
}


int rd_fe::send_warning(int id, const char* msg)
{
	int len = strlen(msg) + 8;
	unsigned char* buf = new unsigned char[len];
	uint32_t* p_int = reinterpret_cast<uint32_t*>(buf);
	int ret;

	p_int[0] = id;
	p_int[0] = (p_int[0] << 24) + len;
	p_int[1] = thread_id;
	p_int[1] = (p_int[1] << 24) + 3;
	memcpy(buf + 8, msg, len-8);
	ret = rb_msg->write(buf, len);

	delete buf;
	if (ret == -1)
		return -E_RING_BUF_MSG;
	return 0;
}

/* Sorry that there are many goto's in this function. Don't be fooled by text
 * books saying nerver use goto in c/c++. If you dig into linux kernel source
 * code, you'll see many goto's. Do you think the linux kernel developers are
 * amateur?*/
int rd_fe::do_rd_mods(modules* the_mods, int m_type)
{
	int ret, sz_in, sz_out;
	int head_sz;
	bool discard;
	uint32_t* p_head = reinterpret_cast<uint32_t*>(blt_buf);

	if (!the_mods) {
		sz_out = 0;
		goto begin3;
	}

	/* The readout of the vme modules is made extreamly simple thanks to
	 * the implementation of the class modules.
	 * The items in the ring buffer*/
	discard = false;
	ret = comp_head(NULL, NULL, head_sz, 0); /* get header size */
	RET_IF_NONZERO(ret);
begin:
	/* read event buffer from modules */
	sz_in = blt_buf_sz - head_sz;
	ret = the_mods->read_evtbuf(blt_buf + head_sz, sz_in, &sz_out);
	RET_IF_NONZERO(ret);
	if (sz_out == 0)
		/* no data from this modules, no further action needed. */
		return 0;
	n_byte += sz_out;
	if ((sz_out+100) >= sz_in) {
		/* the blt_buf_sz is too small */
		ret = send_text_mes("blt_buf_sz is too small, need to increase!", 
				MSG_LEV_WARN);
		RET_IF_NONZERO(ret);
		discard = true;
		/* goto begin because we want to clear the buffer of the module
		 * (by reading all data out)*/
		goto begin;
	}
	if (discard)
		goto begin;
#ifdef USE_DIRECT_SAVE
	fwrite(blt_buf+head_sz, 1, sz_out, fp_direct_write);
#endif

	/* if the pipe line for the data stream is broken, we stop writting
	 * into it */
	if (is_pipe_broken) 
		return 0;

	/* compose the header */
begin3:
	ret = comp_head(p_head, the_mods, head_sz, sz_out, m_type);
	RET_IF_NONZERO(ret);

begin2:
	/* save the event into the ring buffer */
	if (rb_data->get_lock())
		return -E_SYSCALL;
	if (rb_data->get_free_nolock() < p_head[0]) {
		/* Note: we cannot use the rb_data->wait_buf_free()
		 * because there are two producers for the ring buffer
		 * (trig and scal)*/
		rb_data->rel_lock();
		usleep(100);
		goto begin2;
	}
	/* now we have got enough space in the ring buffer. */
	rb_data->write(blt_buf, p_head[0], false);
	rb_data->rel_lock();
	return 0;
}

int rd_fe::comp_head(uint32_t* p_head, modules* the_mods, 
		int& head_sz, int body_sz, int m_type)
{
	struct timespec ts;
	int the_head_sz = 6*4;
	char type;
	int head_type;

	if (!p_head) {
		head_sz = the_head_sz;
		return 0;
	}

	
	if (clock_gettime(CLOCK_MONOTONIC_COARSE, &ts))
		return -E_SYSCALL;
	if (the_mods) {
		type = the_mods->get_1st_mod()->get_type();
		if (type == 'T')
			/* trigger type */
			head_type = 1;
		else
			head_type = 2;
	}
	else {
		if (m_type == 0)
			head_type = 0;
		if (m_type == 1)
			head_type = 3;
	}

	p_head[0] = body_sz + the_head_sz;
	p_head[1] = the_head_sz/4 - 1;
	p_head[2] = head_type;
	if (the_mods)
		p_head[3] = (the_mods->get_1st_mod()->get_mod_id() << 16) +
			the_mods->get_crate();
	else
		p_head[3] = 0;
	p_head[4] = ts.tv_sec >> 32;
	p_head[5] = ts.tv_sec & 0xFFFFFFFF;
	
	return 0;
}
