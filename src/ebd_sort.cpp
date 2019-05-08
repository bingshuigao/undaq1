#include "ebd_sort.h"
#include <unistd.h>
#include <string.h>
#include <iostream>


ebd_sort::ebd_sort()
{
	int i, j;

	thread_id = 2;
	evt_buf = NULL;
	slot_map = NULL;
	mask_to_slot = NULL;
	for (i = 0; i < MAX_CRATE; i++) {
		for (j = 0; j < MAX_MODULE; j++) {
			rb_map[i][j] = NULL;
		}
	}
	init_fun.push_back(&ebd_sort_init);
}

ebd_sort::~ebd_sort()
{
	if (evt_buf)
		delete evt_buf;
	if (mask_to_slot)
		delete mask_to_slot;

}

int ebd_sort::init_mask_to_slot()
{
	int i;

	/* initialize the mask_to_slot */
	if (MAX_MODULE > 21)
		/* in this case, we need modify the algrithm of mask-to-slot. */
		return -E_GENERIC;
	mask_to_slot = new char[(1L<<(MAX_MODULE-1)) + 1];
	for (i = 0; i < MAX_MODULE; i++) {
		mask_to_slot[1L<<i] = i;
	}
	return 0;
}

int ebd_sort::ebd_sort_init(my_thread* This, initzer* the_initzer) 
{
	int ret;

	ebd_sort* ptr = reinterpret_cast<ebd_sort*>(This);
	ptr->evt_buf = new uint32_t[the_initzer->get_fe_blt_buf_sz()/4+100];
	ptr->evt_buf += 100; /* make space for the header */
	ptr->hz = the_initzer->get_ebd_sort_clock_hz();
	ret = the_initzer->get_conf_vme_mod(ptr->conf);
	RET_IF_NONZERO(ret);
	/* the inialization of the rb_map should be done in the event handler
	 * because presently the rb_data is still empty. */
//	ret = ptr->init_rb_map();
//	RET_IF_NONZERO(ret);
	ret = ptr->init_mask_to_slot();
	RET_IF_NONZERO(ret);

	return 0;
}

int ebd_sort::init_rb_map()
{
	/* debug ... */
//	std::cout<<"printed from sorter "<<rb_data.size()<<std::endl;
	for (auto it = rb_data.begin(); it != rb_data.end(); it++) {
		char* p_udata = (*it)->get_usr_data();
		int crate = p_udata[0];
		int slot = p_udata[1];
		if (crate >= MAX_CRATE)
			return -E_MAX_CRATE;
		if (slot >= MAX_MODULE)
			return -E_MAX_MODULE;
		rb_map[crate][slot] = (*it);
	}
	return 0;
}

int ebd_sort::handle_msg(uint32_t* msg_body)
{
	/* The message type of the current thread are defined as following 
	 * 1 --> run status transition
	 * 2 --> slot map and init_rb_map
	 * */
	uint32_t msg_type = msg_body[0] & 0xFFFFFF;
	switch (msg_type) {
	case 1:
		/* run status transition
		 * */
		return switch_run(msg_body[1]);
	case 2:
		slot_map = (reinterpret_cast<char**>(msg_body+1))[0];
		return init_rb_map();
	default:
		return -E_MSG_TYPE;
	}
}


int ebd_sort::start()
{
	acq_stat = DAQ_RUN;
	n_readout = 0;
	
	/* proporgate the message to the next thread */
	return send_msg(1, 1, &acq_stat, 4);
	
}

int ebd_sort::stop()
{
	acq_stat = DAQ_STOP;

	/* proporgate the stop message to the next thread */
	return send_msg(3, 1, &acq_stat, 4);
}

int ebd_sort::quit()
{
	acq_stat = DAQ_EXIT;
	return 0;
}

int ebd_sort::main_proc()
{
	uint32_t buf_len, evt_len;
	
	/* One should check if the slot_map has been inited. */
	if (!slot_map)
		return 0;

	/* We need to look into the ring buffer of raw data rb_fe. The data
	 * format in the rb_fe is documented in fe_thread.h */
	if (rb_fe->get_lock())
		return -E_SYSCALL;
	buf_len = rb_fe->get_used1();
	if (buf_len == 0) {
		rb_fe->rel_lock();
		/* take a short break before returning ^_^*/
		usleep(120);
		return 0;
	}

	/* the ring buffer is not empty, let's see if it is big enough holding
	 * a complete event. */
	if (rb_fe->read1(&evt_len, 4, true) != 4) {
		rb_fe->rel_lock();
		return -E_RING_BUF_DATA;
	}
	if (buf_len < evt_len) {
		rb_fe->rel_lock();
		usleep(120);
		return 0;
	}
	
	/* now, the ring buffer has a complete event */
	if (rb_fe->read1(evt_buf, evt_len) != evt_len) {
		rb_fe->rel_lock();
		return -E_RING_BUF_DATA;
	}
	rb_fe->rel_lock();
	return handle_evt();
}

