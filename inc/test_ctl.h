/* This class represents a virtual  vme controller for testing purpose
 * By B.Gao Jun. 2020 */

#ifndef TEST_CTL_HHH
#define TEST_CTL_HHH
#include "vme_ctl.h"


class test_ctl : public vme_ctl
{
public:
	test_ctl() {name = "test_ctl";}
	~test_ctl() {};

	/* open the device 
	 * @par a pointer of parameters 
	 * @return 0 if succeed, non-zero error codes are returned if error */
	int open(void* par) {return 0;}

	/* close the device 
	 * @return 0 if succeed, non-zero error codes are returned if error */
	int close() {return 0;}

	/* read from an address. The address modifier and data width are
	 * provided by the member variables hence no need to pass as a
	 * parameter.
	 * @param addr vme bus address
	 * @param data pointer to a buffer where the results will be read to
	 * @return 0 if succeed, non-zero error codes in case of error.*/
	int read(unsigned long addr, void* data) {return 0;}

	/* write to an address . The address modifier and data width are
	 * provided by the member variables hence no need to pass as a
	 * parameter.
	 * @param addr vme bus address
	 * @param data pointer to a buffer containing the data to write.
	 * @return 0 if succeed, non-zero error codes in case of error.*/
	int write(unsigned long addr, void* data) {return 0;}

	/* read from an internal register .
	 * @param reg Internal register identifier.
	 * @param data pointer to a buffer where the results will be read to
	 * @return 0 if succeed, non-zero error codes in case of error.*/
	int read_reg(long reg, void* data) {return 0;}
	
	/* write to an internal register.
	 * @param reg Internal register identifier.
	 * @param data pointer to a buffer containing data to write.
	 * @return 0 if succeed, non-zero error codes in case of error.*/
	int write_reg(long reg, void* data) {return 0;}

	/* (m)blt read from a module.
	 * @param addr vme bus address 
	 * @param buf buffer where the data will be read out to
	 * @param sz_in size of data to read (in bytes)
	 * @param sz_out size of data actually read (in bytes).
	 * @return 0 if succeed, non-zero error codes in case of error. */
	int blt_read(unsigned long addr, void* buf, int sz_in, int* sz_out);
	int mblt_read(unsigned long addr, void* buf, int sz_in, int* sz_out) {return 0;}
	
	/* the out port 1 (the second port) will be used */
	virtual int send_pulse(bool invt) {return 0;}
	/* the out port 0 (the first port) will be used */
	virtual int send_pulse1(bool invt) {return 0;}
	/* the out port 0 (the first port) will be used */
	virtual int send_pulse2(bool invt) {return 0;}
};
#endif

