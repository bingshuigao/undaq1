/* This class is the initializer which initialies everything for the frontend,
 * including vme modules found in the config file, ring buffers etc. 
 *
 * By B.Gao Oct. 2018 */
#ifndef INITZER_HHH
#define INITZER_HHH

#include "err_code.h"
#include "xml_parser.h"
#include "ring_buf.h"
#include "imp_daq.h"
#include <vector>
#include <string>

#ifdef MAKE_ANALYZER
#include "hist_man.h"
#endif

#ifdef MAKE_FRONTEND
#include "modules.h"
#include "madc32.h"
#include "v2718.h"
#include "v1190.h"
#include "v830.h"
#include "v1740.h"
#endif

struct mod_rb_par
{
	int crate;
	int slot;
};

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
#ifdef MAKE_FRONTEND
	std::vector<struct mod_rb_par> get_rbs_ebd(int& ret) 
	{
		ret = 0;
		if (get_mod_rbs())
			ret = -E_GENERIC;
		return rbs_ebd;
	}
	/* get the server address of the frontend control thread
	 *  */
	std::string get_fe_ctl_svr_addr();
	
	/* get the on_start_t_max of frontend */
	int get_fe_on_start_t_max();

	/* get the time out (us) of the select syscall  of the frontend control
	 * thread
	 *  */
	int get_fe_ctl_t_us();
	/* get the socket buffer size of the frontend control thread
	 * */
	int get_fe_ctl_buf_sz();
	

	/* get the interval of the frontend data sender on which it spends to
	 * */
	int get_fe_sender_itv();

	
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

	/* Get frontend buffer size. Return 0 if the configuration is not found
	 * in the config file. 
	 * @param id is the id of ring buffer: 1--> data ring buffer, 2-->
	 * message ring buffer.*/
	int get_fe_buf_sz(int id);

	/* get the variable n_try,
	 * */
	int get_fe_ntry();

#endif


#ifdef MAKE_EVENT_BUILDER
	/* get the server address of the event builder control thread
	 *  */
	std::string get_ebd_ctl_svr_addr();
	/* get the timeout (us) of the select syscall of the event builder
	 * receiver thread (how many micro seconds are spend to wait for
	 * incoming data packet from frontend)*/
	int get_ebd_recv_t_us();
	
	/* get the time out (us) of the select syscall  of the ebd control
	 * thread
	 *  */
	int get_ebd_ctl_t_us();
	
	/* get the socket buffer size of the event builder control thread
	 * */
	int get_ebd_ctl_buf_sz();
	

	/* Similar as the get_fe_rb,
	 * id == 5 --> The scaler event ring buffer, analog to those returned
	 *             in get_ebd_rbs();
	 * id == 3 --> the built event ring buffer 
	 * id == 2 --> the message ring buffer.
	 * */
	ring_buf* get_ebd_rb(int id);

	/* similar as get_ebd_rb(int), however, because there may be multiple
	 * frontends, we need to return a vector here. an empty vector
	 * indicates error.*/
	std::vector<ring_buf*> get_ebd_rb();

	/* get the clock freqency */
	uint32_t get_ebd_sort_clock_hz();

	/* get the glom for merger */
	uint32_t get_ebd_merge_glom();
	
	/* get the event builder type (ts/cnt) */
	uint32_t get_ebd_merge_type();

	/* get the buffer size for the merged events */
	uint32_t get_ebd_merge_merged_buf_sz();

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

	/* get the max length (32-byte words) of a single event */
	int get_ebd_max_evt_len();


#endif

#ifdef MAKE_LOGGER
	/* get the server address of the logger controler thread */
	std::string get_log_ctl_svr_addr();
	/* get the data saving path */
	std::string get_log_save_path();

	/* get the buffer length of the logger save thread */
	uint32_t get_log_save_buf_len();

	/* similar as the get_ebd_recv_t_us();
	 * */
	int get_log_recv_t_us();
	/* get the time out (us) of the select syscall  of the log control
	 * thread
	 *  */
	int get_log_ctl_t_us();
	
	/* get the socket buffer size of the logger control thread */
	int get_log_ctl_buf_sz();

	/* Similar as the get_fe_rb,
	 * id == 0 --> The trigger event ring buffer
	 * id == 1 --> the message ring buffer
	 * id == 2 --> the scaler event ring buffer.
	 * */
	ring_buf* get_log_rb(int id);

	/* Get the buffer size of logger, return 0 if the configuration
	 * is not found in the config file.
	 * @param id is the buffer type: 
	 * id == 0 --> The trigger event ring buffer
	 * id == 1 --> the message ring buffer
	 * id == 2 --> the scaler event ring buffer.
	 *     */
	int get_log_buf_sz(int id);

#endif


#ifdef MAKE_ANALYZER
	/* get the server address of the analyzer control thread */
	std::string get_ana_ctl_svr_addr();
	/* similar as the get_ebd_recv_t_us();
	 * */
	int get_ana_recv_t_us();
	/* get the time out (us) of the select syscall  of the analyzer control
	 * thread */
	int get_ana_ctl_t_us();
	/* get the socket buffer size of the analyzer control thread */
	int get_ana_ctl_buf_sz();
	
	/* get a vector of histogram parameters */
	std::vector<hist_pars> get_ana_hists() 
	{return p_parser->get_ana_hists();}
	
	/* get the analyzer ring buffer 
	 * id == 0 --> The trigger event ring buffer
	 * id == 1 --> the message ring buffer
	 * id == 2 --> the scaler event ring buffer.
	 *
	 * */
	ring_buf* get_ana_rb(int id);

	/* get the analyzer ring buffer size */
	int get_ana_buf_sz(int id);
	
	/* get the analyzer event buffer size */
	int get_ana_main_buf_sz();
	
	/* get the analyzer roody server listening port */
	int get_ana_roody_svr_port();

