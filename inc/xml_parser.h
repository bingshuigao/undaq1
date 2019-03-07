/* This class takes care of the configuration file which is an xml file. It
 * reads the xml file and parse it. It provides useful methods which can be
 * used to get the configuration information defined in the xml file. The
 * parser is implemented by rapidxml
 * (http://rapidxml.sourceforge.net/manual.html)
 *
 * By B.Gao Sep. 2018 */

#ifndef XML_PARSER_HHH
#define XML_PARSER_HHH

#include <string>
#include <stdint.h>
#include <vector>
#include "err_code.h"
#include "rapid_xml.hpp"
#include "imp_daq.h"

#ifdef MAKE_ANALYZER
#include "hist_man.h"
#endif

/* This structure represents a configuration parameters for a vme module. */
struct conf_vme_mod
{
	/* name is the name of the global variable, for registers name == ""*/
	std::string name;
	/* offset is the offset of a register, this field is meaningless for
	 * global variables (if name!="")*/
	uint32_t offset;

	/* this union is designed to accommodate different types of data to be
	 * contained. For registers, use the field val_uint64, for global
	 * variables, use appropriate field accordingly .*/
	union val
	{
		uint64_t val_uint64;
		char* val_str;
		char val_char;
	} val;
};

/* This structure represents an advanced configuration */
struct conf_adv
{
	std::string name;
	long int value;
	/* value2 is the 'raw' string of the variable saved in the xml file */
	std::string value2;
};

class xml_parser
{
public:
	xml_parser();
	~xml_parser();
	
	/* parse the configuration file.
	 * return 0 if the configuration file has been parsed successfully,
	 * otherwise return an error code.*/
	int parse(const char* f_name);

	/* get the configruations of the vme modules (a vector of vectors). The
	 * results are returned as a vector containing a vector of struct
	 * conf_vme_mod for each vme module*/
	std::vector<std::vector<struct conf_vme_mod> > 
		get_conf_vme_mod(int& status);

	/* Get the advanced configurations (if any)
	 * @param id = 1 -> get advanced config of frontend
	 * @param id = 2 -> get advanced config of event builder 
	 * @param id = 3 -> get advanced config of GUI controler
	 * @param id = 4 -> get advanced config of logger
	 * @param id = 5 -> get advanced config of analyzer
	 * @param status -> error code */
	std::vector<struct conf_adv> get_conf_adv(int id, int& status);
	
	std::vector<struct conf_adv> get_conf_adv_fe(int& status)
	{ return get_conf_adv(1, status);}
	
	std::vector<struct conf_adv> get_conf_adv_ebd(int& status)
	{ return get_conf_adv(2, status);}

#ifdef MAKE_ANALYZER
	/* get a list of histogram parameters (if any) */
	std::vector<hist_pars> get_ana_hists();
#endif

private:
	/* get the register and global var from the current node, return 0 if
	 * succeed, otherwise return the error code  */
	int get_register(struct conf_vme_mod* conf);
	int get_global_var(struct conf_vme_mod* conf);
	int get_adv_var(struct conf_vme_mod* conf);

private:
	rapidxml::xml_document<> doc;
	char* raw_str; /* raw string containing the whole xml file */
	const char* e_where; /* where the parsing error occurs. */
	const char* e_what; /* what is the parsing error. */
	rapidxml::xml_node<> * root; /* the root node */
	rapidxml::xml_node<> * fe; /* the node of frontend */
	rapidxml::xml_node<> * ebd;/* the node of event builder */
	rapidxml::xml_node<> * ctl;/* the node of GUI controler */
	rapidxml::xml_node<> * log;/* the node of logger */
	rapidxml::xml_node<> * ana;/* the node of analyzer */
	rapidxml::xml_node<> * cur_node; /* the current active node */
};



#endif
