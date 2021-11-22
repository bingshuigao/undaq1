/* This class represents the pixie16 controller ,which unlike vme controllers,
 * does not physically exists. We also derive this class from vme_ctl class
 * though it sounds weird. But their lowest level principle of operations are
 * similar: open, read, write, close.
 * By B.Gao Nov. 2021 */

#ifndef PIXIE16_CTL_HHH
#define PIXIE16_CTL_HHH
#include "vme_ctl.h"
#include <stdint.h>
#include "../os_libs/pixie16/pixie16.h"

struct pixie16_ctl_open_par
{
	unsigned short mod_num;       /* total number of modules */
	unsigned short* pxi_slot_map; /* an array containing the slot number of
					 each Pixie-16 module */
	char *ComFPGAConfigFile;      /* name of ComFPGA configuration file */
	char *SPFPGAConfigFile;       /* name of SPFPGA configuration file */ 
	char *TrigFPGAConfigFile;     /* name of trigger FPGA file */         
	char *DSPCodeFile;            /* name of DSP code file */             
	char *DSPParFile;             /* name of DSP parameter file */        
	char *DSPVarFile;             /* name of DSP variable names file */   
};

class pixie16_ctl : public vme_ctl
{
public:
	pixie16_ctl() 
	{
		name = "pixie16_ctl";
		evt_max_sz = DEF_PIXIE16_EVT_MAX;
	}
	~pixie16_ctl() {};

	void set_max_evt_sz(unsigned int sz) {evt_max_sz = sz;}
	/* open the device 
	 * @par a pointer of parameters 
	 * @return 0 if succeed, non-zero error codes are returned if error */
	int open(void* par);

	/* close the device 
	 * @return 0 if succeed, non-zero error codes are returned if error */
	int close();

	/* do not call */
	int read(unsigned long addr, void* data) {return -E_DONT_CALL;}

	/* do not call */
	int write(unsigned long addr, void* data) {return -E_DONT_CALL;}

	/* do not call */
	int read_reg(long reg, void* data) {return -E_DONT_CALL;}
	
	/* do not call */
	int write_reg(long reg, void* data) {return -E_DONT_CALL;}

	/* read event data from pixie16 modules. We use the
	 * Pixie16ReadDataFromExternalFIFO function. Since this function is not
	 * event-aligned readout, we check if the data read out truncated the
	 * last event and, if necessary, do a second read out to restore the
	 * 'event-aligned' feature.
	 * @param addr module number
	 * @param buf buffer where the data will be read out to
	 * @param sz_in size of data to read (in bytes)
	 * @param sz_out size of data actually read (in bytes).
	 * @return 0 if succeed, non-zero error codes in case of error. */
	int blt_read(unsigned long addr, void* buf, int sz_in, int* sz_out);
	int mblt_read(unsigned long addr, void* buf, int sz_in, int* sz_out);
	
	/* the out port 1 (the second port) will be used */
	virtual int send_pulse(bool invt);
	/* the out port 0 (the first port) will be used */
	virtual int send_pulse1(bool invt);
	/* the out port 2 (the third port) will be used */
	virtual int send_pulse2(bool invt);

private:
	/* total number of pixie16 modules installed in the system */
	unsigned short mod_num;
	/* max size of single event (number of 32-bit words) */
	unsigned int evt_max_sz;

private:
	/* check if the events in the buf are event aligned.
	 * @param buf buffer containing the events
	 * @param n_word length of the events in the buffer (number of 32-bit
	 * words)
	 * @return the number of remaining number of words should be readout to
	 * make it event-aligned.
	 * */
	unsigned int check_evt_align(uint32_t* buf, unsigned int n_word);
};
#endif

