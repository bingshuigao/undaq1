/* This class has a list of vme modules (of the same type) and manages all
 * these modules. The purpose of this class is to make the readout of multiple
 * modules easier. 
 * By B.Gao Aug. 2018 */

#ifndef MODULES_HHH
#define MODULES_HHH


#include <vector>
#include "module.h"

/* The number of modules cannot excceed 21 in a vme crate, for simplicity, we
 * don't use std::vector, we just use a array with fixed size of MAX_MODULE  */
#define MAX_MODULE 21 

class modules
{
public:
	modules();
	~modules(){};

	/* Add an module object and let the modules to manage it. The module
	 * object should have been properly initialized. The position of the
	 * module object in the array 'mods' is determined by its slot number.
	 * Another hidden job of this function is that at the end of the
	 * fuction, the update_read_list method is called . 
	 * Note: the modules class is smart enough to figure out the fasted
	 * possible way for each readout of all modules (mblt + cmblt).
	 * however, it is not smart enough to figure out the address modifiers
	 * to be used in case of mblt and cmblt. so they should be provided
	 * here.
	 * @param am_mblt Address modifier to be used in mblt readout
	 * @param am_cmblt Address modifier to be used in cmblt readout
	 * @return 0 if succeed, non-zero error code if error.
	 * */
	int add_mod(module* mod, int am_mblt, int am_cmblt);

	/* read the event buffers of all the modules. 
	 * @param buf Buffer where the events will be saved.
	 * @param sz_in Maximum requested size to read (bytes)
	 * @param sz_out Actual size of data read (bytes)
	 * @return 0 if succeed, non-zero error code if error */
	int read_evtbuf(void* buf, int sz_in, int* sz_out);

	/* Readout reset, should be called after each readout.
	 * @return 0 if succeed, non-zero error code if error */
	int readout_reset();

private:

	/* Update the list of parameters used for reading out. These values
	 * are determined by the settings of the madc32 such
	 * that the fastest way is chosen when reading event buffer.
	 * @return 0 if succeed, non-zero error code if error. */
	int update_read_list();

	/* Get the name of the vme controller */
	std::string get_ctl_name();

	/* similar as the update_read_list, but this function dues with the
	 * case when v2718 is the vme controller, and this function is called
	 * by the update_read_list() */
	int update_read_list_v2718();

	/* Get the length of a cmblt chain starting from slot number n
	 * @param n Slot number where the chain starts 
	 * @param len [out] The length of the chain (including the one at slot
	 * n). If no complete chain is found, return -1.
	 * @return 0 if succeed, nonzero error code if error  */
	int get_chain_len(int n, int* len);

	/* Add elements to the vectors (am, dw ...) for a mblt readout.
	 * @param n Slot number
	 **/
	void add_mblt(int n);
	
	/* Add elements to the vectors (am, dw ...) for a cmblt readout.
	 * @param n Slot number
	 * @param addr cblt address
	 **/
	void add_cmblt(int n, uint32_t addr);


private:
	module* mods[MAX_MODULE]; /* an array of pointers of module objects */
	int am_mblts[MAX_MODULE]; /* an array of address modifiers to be used 
				 in case of (c)mblt readout */
	int am_cmblts[MAX_MODULE];

	/* a list of parameters to be used for each event readout, they should
	 * all have the same size: */
	/* a list of address modifier to be used for reading out event */
	std::vector<int> am;
	/* a list of data width to be used for reading out event */
	std::vector<int> dw;
	/* a list of module number (slot number) which should perform a readout */
	std::vector<uint32_t> mod_n;
	/* a list of readout functions which shall be used for reading out 
	 * If read_fun[i] == 0, use read_single_evt
	 * If read_fun[i] == 1, use read_evt_blt
	 * If read_fun[i] == 2, use read_evt_mblt */
	std::vector<int> read_fun;
	/* A list of blt address to be used for cblt readout */
	std::vector<uint32_t> blt_addr;
	/* A list of bool, if 1 the corresponding readout is c(m)blt, otherwise
	 * use (m)blt */
	std::vector<int> chain;
};

#endif 
