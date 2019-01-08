#ifndef CTL_THREAD_HHH
#define CTL_THREAD_HHH

/* This class is the base calss for the control threads which communicates with
 * the daq controller. 
 *
 * By B.Gao Jan. 2019*/

#include <string>

class ctl_thread 
{
public:
	ctl_thread();
	~ctl_thread();

protected:

	/* handle the message from the GUI. The parameter msg points to the
	 * 128-byte packet.
	 * Return 0 if succeed, otherwise return error code.*/
	virtual int handle_GUI_msg(unsigned char* msg) = 0;

	/* start, stop and exit the run.
	 * return 0 if succeed, otherwise return error code 
	 * */
	int ctl_start();
	int ctl_stop();
	int ctl_quit();


	/* The main procudure in the main_loop. This procudure is executed only
	 * if the daq is in running status. 
	 * Return 0 if succeed, otherwise return error code. */
	int ctl_main_proc();

	/* Check if there is incoming messages from the GUI (we use select
	 * syscall). Return true if yes, otherwise return false. The error code
	 * is returned in paramter 'stat' if it is not NULL*/
	bool has_GUI_msg(int* err = NULL);

protected:
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
