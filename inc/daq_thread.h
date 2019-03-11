 /* This is the base class for the working threads of frontend or event
  * builder. For details, refer to the source files of the sub classes.
  *
 * The communication between threads are done using the ring buffers. Not only
 * the data transfer, but also the messages like 'start', 'stop', 'quit' and
 * error messages. Because the ring buffer already has its mutex, we don't have
 * to worry about that part anymore. To use ring buffer to share messages, we
 * use the following mechinism. Since there are many threads using the same
 * ring buffer to share messages, anyone can write to the ring buffer. However,
 * only the designated 'receiver' of the message could remove the message from
 * the ring buffer, other thread can 'peek' messages, and if the message is not
 * to them, they won't remove it from the ring buffer. In order to be able to
 * identify the 'receiver', we use the following format for each message in the
 * ring buffer:
 *              _________________
 *              |    header     |
 *              |_______________|
 *              |    body       |
 *              |_______________|
 * The header is a 4-byte integer which identifies the receiver (in the most
 * significant byte) and the length (bytes) of the message (in the lower 3
 * bytes, including the header). The body has a sub-header (4-byte integer)
 * which identifies the the sender (most significant byte) and the type of
 * message ( the other three bytes):
 *              ________________
 *              |    sub header |
 *              |_______________|
 *              |    true body  |
 *              |_______________|
 *
 * Here is a summary of the message types:
 * Frontend:
 * rd_trig: 1--> status transition. The new status is contained in the first
 *               4-byte word of the true body.
 *          2--> To be defined
 *          3--> A warning message. The message is a null-terminated string
 *               which is contained in the true body.
 * rd_scal: 1--> The same as rd_trig
 *          2--> To be defined
 *          3--> The same as rd_trig
 * fe_sender: 1--> The same as rd_trig
 *            2--> To be defined
 *            3--> The same as rd_trig
 * fe_ctl: 1--> The same as rd_trig
 *         2--> To be defined
 *         3--> The same as rd_trig
 *
 * Event builder:
 * ebd_recv: 1--> The same as rd_trig
 *           2--> To be defined
 *           3--> The same as rd_trig
 * ebd_sort:
 *   1--> The same as rd_trig
 *   2--> The slot_map. The pointer of the slot_map is contained in the first
 *                      8-byte word of the true body.
 *   3--> The same as rd_trig
 * ebd_merge:
 *   1--> The same as rd_trig
 *   2--> To be defined
 *   3--> The same as rd_trig
 * ebd_sender:
 *   1--> The same as rd_trig
 *   2--> the analyzer needs re-connect. The true body is meaningless
 *   3--> The same as rd_trig
 * ebd_ctl:
 *   1--> The same as rd_trig
 *   2--> To be defined
 *   3--> The same as rd_trig
 *
 *
 * Logger:
 * log_recv: 
 *   1--> The same as rd_trig
 *   2--> To be defined
 *   3--> The same as rd_trig
 * log_save:
 *   1--> Run status transition. The new status is in the first 4-byte of the
 *         true body. If the new status is DAQ_RUN, then the second 4-byte word
 *         of the true body contains the run number and the
 *         third 4-byte word the if_save flag.
 *   2--> To be defined
 *   3--> The same as rd_trig
 * log_ctl:
 *   1--> The same as rd_trig
 *   2--> To be defined
 *   3--> The same as rd_trig
 * 
 * Analyzer:
 * ana_recv:
 *   1--> The same as rd_trig
 *   2--> To be defined
 *   3--> The same as rd_trig
 * ana_main:
 *   1--> The same as rd_trig
 *   2--> To be defined
 *   3--> The same as rd_trig
 * ana_roody_svr:
 *   1--> The same as rd_trig
 *   2--> Need to establish the connection with roody (The true body is
 *         meaningless)
 *   3--> The same as rd_trig
 * ana_ctl:
 *   1--> The same as rd_trig
 *   2--> To be defined
 *   3--> The same as rd_trig
 * 
 *   
 *
 *
 * By B.Gao Oct. 2018 */

#ifndef DAQ_THREAD_HHH
#define DAQ_THREAD_HHH

#include "initzer.h"
#include <vector>
#include "module.h"
#include <pthread.h>
#include "my_timer.h"
#include "my_thread.h"

class daq_thread : public my_thread
{
public:
	daq_thread();
	~daq_thread();

protected:

	/* read messages from rb_msg and take actions accordingly, e.g.
	 * transition of run status, return 0 if succeed, otherwise return
	 * error code.  */
	int rd_msg()
	{
		/* read message every 10 ms*/
		if (the_timer.time_out(10000)) {
			the_timer.reset();
			return do_rd_msg();
		}
		return 0;
	}

	/* read message (from the ring buffer) and take actions accordingly.
	 * return 0 if succeed, otherwise return error code */
	int do_rd_msg();

	/* send a message into the message ring buffer. 
	 * @param id. The thread id of the receiver.
	 * @param type. Type of the message.
	 * @param msg. True body of the message (not including the sub header)
	 * @param len. Length (in bytes) of the msg.
	 * @return Return zero if succeed, otherwise return error code.*/
	int send_msg(int id, int type, void* msg, int len);


	/* handle the message. The pointer points to the message body (not
	 * including the message header).
	 * return 0 if succeed, otherwise return error code */
	virtual int handle_msg(uint32_t* msg_body) = 0;

	/* start, stop and exit the run.
	 * return 0 if succeed, otherwise return error code 
	 * */
	virtual int start() = 0;
	virtual int stop() = 0;
	virtual int quit() = 0;


	/* switch the run status, the new status is indicated by the parameter
	 * status. 
	 * return 0 if succeed, otherwise return error code 
	 * */
	int switch_run(uint32_t status);

	/* the entry point of the thread */
	void* main_loop(void* arg);

	/* The main procudure in the main_loop. This procudure is executed only
	 * if the daq is in running status. 
	 * Return 0 if succeed, otherwise return error code. */
	virtual int main_proc() = 0;


protected:
	/* this ring buffer is for message sharing (The initialization is done
	 * by subclasses.  */
	ring_buf* rb_msg;

	/* running status, 0: stopped, 1: running, 2: exit, 3: in transition.
	 * The ordering of the start/stop of the threads matters, we would like
	 * this order:
	 *     for frontend: 
	 *     1) To start 4->3->2->1; 
	 *     2) To stop  1->2->3->4. 
	 *     So the start or stop is not in parrelle, but sequencialy like a
	 *     daisy chain. So in order to start a run, first we (thread 4)
	 *     send a start message to thread 4, after processing the message,
	 *     proporgate the starting message to 3, then to 2 then to 1. To
	 *     stop a run, the proporgation goes in the reversed direction. To
	 *     quit the run, it goes in parrelle, because we designed the daq
	 *     such that the user has to stop the run first, then quit the run.
	 *     It doesn't matter which exits first since they are all in
	 *     stopped status.
	 *     
	 *     for event builder:
	 *     1) To start 5->4->3->2->1
	 *     2) To stop  1->2->3->4->5
	 *     
	 *     for logger
	 *     1) To start 3->2->1
	 *     2) To stop  1->2->3
	 *     */
	int acq_stat;


	/* the timer used to read message */
	my_timer the_timer;


	/* thread id (to identify the receiver/sender of messages, it should be
	 * initialized in the constructor of subclasses*/
	int thread_id;

};



#endif 
