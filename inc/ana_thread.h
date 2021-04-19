#ifndef ANA_THREAD_HHH
#define ANA_THREAD_HHH

/* This is the base class of the threads of analyzer. For details, refer
 * to the subclasses.
 *
 * The events in the ring buffers has the same formats as those in the event
 * builder. I repeat here:
 * For scalers:
 *             ______________________________
 *             | header                      |
 *             |_____________________________|
 *             | distance to true body       |
 *             |_____________________________|
 *             | type                        |
 *             |_____________________________|
 *             | module id and crate n       |
 *             |_____________________________|
 *             | timestamp from os (sec) high|
 *             |_____________________________|
 *             | timestamp from os (sec) low |
 *             |_____________________________|
 *             | true body                   |
 *             |_____________________________|
 *
 * The header is of 4-byte length (as an uint32_t) and contains information 
 *  the self-inclusive total length (in bytes)
 * The distance is the number of 4-byte entries from the current position to
 * the real true data. 
 * The module id and crate n are in most significant two bytes and least
 * significant two bytes, respectively.  The type is the type of data:
 *                 0-->End Of Read mark (see comments in ebd_thread.h)
 *                 1-->Trigger type
 *                 2-->Scaler type
 *                 3-->Begin of Run mark (see comments in ebd_sort.h)
 *
 * The module id are defined as:
 * 1 --> madc32;
 * 2 --> v1190;
 * 3 --> v830;
 * 4 --> v977;
 * 5 --> v1740;
 * 6 --> v775
 * 7 --> v775n
 * 8 --> v785
 * 9 --> v785n
 * 10 --> fake_module
 * 11 --> v1751
 * 12 --> mqdc32
 *
 *
 * For trigger events:
 *         _____________________________________________
 *         | inclusive length (number of 4-byte words) |
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
 *     ring buffers except that each fragment is preceded by a word containing
 *     the slot#, crate# and daq# from low to high byte. The timestamp is the
 *     minimum of the timestamps of the fragments.
 *     Format of the individual buffers:
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
 *
 *
 * By B.Gao Jan. 2019 */

#include "daq_thread.h"

class ana_thread : public daq_thread
{
public:
	ana_thread();
	~ana_thread();
private:
	static int ana_thread_init(my_thread* This, initzer* the_initzer);

protected:
	/* This is for data sharing of trigger and scaler event */
	ring_buf* rb_evt;
	ring_buf* rb_scal;

};



#endif
