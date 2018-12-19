#ifndef EBD_SORT_HHH
#define EBD_SORT_HHH

/* This is the sorter thread of the event builder. In this thread, we read the
 * 'raw data' from the ring buffer (feeded by the receiver thread), split the
 * blocks of data into pieces. Each piece of data is one complete event from a
 * vme module. In addition, we also extract the timestamp from the event,
 * calculate the number of overflows and create a global monotonic timestamp
 * (64-bit). Then we save the timestamp together with the event into the
 * corresponding ring buffer. The monotonic clock is reset to the time
 * immediately after the on_start()  of all modules (because the timestamp of
 * each vme module should be reset at each on_start()). 
 * 
 * This is perhaps the most critical (difficult)
 * part of the event builder. 
 * By B.Gao Dec.2018
 * 
 * */

#include "ebd_thread.h"


class ebd_sort : public ebd_thread
{
public:
	ebd_sort();
	~ebd_sort();

private:
	static int ebd_sort_init(my_thread* This, initzer* the_initzer);

	/* implementation of virtual functions declared in base classes. See
	 * comments in base classes for explainations */
	int handle_msg(uint32_t* msg_body);
	int start();
	int stop();
	int quit();
	int main_proc();

	/* process the event. The event is in the evt_buf. 
	 * Return 0 if succeed, otherwise return error code. */
	int handle_evt();

	/* process the event according to its type.
	 * Return 0 if succeed, otherwise return error code.
	 * */
	int handle_EOR();
	int handle_trig();
	int handle_scal();
	int handle_BOR();

	/* process one piece of data pointed by ptr. The piece of data has the
	 * format explained in modules.h. 'One piece' of data means one data
	 * block as:
	 *             __________
	 *             | length  |
	 *             |_________|
	 *             | mask    |
	 *             |_________|
	 *             |data block|
	 *             |_________|

	 * return 0 if succeed, otherwise return error code.
	 * */
	int handle_data_block(uint32_t* ptr);

	/* handle a single event pointed by the pointer, the event
	 * length (bytes) is returned in the evt_len. evt points to the
	 * begining of the raw data directly from vme module with no additional
	 * information. max_len is the max number of 32-bit words from evt (to
	 * prevent segfault).
	 * Return 0 if succeed, otherwise return error code .*/
	int handle_single_evt(uint32_t* evt, int& evt_len, int max_len)
	{
		switch (mod_id) {
		case 1:
			return handle_single_evt_madc32(evt, evt_len, max_len);
		case 2:
			return handle_single_evt_v1190(evt, evt_len, max_len);
		case 3:
			return handle_single_evt_v830(evt, evt_len, max_len);
		default:
			return -E_UNKOWN_MOD;
		}
	}
	int handle_single_evt_madc32(uint32_t* evt, int& evt_len, int max_len);
	int handle_single_evt_v1190(uint32_t* evt, int& evt_len, int max_len);
	int handle_single_evt_v830(uint32_t* evt, int& evt_len, int max_len);


	/* initialize the rb_map. 
	 * return 0 if succeed, otherwise return error code 
	 * */
	int init_rb_map();

	/* set the slot number according to the mask 
	 * if the mask shows only one module, then the slot number should be
	 * easily obtained from the mask. if the mask shown more than one
	 * module, we have to query the slot_map to get the slot number. In the
	 * latter case, we set slot = -1 to indicate that query of the slot_map
	 * is needed. */
	void set_slot_mask(uint32_t mask);

	/* calculate the monotonic time stamp.
	 * @param ts the time stamp of the module event
	 * @param the number of bit of the module event time stamp
	 * @return return the calculated monotonic time stamp. If the time
	 * stamp does not match the system clock time, return 0 indicating
	 * errors. 
	 * */
	uint64_t get_mono_ts(uint64_t ts, int n_bit);

	/* save the event into the right ring buffer. Before
	 * calling this function, the slot and crate numbers should have been
	 * set correctly so that the corresponding ring buffer can be chosen.
	 * @param buf The buffer which is guranteed to be biger than event
	 * length. This buffer is allocated outside of the function.
	 * @param evt. Pointer to the event.
	 * @param evt_len_wd. the event length (32-bit words).
	 * @param ts The monotonic timestamp in the unit of clock ticks.
	 * return 0 if succeed, otherwise return error code 
	 * */
	int save_evt(uint32_t* buf, uint32_t* evt, int evt_len_wd, uint64_t ts);

	/* init the mask_to_slot 
	 * return 0 if succeed, otherwise return error code 
	 * */
	int init_mask_to_slot();



private:
	/* the starting time of the run (seconds) */
	uint64_t t_start;

	/* the current value of the unix time (seconds) */
	uint64_t t_now;

	/* crate number of the data in evt_buf */
	int crate;

	/* module id of the data in evt_buf */
	int mod_id;

	/* The event buffer for a single event (a blt read) */
	uint32_t* evt_buf;

	/* The freqency of the clock */
	uint32_t hz;

	/* current slot number (not to be confused by the slot_n in base class
	 * module.h  */
	int slot;

	/* configurations of all the vme modules (we may need this to
	 * interprete the data from vme modules) */
	std::vector<std::vector<struct conf_vme_mod> > conf;

	/* The map to get slot number from module id. To get slot number, just
	 * say: return slot_map[SLOT_MAP_IDX(crate,module,id)]. The crate is
	 * the crate number, module is the module id (see data structure in
	 * fe_thread.h), id is the GEO. The max crate number is 10, the max
	 * module number is 50, the max id is 32. We can increase these limits
	 * if needed. */
	char* slot_map;

	/* the ring buffer map. similar as the slot_map. to get the pointer of
	 * the ring buffer for the module in crate n and slot m, just say:
	 * return rb_map[n][m] */
	ring_buf* rb_map[MAX_CRATE][MAX_MODULE];

	/* This is used to fast conversion from mask to slot number. Although
	 * this could be a very large array (2MB), only 21 of the elements are
	 * used. So it takes 2MB of virtual space, but only 21 physical pages
	 * (which is 21*4KB physical memory space, or slightly less).*/
	char* mask_to_slot;

	/* see comments in ebd_thread.h */
	int n_readout;

};


#endif