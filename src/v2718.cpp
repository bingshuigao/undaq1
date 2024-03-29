#include "v2718.h"
//#include "CAENVMElib.h"
#include "err_code.h"
#include <iostream>



/* open the device 
 * @par a pointer of parameters 
 * @return 0 if succeed, non-zero error codes are returned if error */
int v2718::open(void* par)
{
	struct v2718_open_par* tmp = (struct v2718_open_par*)par;
	int ret;
	
#ifndef DAQ_XIA
	ret = CAENVME_Init(static_cast<CVBoardTypes>(tmp->bd_type), tmp->link,
			tmp->bd_num, &handle);
#endif
	return error_code(ret);
}


/* close the device 
 * @return 0 if succeed, non-zero error codes are returned if error */
int v2718::close()
{
	int ret;

#ifndef DAQ_XIA
	ret = CAENVME_End(handle);
#endif
	return error_code(ret);
}

/* convert the dw to the data width code consistant with those defined
 * by the caen lib */
CVDataWidth v2718::caen_dw()
{
	switch (dw) {
	case 8:
		return cvD8;
	case 16:
		return cvD16;
	case 32:
		return cvD32;
	case 64:
		return cvD64;
	default:
		return cvD16; /* should not happen */
	}
}

/* convert the error code returned by caen lib functions to standard
 * error codes*/
int v2718::error_code(int code)
{
	switch (code) {
	case cvBusError:
		return -E_VME_BUS;
	case cvSuccess:
		return 0;
	default:
		return -E_VME_COMM; /* a fatal error. */
	}
}

/* read from an address. The address modifier and data width are
 * provided by the member variables hence no need to pass as a
 * parameter.
 * @param addr vme bus address
 * @param data pointer to a buffer where the results will be read to
 * @return 0 if succeed, non-zero error codes in case of error.*/
int v2718::read(unsigned long addr, void* data)
{
	int ret;

#ifndef DAQ_XIA
	ret = CAENVME_ReadCycle(handle, addr, data,
			static_cast<CVAddressModifier>(am), caen_dw());
#endif
	return error_code(ret);
}

/* write to an address . The address modifier and data width are
 * provided by the member variables hence no need to pass as a
 * parameter.
 * @param addr vme bus address
 * @param data pointer to a buffer containing the data to write.
 * @return 0 if succeed, non-zero error codes in case of error.*/
int v2718::write(unsigned long addr, void* data)
{
	int ret;

#ifndef DAQ_XIA
	ret = CAENVME_WriteCycle(handle, addr, data,
			static_cast<CVAddressModifier>(am), caen_dw());
#endif
	return error_code(ret);
}

/* read from an internal register .
 * @param reg Internal register identifier.
 * @param data pointer to a buffer where the results will be read to
 * @return 0 if succeed, non-zero error codes in case of error.*/
int v2718::read_reg(long reg, void* data)
{
	int ret;

#ifndef DAQ_XIA
	ret = CAENVME_ReadRegister(handle, static_cast<CVRegisters>(reg), (unsigned int*) data);
#endif
	return error_code(ret);
}

/* write to an internal register.
 * @param reg Internal register identifier.
 * @param data pointer to a buffer containing data to write.
 * @return 0 if succeed, non-zero error codes in case of error.*/
int v2718::write_reg(long reg, void* data)
{
	int ret;
	unsigned int val = *((unsigned int*)data);

#ifndef DAQ_XIA
	ret = CAENVME_WriteRegister(handle, static_cast<CVRegisters>(reg), val);
#endif
	return error_code(ret);
}

/* (m)blt read from a module.
 * @param addr vme bus address 
 * @param buf buffer where the data will be read out to
 * @param sz_in size of data to read (in bytes)
 * @param sz_out size of data actually read (in bytes).
 * @return 0 if succeed, non-zero error codes in case of error. */
int v2718::blt_read(unsigned long addr, void* buf, int sz_in, int* sz_out)
{
	int ret;

#ifndef DAQ_XIA
	ret = CAENVME_FIFOBLTReadCycle(handle, addr, buf, sz_in,
			static_cast<CVAddressModifier>(am), caen_dw(),
			sz_out);
#endif
	return error_code(ret);
}
int v2718::mblt_read(unsigned long addr, void* buf, int sz_in, int* sz_out)
{
	int ret;

#ifndef DAQ_XIA
	ret = CAENVME_FIFOMBLTReadCycle(handle, addr, buf, sz_in,
			static_cast<CVAddressModifier>(am), sz_out);
#endif
	return error_code(ret);
}

int v2718::send_pulse(bool invt)
{
	int ret, val;
//	std::cout<<"in v2718"<<std::endl;
	val = 0x3f;
	ret = write_reg(0xc, &val);
	RET_IF_NONZERO(ret);
	val = 0x800;
	if (invt)
		ret = write_reg(0x0c, &val);
	else
		ret = write_reg(0x12, &val);
	RET_IF_NONZERO(ret);

	val = 0x80;
	ret = write_reg(0x10, &val);
	RET_IF_NONZERO(ret);
	val = 0x80;
	ret = write_reg(0x0a, &val);
	RET_IF_NONZERO(ret);
	val = 0x80;
	ret = write_reg(0x10, &val);
	RET_IF_NONZERO(ret);

	return 0;
}

int v2718::send_pulse1(bool invt)
{
	int ret, val;
//	std::cout<<"in v2718"<<std::endl;
	val = 0x3f;
	ret = write_reg(0xc, &val);
	RET_IF_NONZERO(ret);
	val = 0x400;
	if (invt)
		ret = write_reg(0x0c, &val);
	else
		ret = write_reg(0x12, &val);
	RET_IF_NONZERO(ret);

	val = 0x40;
	ret = write_reg(0x10, &val);
	RET_IF_NONZERO(ret);
	val = 0x40;
	ret = write_reg(0x0a, &val);
	RET_IF_NONZERO(ret);
	val = 0x40;
	ret = write_reg(0x10, &val);
	RET_IF_NONZERO(ret);

	return 0;
}


int v2718::send_pulse2(bool invt)
{
	int ret, val;
//	std::cout<<"in v2718"<<std::endl;
	val = 0x3f;
	ret = write_reg(0xc, &val);
	RET_IF_NONZERO(ret);
	val = 0x1000;
	if (invt)
		ret = write_reg(0x0c, &val);
	else
		ret = write_reg(0x12, &val);
	RET_IF_NONZERO(ret);

	val = 0x100;
	ret = write_reg(0x10, &val);
	RET_IF_NONZERO(ret);
	val = 0x100;
	ret = write_reg(0x0a, &val);
	RET_IF_NONZERO(ret);
	val = 0x100;
	ret = write_reg(0x10, &val);
	RET_IF_NONZERO(ret);

	return 0;
}