int ebd_sort::handle_evt()
{
	int evt_type = evt_buf[2];

	switch (evt_type) {
	case 0:
		return handle_EOR();
	case 1:
		return handle_trig();
	case 2:
		return handle_scal();
	case 3:
		return handle_BOR();
	default:
		return -E_EVT_TYPE;
	}
}

int ebd_sort::handle_EOR()
{
	uint32_t eor[3];

	/* If we see a end of run mark, we place it in each of the individual
	 * ring buffers. */
	eor[0] = 2;
	eor[1] = 1;
	eor[2] = n_readout++;
	for (auto it = rb_data.begin(); it != rb_data.end(); it++) {
		if ((*it)->get_lock())
			return -E_SYSCALL;
		(*it)->write1(eor, 12);
		(*it)->rel_lock();
	}

	return 0;
}

int ebd_sort::handle_trig()
{
	/* pointer to the true_body */
	uint32_t* ptr = evt_buf + 1 + evt_buf[1];
	uint32_t sum, tot_len;

	/* extract the header info */
	t_now = evt_buf[4]; /* high word */
	t_now <<= 32;
	t_now += evt_buf[5]; /* low word */
	crate = evt_buf[3] & 0xFFFF;
	mod_id = evt_buf[3] >> 16; 
	
	/* The format of the data block readout by modules is explained in
	 * modules.h. The data block can be further break into pieces. We need
	 * to handle the data piece by piece.  */
	sum = 0;
	tot_len = evt_buf[0] - (evt_buf[1] + 1) * 4;
	while (sum < tot_len) {
		int ret = handle_data_block(ptr);
		RET_IF_NONZERO(ret);
		sum += ptr[0];
		ptr += ptr[0]/4;
	} 

	return 0;
}

int ebd_sort::handle_data_block(uint32_t* ptr)
{
	/* because the event is readout in BLT mode, even one piece of data may
	 * contain multiple events, we should deal with these events one by one
	 * */
	uint32_t sum, tot_len;
	int ret;

	tot_len = ptr[0];
	sum = 0;
	while (sum < tot_len - 8) {
		int evt_len;
		set_slot_mask(ptr[1]);
		ret = handle_single_evt(ptr + 2 + sum/4, evt_len, 
				(tot_len-8-sum)/4);
		RET_IF_NONZERO(ret);
		sum += evt_len;
	}

	return 0;
}

int ebd_sort::handle_scal()
{
	int sz = evt_buf[0];

	/* we just make a copy of the event in the rb_scal ring buffer */
	if (rb_scal->get_lock())
		return -E_SYSCALL;
	rb_scal->write1(evt_buf, sz);
	rb_scal->rel_lock();

	return 0;
}

int ebd_sort::handle_BOR()
{
	/* The purpose of the begin of run is to reset the global monotonic
	 * time*/
	t_start = evt_buf[4];  /* time stamp high word*/
	t_start <<= 32;
	t_start += evt_buf[5]; /* time stamp low word */
	return 0;
}


void ebd_sort::set_slot_mask(uint32_t mask)
{
	int n = 0;

	/* first, count the number of set bits (from
	 * https://www.geeksforgeeks.org/count-set-bits-in-an-integer/) */
	uint32_t mask_old = mask;
	while (mask) {
		mask &= (mask - 1);
		n++;
	}
	if (n > 1) {
		slot = -1;
		return;
	}

	/* now only 1 bit is set, let's find which bit, to make the process
	 * faster, we trade off the memory space.*/
	slot = mask_to_slot[mask_old];
}

uint64_t ebd_sort::get_mono_ts(uint64_t ts, int n_bit)
{
	uint64_t t_ellips, t_range, n_overf;

	if (n_bit >= 64)
		/* it should never overflow */
		return ts;
	
	t_ellips = (t_now - t_start) * hz;
	t_range = 1L << n_bit;
	if (t_ellips <= ts)
		return ts;
	n_overf = (t_ellips - ts + t_range/2) / t_range;

	/* check the integrety of the ts */
	/* *************/

	return t_range * n_overf + ts;

}

