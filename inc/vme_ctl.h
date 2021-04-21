/* The class vme_ctl is the base class of vme controllers (e.g. v2718)
 * By B.Gao Jul. 2018 */

#ifndef VME_CTL_HHH
#define VME_CTL_HHH

#include "err_code.h"
#include <string>

/* vme address modifers */
#define VME_A16_S       0x2D  /* A16 supervisory access                   */
#define VME_A16_U       0x29  /* A16 non-privileged                       */
#define VME_A16_LCK     0x2C  /* A16 lock command                         */
#define VME_A24_S_BLT   0x3F  /* A24 supervisory block transfer           */
#define VME_A24_S_PGM   0x3E  /* A24 supervisory program access           */
#define VME_A24_S_DATA  0x3D  /* A24 supervisory data access              */
#define VME_A24_S_MBLT  0x3C  /* A24 supervisory 64-bit block trnsfer     */
#define VME_A24_U_BLT   0x3B  /* A24 non-privileged block transfer        */
#define VME_A24_U_PGM   0x3A  /* A24 non-privileged program access        */
#define VME_A24_U_DATA  0x39  /* A24 non-privileged data access           */
#define VME_A24_U_MBLT  0x38  /* A24 non-privileged 64-bit block trnsfer  */
#define VME_A24_LCK     0x32  /* A24 lock command                         */
#define VME_A32_S_BLT   0x0F  /* A32 supervisory block transfer           */
#define VME_A32_S_PGM   0x0E  /* A32 supervisory program access           */
#define VME_A32_S_DATA  0x0D  /* A32 supervisory data access              */
#define VME_A32_S_MBLT  0x0C  /* A32 supervisory 64-bit block trnsfer     */
#define VME_A32_U_BLT   0x0B  /* A32 non-privileged block transfer        */
#define VME_A32_U_PGM   0x0A  /* A32 non-privileged program access        */
#define VME_A32_U_DATA  0x09  /* A32 non-privileged data access           */
#define VME_A32_U_MBLT  0x08  /* A32 non-privileged 64-bit block trnsfer  */
#define VME_A32_LCK     0x05  /* A32 lock command                         */
#define VME_CR_CSR      0x2F  /* CR/CSR space                             */
#define VME_A40_BLT     0x37  /* A40 block transfer (MD32)                */
#define VME_A40_LCK     0x35  /* A40 lock command                         */
#define VME_A40         0x34  /* A40 access                               */
#define VME_A64         0x01  /* A64 single trnsfer access                */
#define VME_A64_BLT     0x03  /* A64 block transfer                       */
#define VME_A64_MBLT    0x00  /* A64 64-bit block transfer                */
#define VME_A64_LCK     0x04  /* A64 lock command                         */
#define VME_A3U_2eVME   0x21  /* 2eVME for 3U modules                     */
#define VME_A6U_2eVME   0x20  /* 2eVME for 6U modules                     */

class vme_ctl
{
public:
	vme_ctl() {};
	~vme_ctl() {};

	/* set/get address modifier */
	void set_am(int am) {this->am = am;}
	int get_am() {return am;}
	
	/* set/get data length */
	void set_dw(int dw) {this->dw = dw;}
	int get_dw() {return dw;}

	void set_crate(int n) {crate = n;}
	int get_crate() {return crate;}

	/* Get the name of the controller */
	std::string get_name() {return name;}

	/* open the device 
	 * @par a pointer of parameters 
	 * @return 0 if succeed, non-zero error codes are returned if error */
	virtual int open(void* par) = 0;
	
	/* close the device 
	 * @return 0 if succeed, non-zero error codes are returned if error */
	virtual int close() = 0;

	/* read from an address (should be overwritten by sub class). The
	 * address modifier and data width are provided by the member variables
	 * hence no need to pass as a parameter.
	 * @param addr vme bus address
	 * @param data pointer to a buffer where the results will be read to
	 * @return 0 if succeed, non-zero error codes in case of error.*/
	virtual int read(unsigned long addr, void* data) = 0;

	/* write to an address (should be overwritten by sub class). The
	 * address modifier and data width are provided by the member variables
	 * hence no need to pass as a parameter.
	 * @param addr vme bus address
	 * @param data pointer to a buffer containing the data to write.
	 * @return 0 if succeed, non-zero error codes in case of error.*/
	virtual int write(unsigned long addr, void* data) = 0;

	/* read from an internal register (should be overwritten by sub class).
	 * @param reg Internal register identifier.
	 * @param data pointer to a buffer where the results will be read to
	 * @return 0 if succeed, non-zero error codes in case of error.*/
	virtual int read_reg(long reg, void* data) = 0;
	
	/* write to an internal register (should be overwritten by sub class).
	 * @param reg Internal register identifier.
	 * @param data pointer to a buffer containing data to write.
	 * @return 0 if succeed, non-zero error codes in case of error.*/
	virtual int write_reg(long reg, void* data) = 0;

	/* (m)blt read from a module.
	 * @param addr vme bus address 
	 * @param buf buffer where the data will be read out to
	 * @param sz_in size of data to read (in bytes)
	 * @param sz_out size of data actually read (in bytes).
	 * @return 0 if succeed, non-zero error codes in case of error. */
	virtual int blt_read(unsigned long addr, void* buf, int sz_in, int*
			sz_out) = 0;
	virtual int mblt_read(unsigned long addr, void* buf, int sz_in, int*
			sz_out) = 0;

	/* send a single pulse.
	 * @param invt direct or inverted output.
	 * @return 0 if succeed, non-zero error codes in case of error. 
	 * */
	virtual int send_pulse(bool invt) = 0;
	virtual int send_pulse1(bool invt) = 0;
	virtual int send_pulse2(bool invt) = 0;

protected:
	int am; /* address modifier */
	int dw; /* data width (dw = 16, or 32, or 64) */
	std::string name; /* name of the controller */
	int crate; /* crate number of the controller */
};
#endif
