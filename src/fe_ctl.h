#ifndef FE_CTL_HHH
#define FE_CTL_HHH

/* This class is the thread in frontend which communicates with the daq
 * controller. 
 *
 * By B.Gao Nov. 2018*/

#include "fe_thread.h"
#include <string>

class fe_ctl : public fe_thread
{
public:
	fe_ctl();
	~fe_ctl();

protected:
	/* handle the message. The pointer points to the message body (not
	 * including the message header).
	 * return 0 if succeed, otherwise return error code */
	virtual int handle_msg(uint32_t* msg_body);

	/* handle the message from the GUI. The parameter msg points to the
	 * 128-byte packet.
	 * Return 0 if succeed, otherwise return error code.*/
	int handle_GUI_msg(unsigned char* msg);

	/* start, stop and exit the run.
	 * return 0 if succeed, otherwise return error code 
	 * */
	virtual int start();
	virtual int stop();
	virtual int quit();


	/* The main procudure in the main_loop. This procudure is executed only
	 * if the daq is in running status. 
	 * Return 0 if succeed, otherwise return error code. */
	virtual int main_proc();

	/* Check if there is incoming messages from the GUI (we use select
	 * syscall). Return true if yes, otherwise return false. The error code
	 * is returned in paramter 'stat' if it is not NULL*/
	bool has_GUI_msg(int* err = NULL);

private:
	static int fe_ctl_init(my_thread* This, initzer* the_initzer);
	/* communication socket. */
	int sock;
	unsigned char* sock_buf;
	int sock_buf_sz;
	std::string svr_addr;
	int port;
	/* timeout (us) for the select syscall. We use select to check incoming
	 * packets from the GUI controler. */
	int t_us;

	/* The real_stat is analog to the acq_stat. Because the control thread
	 * is really special, we need it to be *always* in the running status,
	 * unless we want to exit the thread. That means the variable acq_stat
	 * is always 1 (or 2 whan to exit), it is never 0. To represent the
	 * 'real' status of the daq, we use the 'real_stat' variable. */
	int real_stat;
};

#endif