int ebd_sort::handle_single_evt_madc32(uint32_t* evt, int& evt_len, int max_len)
{
	uint32_t sig;
	uint32_t buf[50]; /* big enough to accomadate a madc32 event plus the
			     additional header .*/
	uint64_t ts;
	uint64_t ts_hi;
	uint64_t ts_mono;
	bool has_et = false;
	int idx, evt_len_w;
	
	/* first, we make sure that it has event header */
	sig = evt[0] >> 24;
	if (sig != 0x40)
		/* Opps! Not a header, corrupted data... */
		goto err_data;

	/* Now we try to get event length */
	evt_len_w = ((evt[0] & 0xFFF) + 1); 
	evt_len = evt_len_w * 4;
	if (evt_len_w > max_len)
		goto err_data;

	/* get slot number if necessary  */
	if (slot == -1)
		slot = slot_map[SLOT_MAP_IDX(crate,mod_id,(evt[0]>>16)&0xFF)];

	/* get time stamp */
	idx = evt_len_w - 1;
	sig = evt[idx] >> 30;
	if (sig != 0x3)
		goto err_data;
	ts = evt[idx] & 0x3FFFFFFF;

	/* now try to find the extended ts */
	idx = evt_len_w - 2;
	if (evt[idx] == 0)
		/* this is a filler */
		idx--;
	sig = evt[idx] >> 21;
	if (sig == 0x24)
		has_et = true;
	if (has_et) {
		ts_hi = evt[idx] & 0xFFFF;
		ts += ts_hi;
	}

	/* calculate the monotonic time stamp */
	ts = get_mono_ts(ts, has_et ? 46 : 30);
	if (ts == 0)
		return -E_SYNC_CLOCK;

	/* save the event into the ring buffer */
	return save_evt(buf, evt, evt_len_w, ts);

err_data:
	return -E_DATA_MADC32;
}

int ebd_sort::handle_single_evt_v1190(uint32_t* evt, int& evt_len, int max_len)
{
	/* Here we assume trigger matching mode. We also assume the clock
	 * frequency of 40MHz. If this is not the case, things may go wrong */
	uint32_t sig;
	uint32_t buf[200]; /* should be big enough */
	uint64_t ts, ts_hi, ts_mono;
	int evt_len_wd, geo, i;
	bool has_ettt = false;

	/* first, make sure the first word is the header. */
	sig = evt[0] >> 27;
	if (sig != 0x08)
		goto err_data;

	/* get geo and slot number */
	geo = evt[0] & 0x1F;
	if (slot == -1)
		slot = slot_map[SLOT_MAP_IDX(crate,mod_id,geo)];

	/* find the ettt and trailer */
	for (i = 1; i < max_len; i++) {
		sig = evt[i] >> 27;
		if (sig == 0x11) {
			/* ETTT */
			ts = (evt[i] & 0x7FFFFFF)<<5;
			has_ettt = true;
		}
		else if (sig == 0x10) {
			/* trailer */
			if (!has_ettt) 
				goto err_data;
			ts += evt[i] & 0x1F;
			/* check the word count */
			evt_len_wd = ((evt[i]>>5) & 0xFFFF);
			if (((i+1) != evt_len_wd) || (evt_len_wd >= 196))
				goto err_data;
			evt_len = evt_len_wd * 4;
		}
	}
	if (!has_ettt)
		goto err_data;

	/* calculate the monotonic time stamp */
	if (hz != 40000000)
		return -E_V1190_CLOCK;
	ts = get_mono_ts(ts, 32);
	if (ts == 0)
		return -E_SYNC_CLOCK;

	return save_evt(buf, evt, evt_len_wd, ts);

err_data:
	return -E_DATA_V1190;
}


int ebd_sort::save_evt(uint32_t* buf, uint32_t* evt, int evt_len_wd, uint64_t ts)
{
	int len;
	ring_buf* p_rb;

	/* fill the header and body (see data format in ebd_thread.h */
	buf[0] = 4 + evt_len_wd; /* 4 is the header length */
	buf[1] = 0;
	buf[2] = ts >> 32;
	buf[3] = ts & 0xFFFFFFFF;
	memcpy(buf + 4, evt, evt_len_wd*4);
	

	/* copyt the event and save to the ring buffer */
	len = buf[0] * 4;
	p_rb = rb_map[crate][slot];
	if (!p_rb)
		return -E_RING_BUF_DATA;
	p_rb->wait_buf_free(len);
	if (p_rb->get_lock())
		return -E_SYSCALL;
	p_rb->write1(buf, len);
	p_rb->rel_lock();

	return 0;
}

int ebd_sort::handle_single_evt_v830(uint32_t* evt, int& evt_len, int max_len)
{
	/* it has no timestamp, thus cannot be used as a trigger-type module */
	return -E_VME_GENERIC;
}
