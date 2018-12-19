/* This class is the initializer which initialies everything for the frontend,
 * including vme modules found in the config file, ring buffers etc. 
 *
 * By B.Gao Oct. 2018 */
#ifndef INITZER_HHH
#define INITZER_HHH

#include "err_code.h"
#include "xml_parser.h"
#include "ring_buf.h"
#include "modules.h"
#include "madc32.h"
#include "v2718.h"
#include "v1190.h"
#include "v830.h"
#include "imp_daq.h"
#include <vector>
#include <string>


class initzer
{
public:
	initzer();
	~initzer();

	/* This method should be called before doing anything else */
	void set_parser(xml_parser* parser) {p_parser = parser;}


	/* see xml_parser::get_conf_vme_mod() */
	int 
	get_conf_vme_mod(std::vector<std::vector<struct conf_vme_mod> >& conf)
	{
		int ret;
		if (!p_parser)
			ret = -E_GENERIC;
		else
			conf = p_parser->get_conf_vme_mod(ret);
		return ret;
	}
	/* Get a list of trigger/scaler-type modules. Each element in the
	 * vector is an object of class modules. We don't return individual vme
	 * module, instead we return modules with which the readout is much
	 * easier. The trigger-type modules are added to the same 'modules'
	 * object only if 1) they are the same vme module 2) they belong the
	 * same crate. scaler-type modules are added to the same
	 * 'modules' object only if 1) they are the same vme module 2) they
	 * have the same reading period. 3) they belong the same crate . 
	 * Return 0 if succeed, otherwise return error code. */
	int get_modules(char type, std::vector<modules*>& list_modules);

	/* Get the trigger module, return NULL if error or not found. */
	module* get_trig_mod();


	/* get the pointer of a ring buffer object (for data and message
	 * sharing) for the frontend, return NULL in case of error.  The user
	 * is responsible to delete the buffer. 
	 * id == 1 --> the data ring buffer 
	 * id == 2 --> the message ring buffer */
	ring_buf* get_fe_rb(int id);

	/* Similar as the get_fe_rb,
	 * id == 5 --> The scaler event ring buffer, analog to those returned
	 *             in get_ebd_rbs();
	 * id == 3 --> the built event ring buffer 
	 * id == 2 --> the message ring buffer.
	 * id == 1 --> the raw data received from frontend
	 * */
	ring_buf* get_ebd_rb(int id);

	/* get the clock freqency */
	uint32_t get_ebd_sort_clock_hz();

	/* Get frontend buffer size. Return 0 if the configuration is not found
	 * in the config file. 
	 * @param id is the id of ring buffer: 1--> data ring buffer, 2-->
	 * message ring buffer.*/
	int get_fe_buf_sz(int id);

	/* Get the buffer size of event builder, return 0 if the configuration
	 * is not found in the config file.
	 * @param id is the buffer type: 
	 *     1--> raw data ring buffer, 
	 *     2--> message ring buffer, 
	 *     3-->built event ring buffer,
	 *     4-->individual ring buffer for each vme module.
	 *     5-->scaler ring buffer 
	 *     */
	int get_ebd_buf_sz(int id);


	/* get a vector of ring buffers. each element in the vector is a
	 * pointer of a ring buffer dedicated to one vme module, the modules
	 * are distinguished by their daq number, crate number and slot number.
	 * */
	std::vector<ring_buf*> get_ebd_rbs();

	/* get the variable n_try,
	 * */
	int get_fe_ntry();


	/* get the listening port of the frontend data sender 
	 * */
	int get_fe_sender_port();

	/* get the listening port of the GUI controler for frontend 
	 * */
	int get_ctl_port_fe();

	/* get the variable blt_buf_sz,
	 * */
	int get_fe_blt_buf_sz();

	/* get the interval of the frontend data sender on which it spends to
	 * */
	int get_fe_sender_itv();

	/* get the socket buffer size of the frontend data sender.
	 * */
	int get_fe_sender_buf_sz();

