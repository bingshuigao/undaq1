/* This class represents the vme controller v2718 
 * By B.Gao Jul. 2018 */

#ifndef V2718_HHH
#define V2718_HHH
#include "vme_ctl.h"
#include "../os_libs/CAENVMELib-2.50/include/CAENVMEtypes.h"
#include "../os_libs/CAENVMELib-2.50/include/CAENVMElib.h"

struct v2718_open_par
{
	int bd_type;
	short link;
	short bd_num;
};

class v2718 : public vme_ctl
{
public:
	v2718() {name = "v2718";}
	~v2718() {};

	/* open the device 
	 * @par a pointer of parameters 
	 * @return 0 if succeed, non-zero error codes are returned if error */
	int open(void* par);

	/* close the device 
	 * @return 0 if succeed, non-zero error codes are returned if error */
	int close();

	/* read from an address. The address modifier and data width are
	 * provided by the member variables hence no need to pass as a
	 * parameter.
	 * @param addr vme bus address
	 * @param data pointer to a buffer where the results will be read to
	 * @return 0 if succeed, non-zero error codes in case of error.*/
	int read(unsigned long addr, void* data);

	/* write to an address . The address modifier and data width are
	 * provided by the member variables hence no need to pass as a
	 * parameter.
	 * @param addr vme bus address
	 * @param data pointer to a buffer containing the data to write.
	 * @return 0 if succeed, non-zero error codes in case of error.*/
	int write(unsigned long addr, void* data);

	/* read from an internal register .
	 * @param reg Internal register identifier.
	 * @param data pointer to a buffer where the results will be read to
	 * @return 0 if succeed, non-zero error codes in case of error.*/
	int read_reg(long reg, void* data);
	
	/* write to an internal register.
	 * @param reg Internal register identifier.
	 * @param data pointer to a buffer containing data to write.
	 * @return 0 if succeed, non-zero error codes in case of error.*/
	int write_reg(long reg, void* data);

	/* (m)blt read from a module.
	 * @param addr vme bus address 
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
	/* convert the dw to the data width code consistant with those defined
	 * by the caen lib */
	CVDataWidth caen_dw();

	/* convert the error code returned by caen lib functions to standard
	 * error codes*/
	int error_code(int code);
private:
	int32_t handle; /* The handle that identifies the device */
};
#endif

