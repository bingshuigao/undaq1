 /* This is the base class for the rd_trig and rd_scal classes. For details,
  * refer to the source files of the sub classes.
 * By B.Gao Oct. 2018 */

#ifndef RD_FE_HHH
#define RD_FE_HHH

#include "initzer.h"
#include <vector>
#include "module.h"
#include <pthread.h>
#include "my_timer.h"
#include "fe_thread.h"

class rd_fe : public fe_thread
{
public:
	rd_fe();
	~rd_fe();

protected:
	/* The main procudure in the main_loop. This procudure is executed only
	 * if the daq is in running status. 
	 * Return 0 if succeed, otherwise return error code. */
	virtual int main_proc() {return try_rd_fe();}


	/* partially initialize. Initialize only the special variables, the
	 * common initialization are done in the function init(). should be
	 * called by the implementations of the init function in the
	 * baseclasses. 
	 * Return 0 if succeed, return error code in case of error */
	virtual int my_init(initzer* the_initzer) = 0;


	/* start, stop and exit the run.
	 * return 0 if succeed, otherwise return error code 
	 * */
	virtual int start();
	virtual int stop();
	virtual int quit();


	/* send a warning message into the message ring buffer 
	 * @param id Id of the receiver,
	 * @param msg message to be sent 
	 * return 0 if succeed, otherwise return error code 
	 * */
	int send_warning(int id, const char* msg);

	/* try to read the frontend, it is called in the mainloop if the daq is
	 * in the running status.
	 * In this function, we should first check if we need to readout the
	 * data. (for trigger-type modules, check trigger condition, for
	 * scaler-type module, check timer). If we do need to readout data from
	 * modules, then we read and handle the data.
	 * Return 0 if succeed, otherwise return error
	 * code. 
	 * @param force_rd: if true, read the modules anyway; if false, read
	 * the modules only if neccessary (e.g. there is trigger) */
	virtual int try_rd_fe(bool force_rd = false) = 0;
	
	/* Read the modules pointed by the_mods and save data to ring buffer. 
	 * return 0 if succeed, otherwise return error code.
	 * @param the_mods the modules to be readout if the_mods == NULL, donot
	 * read modules, just generate a EOR mark (or other marks) (see
	 * comments in ebd_thread.h)
	 * @param m_type. The type of mark to be generated:
	 *     0--> EOR (End of readout, added at each readout)
	 *     1--> BOR (Begin of run, added at the beining of a run)
	 * */
	int do_rd_mods(modules* the_mods, int m_type = 0);


	/* compose the header for the events readout from the modules the_mods.
	 * It can also be used to get the header size (bytes) if p_head ==
	 * NULL.
	 * @param p_head pointer to the buffer 
	 * @param the_mods pointer to the modules where the data is from. If ==
	 * NULL, this header is not associated with any modules, but represents
	 * a End Of Read mark(EOR) or other marks.
	 * @param head_sz Ignored if p_head != NULL, otherwise return the head
	 * size.
	 * @param body_sz the size of body data.
	 * @param m_type. The type of mark to be generated:
	 * return 0 if succeed, otherwise return error code.
	 * */
	int comp_head(uint32_t* p_head, modules* the_mods, 
			int& head_sz, int body_sz, int m_type = 0);

private:
	static int rd_fe_init(my_thread* This, initzer* the_initzer);

protected:

	/* the trigger-type modules which should be readout at each trigger */
	std::vector<modules*> mods;

	/* The number of bytes readout in total since start */
	uint64_t n_byte;

	unsigned char* blt_buf;
	int blt_buf_sz;

	/* The max number of seconds allowed to spend on the on_start()
	 * functions */
	int on_start_t_max;

	/* the vme crate controller */
	vme_ctl* the_ctl;

};



#endif 
