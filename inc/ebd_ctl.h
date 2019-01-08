#ifndef EBD_CTL_HHH
#define EBD_CTL_HHH

/* This class is the thread in event builder which communicates with the daq
 * controller. 
 *
 * By B.Gao Jan. 2019*/

#include "ebd_thread.h"
#include "ctl_thread.h"
#include <string>

class ebd_ctl : public ebd_thread, public ctl_thread
{
public:
	ebd_ctl();
	~ebd_ctl();

protected:
	/* handle the message. The pointer points to the message body (not
	 * including the message header).
	 * return 0 if succeed, otherwise return error code */
	virtual int handle_msg(uint32_t* msg_body);

	/* handle the message from the GUI. The parameter msg points to the
	 * 128-byte packet.
	 * Return 0 if succeed, otherwise return error code.*/
	virtual int handle_GUI_msg(unsigned char* msg);

	/* start, stop and exit the run.
	 * return 0 if succeed, otherwise return error code 
	 * */
	virtual int start();
	virtual int stop();
	virtual int quit();


	/* The main procudure in the main_loop. This procudure is executed only
	 * if the daq is in running status. 
	 * Return 0 if succeed, otherwise return error code. */
	virtual int main_proc() {return ctl_main_proc();}


private:
	static int ebd_ctl_init(my_thread* This, initzer* the_initzer);
};

#endif
