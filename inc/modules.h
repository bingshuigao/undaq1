/* This class has a list of vme modules (of the same type) and manages all
 * these modules. The purpose of this class is to make the readout of multiple
 * modules easier. 
 * The format of the data is as following (each element is a 4-byte word):
 *                  _______________
 *                  | length      |
 *                  |_____________|
 *                  | mask        |
 *                  |_____________|
 *                  | data block  |
 *                  |_____________|
 *                  | length      |
 *                  |_____________|
 *                  | mask        |
 *                  |_____________|
 *                  | data block  |
 *                  |_____________|
 *                  | ...         |
 *                  |_____________|
 * That means for each readout block (either single module or a chain), their
 * is a length, a mask and the data block.  The length is the data length of
 * the readout block (in byte, inclusive). The mask is the corresponding slots
 * of the modules (see private variable mod_n).  The data block is the data
 * readout from the module event buffer.
 *
 *             
 * By B.Gao Aug. 2018 */

#ifndef MODULES_HHH
#define MODULES_HHH


#include <vector>
#include "module.h"
#include "imp_daq.h"

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
	 * @return 0 if succeed, non-zero error code if error.
	 * */
	int add_mod(module* mod);

	/* read the event buffers of all the modules. 
	 * @param buf Buffer where the events will be saved.
	 * @param sz_in Maximum requested size to read (bytes)
	 * @param sz_out Actual size of data read (bytes)
	 * @return 0 if succeed, non-zero error code if error */
	int read_evtbuf(void* buf, int sz_in, int* sz_out);


	/* Prepare to start/stop, should be called at each start/stop of run.
	 * @return 0 if succeed, non-zero error code if error */
	int on_start()
	{
		int i, ret;
		for (i = 0; i < MAX_MODULE; i++) {
			if (!mods[i])
				continue;
			ret = mods[i]->on_start();
			RET_IF_NONZERO(ret);
		}
		return 0;
	}
	int on_stop() 
	{
		int i, ret;
		for (i = 0; i < MAX_MODULE; i++) {
			if (!mods[i])
				continue;
			ret = mods[i]->on_stop();
			RET_IF_NONZERO(ret);
		}
		return 0;
	}

	module** get_modules() {return mods;}
	/* get the first module */
	module*  get_1st_mod()
	{
		for (auto i = 0; i<MAX_MODULE; i++) {
			if (mods[i]) return mods[i];
		}
		return NULL;
	}
	std::string get_name() {return name;}
	int get_crate() {return crate_n;}



private:
	/* Readout reset, should be called after each readout.
	 * @return 0 if succeed, non-zero error code if error */
	int on_readout()
	{
		int i, ret;
		for (i = 0; i < MAX_MODULE; i++) {
			if (!mods[i])
				continue;
			ret = mods[i]->on_readout();
			RET_IF_NONZERO(ret);
		}
		return 0;
	}

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
	 * @param n Slot number of the first module in the chain.
	 * @param addr cblt address
	 * @param len length of the cmblt chain.
	 **/
	void add_cmblt(int n, uint32_t addr, int len);


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
	/* a list of module number (slot number) which should perform a
	 * readout. Each element of the vector is in fact a bit mask pattern,
	 * instead of a integer representing the slot number. For example, if a
	 * module in slot 5 needs readout, then the element value is 0x20 (bit
	 * mask pattern is 0000 0000 0010 0000). If a chain of modules needs
	 * readout, say in slot 5,6,7, then the element value is 0xE0 (bit mask
	 * pattern is 0000 0000 1110 0000). */
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
	/* name of the modules (which equals the name of each module)*/
	std::string name;
	/* crate number of the modules (which equals the crate number of each
	 * module)*/
	int crate_n;
};

#endif 