	/* get the socket buffer size of the frontend control thread
	 * */
	int get_fe_ctl_buf_sz();

	/* get the on_start_t_max of frontend */
	int get_fe_on_start_t_max();

	/* get the time out (us) of the select syscall  of the frontend control
	 * thread
	 *  */
	int get_fe_ctl_t_us();

	/* get the timeout (us) of the select syscall of the event builder
	 * receiver thread (how many micro seconds are spend to wait for
	 * incoming data packet from frontend)*/
	int get_ebd_recv_t_us();

	/* get the server address of the frontend control thread
	 *  */
	std::string get_fe_ctl_svr_addr();

	/* get the server address of the event builder receiver server */
	std::string get_ebd_recv_svr_addr();
	
	/* return NULL indicating errors */
	char* get_slot_map() {return slot_map;}

private:
	/* find and init all the v2718 modules.
	 * Return 0 if succeed, otherwise return error code. */
	int init_v2718();

	/* get the advanced variable (by name),
	 * @param id ID number of the component whoes variable is going to be
	 * retrived. The definition of the ID numbers is the same as those in
	 * xml_parser.h 
	 * @param var_name name of the variable to find
	 * @param found indicates if the wanted variable is found 
	 * @param value2 pointer to a string. if not NULL, the raw string of
	 * the variable is also returned in value2.
	 * @return return the value of the variable (if found) */
	int get_adv_var(int id, std::string& var_name, bool& found, 
			std::string* value2 = NULL);
	int get_fe_adv_var(std::string& var_name, bool& found, 
			std::string* value2 = NULL) 
	{return get_adv_var(1, var_name, found, value2); }
	int get_ebd_adv_var(std::string& var_name, bool& found, 
			std::string* value2 = NULL)
	{return get_adv_var(2, var_name, found, value2); }
	int get_ctl_adv_var(std::string& var_name, bool& found, 
			std::string* value2 = NULL)
	{return get_adv_var(3, var_name, found, value2); }



	/* Init the vme module according to its configurations. Return the
	 * pointer to the vme object. Return NULL in case of error. */
	v2718* do_init_v2718(std::vector<struct conf_vme_mod> &the_conf);


	/* initialize the global variables of the given module 
	 * return 0 if succeed, otherwise return error code. */
	int init_global_var(module* mod, 
			std::vector<struct conf_vme_mod> &the_conf);
	/* get the module name */
	char* get_mod_name(std::vector<struct conf_vme_mod> &the_conf);

	/* Initialize all vme modules that are found in the configuration file
	 * (p_parser). 
	 * In addition, this function also fills the slot maps of the vme
	 * modules. (Why we need slot map ? see "ebd_sort.h")
	 * Return 0 if succeed, otherwise return error code.*/
	int init_vme_mod();

	/* fill an entry in the slot map accroding to the settings of the
	 * module 
	 * return 0 if succeed, otherwise return error code. 
	 * */
	int fill_slot_map(module* mod);


private: xml_parser* p_parser;
	std::vector<module*> p_module;
	std::vector<v2718*> p_v2718;
	std::vector<std::vector<struct conf_vme_mod> > vme_conf;

	bool vme_mod_inited;
	ring_buf* rb_fe; /* This is for data sharing */
	ring_buf* rb_fe2; /* this is for message sharing */

	/* There are 4 types of ring buffers in event builder:
	 * rb_ebd: contains the raw data received from frontend 
	 * rb_ebd2: for message sharing 
	 * rb_ebd3: contains built event 
	 * rb_ebd5: contains scaler event
	 * rbs_ebd: individual ring buffers for each vme module 
	 * */
	ring_buf* rb_ebd;
	ring_buf* rb_ebd2;
	ring_buf* rb_ebd3;
	ring_buf* rb_ebd5;
	std::vector<ring_buf*> rbs_ebd; 

	/* see the comments in ebd_sort.h. Because 3-d arrays are very
	 * difficult to handle, we use a 1d array instead.  */
	char slot_map[MAX_SLOT_MAP];
	bool slot_map_inited;





};



#endif 
