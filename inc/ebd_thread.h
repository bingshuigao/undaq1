#ifndef EBD_THREAD_HHH
#define EBD_THREAD_HHH

/* This is the base class of the threads of Event BuilDer. For details, refer
 * to the subclasses.
 * 
 * How to build the events from data readout from different modules ?
 * We decode and sort the data received from frontend into different ring
 * buffers. We have one ring buffer for each individual vme module. In each
 * ring buffer, the events are sorted in time order. In addition to the raw
 * data readout from vme modules and associated timestamps, we also add a 'end
 * of readout' (EOR) mark at the end of each readout (each trigger in
 * frontend). So the data format in each ring buffer is as following (orgnized
 * as 4-byte words):
 *
 *         ____________________________________________
 *         |      formated event (4-byte words)       |
 *         |__________________________________________|
 *         |      formated event (4-byte words)       |
 *         |__________________________________________|
 *         |      formated event (4-byte words)       |
 *         |__________________________________________|
 *         |      .............                       |
 *         |__________________________________________|
 *         |  ****end of readout mark**************   |
 *         |__________________________________________|
 *         |      formated event (4-byte words)       |
 *         |__________________________________________|
 *         |      formated event (4-byte words)       |
 *         |__________________________________________|
 *         |      formated event (4-byte words)       |
 *         |__________________________________________|
 *         |      .............                       |
 *         |__________________________________________|
 *         |  ****end of readout mark**************   |
 *         |__________________________________________|
 *                            .
 *                            .
 *                            .
 *                            .
 *
 * The formated event has the following structure
 *         _____________________________________________
 *         | inclusive event length (number of 4-byte words |
 *         |___________________________________________|
 *         |  data type = 0                            |
 *         |___________________________________________|
 *         |  monotonic_ts_high (clock tick)           |
 *         |___________________________________________|
 *         |  monotonic_ts_low                         |
 *         |___________________________________________|
 *         | raw event data from vme module            |
 *         |___________________________________________|
 *
 * The data type is used to differentiate the EOR from the normal events:
 * data type == 0 -->normal events
 * data type == 1 -->EOR
 * The monotonic_ts is a monotonic time stamp which is used for event building.
 * It is carefully determined by taking into account the overflow of the module
 * time stamp (because of limited bits).
 *
 * The end of readout mark (EOR) has the following format:
 *         _____________________________________________
 *         | inclusive event length (number of 4-byte words |
 *         |___________________________________________|
 *         |  data type = 1                            |
 *         |___________________________________________|
 *         |  n_readout                                |
 *         |___________________________________________|
 * The n_readout is the n'th readout since the start of the run.  
 *
 * To build events from the individual ring buffers is not trivial. Many things
 * have to be considered. Currently we have the following thoughts:
 *     1) If some of the ring buffers are empty, do not start building and wait
 *     for a short time. This is because we have a EOR mark at the end of each
 *     readout, so it should at least have a EOR rather than being empty. The
 *     reason why it could be empty is that the receiving thread has not yet
 *     received this part of data (or has not yet docoded and filled the ring
 *     buffer), so we should wait.
 *     2) If some of the ring buffers contains only one EOR (no raw data or
 *     anything else), do not start building and wait for a short time.
 *     This is because sometimes data from n'th and n+1'th readout could belong
 *     to the same event. However, data from n'th and n+2'th readout cannot
 *     belong to the same event (or may I'm wrong and they could ?), so if some
 *     ring buffers contains two EOR without any event data, it is OK to start
 *     building. Because the data from next readout (n+2'th readout) cannot
 *     belong to the same event from other ring buffers (with data from the
 *     n'th readout) anyway. So it make no sense to wait for
 *     another readout.
 *     3) When to remove the EOR from ring buffers ? We DO NOT remove it
 *     immediately when it reaches to the top of the ring buffer. In stead, we
 *     maintain a variable 'cur_rd' (current readout number) which equals to
 *     the smallest n_readout of all the ring buffers. Each time when we
 *     encounter a EOR when reading events from the ring buffers, we compare
 *     the value of the n_readout and the value of cur_rd. If n_readout -
 *     cur_rd >= 1, we don't remove it, otherwise we should remove it. For
 *     example, if cur_rd == 5, and we found a EOR with n_readout == 6, we
 *     don't remove the EOR. This is because the following events after the EOR
 *     is from the 7th readout, they cannot belong to the same event as those
 *     from the 5th readout. So it make no sense to start reading data from the
 *     7th readout when we still have some ring buffers containing data from
 *     the 5th readout. Then the question is: when we should increase the value
 *     of cur_rd ? This is tricky and there many implementations, here we just
 *     use a simple but unefficient way. We maintain a cur_rd_rb (current
 *     readout number of the ring buffer) variable for each ring buffer. This
 *     should equal the value of n_readout, we use this variable so that we
 *     don't have to access the ring buffer to get the current readout number
 *     of the ring buffer. We
 *     increase the corresponding variable whenever we remove its EOR. Whenever
 *     we increase any of the cur_rd_rb variables, the cur_rd variable is
 *     re-calculated.
 *     4) In cases of 1) and 2), if we have received a 'stop run' message, we
 *     should starting building events immediately until all the ring buffers
 *     are empty.
 *     5) After considering all complications listed above, the building
 *     process itself is relatively easy: look at the monotonic_ts associated
 *     with events in the ring buffers (if it is an EOR in a ring buffer,
 *     remove it if possible, otherwise just ignore this ring buffer), find the
 *     smallest one, then take all the events out from the ring buffers that
 *     matches the smallest time stamp, and merge them into a single event.
 *
 *  Finally, the merged events has the following format:
 *         _____________________________________________
 *         | Number of fragments                       |
 *         |___________________________________________|
 *         | timestamp high                            | 
 *         |___________________________________________|
 *         | timestamp low                             |
 *         |___________________________________________|
 *         |  fragment 1                               |
 *         |___________________________________________|
 *         |  fragment 2                               |
 *         |___________________________________________|
 *         |  fragment 3                               |
 *         |___________________________________________|
 *         | ...                                       |
 *         |___________________________________________|
 *
 *     Each fragment has exactly the same format as those in the individual
 *     ring buffers. The timestamp is the minimum of the timestamps of the
 *     fragments.
 *
 *  Note: the scaler data are not merged, the final format of the scaler data
 *  are the same as those in frontend (see comments in fe_thread.h).
 * 
 *
 *  The thread ids are defined as following:
 *  1--> The receiver thread
 *  2--> The sorter thread
 *  3--> The builder thread
 *  4--> The sender thread
 *  5--> The controller thread
 *
 * By B.Gao Nov. 2018 */

#include "daq_thread.h"
#include <vector>

class ebd_thread : public daq_thread
{
public:
	ebd_thread();
	~ebd_thread();
private:
	static int ebd_thread_init(my_thread* This, initzer* the_initzer);

protected:
	/* This is for data sharing. The rb_data is a vector of ring buffer
	 * pointers. Because each vme module has a dedicated ring buffer, so we
	 * use vectors to manage the ring buffers. These ring buffers contain
	 * data before built into a complete event. 
	 * The user data of the ring buffers are heavily used. They are defined as following:
	 * p_byte[0] --> crate;
	 * p_byte[1] --> slot;
	 * p_byte[2] --> can_build flag;
	 * p_uint32[1] --> cur_rd_rb;
	 * ...
	 *
	 * */
	std::vector<ring_buf*> rb_data;

	/* This is for data sharing of scaler */
	ring_buf* rb_scal;

	/* This is for data sharing. The rb_evt contains the built events. */
	ring_buf* rb_evt;

	/* This is for data sharing. The rb_fe contains 'raw' data received
	 * from frontend. */
	ring_buf* rb_fe;
};



#endif