#endif



	/* get the variable blt_buf_sz,
	 * */
	int get_fe_blt_buf_sz();

	/* get the socket buffer size of the event builder data sender.
	 * */
	int get_ebd_sender_buf_sz();

	/* get the socket buffer size of the frontend data sender.
	 * */
	int get_fe_sender_buf_sz();

	/* get the listening port of the frontend data sender 
	 * */
	int get_fe_sender_port();
	/* get the listening port of the evt builder data sender 
	 * */
	int get_ebd_sender_port();
	
	/* get the number of receiver thread of the evt builder */
	int get_ebd_n_recv();

	/* get the listening port of the GUI controler for frontend 
	 * */
	int get_ctl_port();

	/* get the n'th server address of the event builder
	 * receiver server, return empty string if error.
	 * */
	std::string get_ebd_recv_svr_addr(int n);

	/* get the server address of the logger receiver server */
	std::string get_log_recv_svr_addr();

	/* get the server address of the analyzer receiver server */
	std::string get_ana_recv_svr_addr();
	
	/* return NULL indicating errors */
	char* get_slot_map() {return slot_map;}
	uint64_t* get_clk_map() {return clk_map;}

private:
#ifdef MAKE_FRONTEND
	/* get a vector of ring buffers. 
	 * NOTE: what we get here is NOT really ring buffers, but the
	 * parameters that are needed to allocate these ring buffers. These
	 * parameters will be sent to the event builder (as soon as the event
	 * builder connects with the frontend), then these ring buffers will be
	 * created by the event builder.
	 * These ring buffers are distinguished by their daq number, crate
	 * number and slot number.
	 * Return 0 if succeed, otherwise return error code. 
	 * */
	int get_mod_rbs();
	
	/* find and init all the v2718 modules.
	 * Return 0 if succeed, otherwise return error code. */
	int init_v2718();
	
	/* Init the vme module according to its configurations. The offset is
	 * the offset between board numbers and crate numbers. Return the
	 * pointer to the vme object. Return NULL in case of error. */
	v2718* do_init_v2718(std::vector<struct conf_vme_mod> &the_conf, int offset);


	/* initialize the global variables of the given module 
	 * return 0 if succeed, otherwise return error code. */
	int init_global_var(module* mod, 
			std::vector<struct conf_vme_mod> &the_conf);
	
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

	/* similar as fill_slot_map() */
	int fill_clk_map(module* mod);
#endif

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
	int get_log_adv_var(std::string& var_name, bool& found, 
			std::string* value2 = NULL)
	{return get_adv_var(4, var_name, found, value2); }
	int get_ana_adv_var(std::string& var_name, bool& found, 
			std::string* value2 = NULL)
	{return get_adv_var(5, var_name, found, value2); }



	/* get the module name */
	char* get_mod_name(std::vector<struct conf_vme_mod> &the_conf);
	/* get the module crate number  */
	int64_t get_mod_crate(std::vector<struct conf_vme_mod> &the_conf);



private: 
	xml_parser* p_parser;

#ifdef MAKE_FRONTEND
	std::vector<module*> p_module;
	std::vector<v2718*> p_v2718;
#endif
	std::vector<std::vector<struct conf_vme_mod> > vme_conf;

	bool vme_mod_inited;
	ring_buf* rb_fe; /* This is for data sharing */
	ring_buf* rb_fe2; /* this is for message sharing */

	/* There are 4 types of ring buffers in event builder:
	 * rb_ebd: contains the raw data received from frontend (vector)
	 * rb_ebd2: for message sharing 
	 * rb_ebd3: contains built event 
	 * rb_ebd5: contains scaler event
	 * rbs_ebd: individual ring buffers for each vme module 
	 * */
	std::vector<ring_buf*> rb_ebd;
	ring_buf* rb_ebd2;
	ring_buf* rb_ebd3;
	ring_buf* rb_ebd5;
	std::vector<struct mod_rb_par> rbs_ebd; 

	/* ring buffers for the logger
	 * rb_log0 --> trigger event ring buffer 
	 * rb_log1 --> message ring buffer 
	 * rb_log2 --> scaler event ring buffer 
	 *
	 * */
	ring_buf* rb_log0;
	ring_buf* rb_log1;
	ring_buf* rb_log2;

	/* ring buffers for the analyzer 
	 * rb_ana0 --> trigger event ring buffer 
	 * rb_ana1 --> message ring buffer 
	 * rb_ana2 --> scaler event ring buffer 
	 * */
	ring_buf* rb_ana0;
	ring_buf* rb_ana1;
	ring_buf* rb_ana2;
	
	/* see the comments in ebd_sort.h. Because 3-d arrays are very
	 * difficult to handle, we use a 1d array instead.  */
	char slot_map[MAX_SLOT_MAP];
	bool slot_map_inited;

	/* similar as slot map */
	uint64_t clk_map[MAX_CLK_MAP];
	bool clk_map_inited;
};



#endif 
