 /* This is the base class for the working threads of frontend 
  * builder. For details, refer to the source files of the sub classes.
  *
  *
 * The receiver/sender are identified as following:
 * 1 --> The read trigger thread
 * 2 --> The read scaler thread
 * 3 --> The thread to send data out via sockets
 * 4 --> the thread which communicates with the controller.
 *
 * For the read trigger thread, the type of message is defined as :
 * 1 --> run status transition
 * 2 --> 
 *
 * For the thread 4, the type of message is defined as:
 * 1 --> warning message, the true body is a string
 * 2 -->
 *
 * For the items in the data ring buffer, we take the following format
 * (each entry is organised as 4-byte uint32_t):
 *              ________________
 *              |   header     |
 *              |______________|
 *              |   body       |
 *              |______________|
 * The header is of 4-byte length (as an uint32_t) and contains information 
 *  the self-inclusive total length (in bytes)
 * The body contains a sub header and the true data. The first 4 bytes is the
 * distance (number of 4-byte entries)  from the sub header to the real true
 * data. The current version has the following information in the sub header:
 *             _______________________________
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
 *             The module id and crate n are in most significant two bytes and
 *             least significant two bytes, respectively.
 *             The type is the type of data:
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
 *
 *
  * By B.Gao Oct. 2018 */

#ifndef FE_THREAD_HHH
#define FE_THREAD_HHH

#include "initzer.h"
#include <vector>
#include "module.h"
#include <pthread.h>
#include "my_timer.h"
#include "daq_thread.h"

class fe_thread : public daq_thread
{
public:
	fe_thread();
	~fe_thread();

private:
	static int fe_thread_init(my_thread* This, initzer* the_initzer);

protected:
	ring_buf* rb_data;
	std::vector<struct mod_rb_par> rbs_ebd; 
};



#endif 
