#ifndef EBD_MERGE_HHH
#define EBD_MERGE_HHH

/* This is the merger thread of the event builder. In this thread, we read the
 * fragments of events from the individual ring buffers of each vme module and
 * merge these fragments into one event, based on their timestamps. 
 * The merging is tricky, see comments in ebd_thread.h.
 *
 * By B.Gao Dec.2018
 * 
 * */

#include "ebd_thread.h"
#include <vector>


class ebd_merge : public ebd_thread
{
public:
	ebd_merge();
	~ebd_merge();

private:
	static int ebd_merge_init(my_thread* This, initzer* the_initzer);

	/* implementation of virtual functions declared in base classes. See
	 * comments in base classes for explainations */
	int handle_msg(uint32_t* msg_body);
	int start();
	int stop();
	int quit();
	int main_proc();

	/* see if it is OK to start the building process. (see comments in
	 * ebd_thread.h) 
	 * if yes, y_n = true, if no y_n = false;
	 * return 0 if succeed, otherwise return error code.
	 * */
	int can_build(bool& y_n);

	/* read events from each individual ring buffers and merge them into a
	 * complete event. The following things should be done:
	 *   1) read (peek) events (if any) from individual ring buffers,
	 *   compare their timestamps and find the smallest timestamp. If an
	 *   EOR is seen when peeking events, remove it if possible. If an EOR
	 *   is removed: 
	 *     i) increase the value of cur_rd_rb and re-calculate the value of
	 *     cur_rd. If the value of cur_rd is changed, restart the iteration
	 *     over all the ring buffers. 
	 *     ii) clear the can_build marker if appropriate. 
	 *
	 *   2) find all the events with timestamp close enough to the smallest
	 *   timestamp, pull these events out from the ring buffers and merge
	 *   them into a complete event and save it into the ring buffer
	 *   'rb_evt'. If an EOR is seen, skip the ring buffer. If anything is
	 *   read out from the ring buffer, clear the can_build marker if
	 *   appropriate.
	 *
	 * The parameter force_merge indicates if we should remove the EOR mark
	 * no matter what. This is only used when a 'STOP' command is
	 * received and we should clear all the ring buffers.
	 * The parameter all_clr is set if all the individual ring buffers are
	 * empty.
	 *
	 * return 0 if succeed, otherwise return error code.
	 * */
	int do_build(bool force_merge, bool& all_clr);

	/* try to remove the EOR marker (based on the values of cur_rd and
	 * cur_rd_rb).  If an EOR is removed: 
	 *     i) increase the value of cur_rd_rb and re-calculate the value of
	 *     cur_rd. 
	 *     ii) clear the can_build marker if appropriate. 
	 * @param rb pointer to the ring buffer to be operated on.
	 * @param rm set to true if the EOR is removed.
	 * @param chg set to true if the cur_rd is changed.
	 *
	 * return 0 if succeed, otherwise return error code.
	 * */
	int try_rm_EOR(ring_buf* rb, bool& rm, bool& chg);

	/* calculate the value of cur_rd. If the value of cur_rd as a result of
	 * the calculation, chg is set to true. 
	 *
	 * return 0 if succeed, otherwise return error code.
	 * */
	int cal_cur_rd(bool& chg);

	/* get time stamp for the event count (wall time)*/
	void get_evt_ts();

private:
	/* for the meaning of this variables, refer to "ebd_thread.h" */
	uint32_t cur_rd;

	/* evt count and time stamp:
	 * evt_cnt_ts[0] --> high word of ts 
	 * evt_cnt_ts[1] --> low word of ts
	 * evt_cnt_ts[2] --> evt count
	 * */
	uint32_t evt_cnt_ts[3];

	/* this is the time interval (clock ticks) below which events should be
	 * merged */
	uint32_t glom;

	/* the buffer for the merged events */
	uint32_t* merged_buf;
	
	/* the size of the merged_buf (4-byte word) */
	uint32_t merged_buf_sz;

	/* check if the rb_data is ready */
	bool rb_data_ready;


};


#endif
