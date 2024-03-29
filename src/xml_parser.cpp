#include "xml_parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

xml_parser::xml_parser()
{
	raw_str = NULL;
	e_where = NULL;
	e_what = NULL;
	root = NULL;
	fe = NULL;
	ctl = NULL;
	log = NULL;
	ana = NULL;
	cur_node = NULL;

}

xml_parser::~xml_parser()
{
	if (raw_str)
		free(raw_str);
}
int xml_parser::parse(const char* f_name, int n1, int n2)
{
	/* First, read in the file to a long string. I don't like using
	 * ifstream things, they are too complicated :(*/
	FILE* fp = fopen(f_name, "rb");
	int sz;
	if (!fp) 
		return -E_FILE_O;
	fseek(fp, 0, SEEK_END);
	sz = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	/* becase the raw string might be modified in place, it's better to
	 * allocate a much bigger memory than the actual size of the raw string
	 * */
	raw_str = (char*) malloc(sz<<1);
	if (!raw_str) 
		return -E_MEMORY;
	if (fread(raw_str, 1, sz, fp) != sz) 
		return -E_IO;
	fclose(fp);
	raw_str[sz] = 0;

	/* parse the string */
	try {
		doc.parse<0>(raw_str);
	} catch (rapidxml::parse_error& e) {
		e_where = e.where<char>();
		e_what = e.what();
		return -E_FILE_PAR;
	}

	/* get the root node and child nodes*/
	root = doc.first_node();
	fe = root->first_node("frontend");
	ebd = root->first_node("event_builder");
	ctl = root->first_node("GUI_ctl");
	log = root->first_node("logger");
	ana = root->first_node("analyzer");

	/* now we need to remove some nodes in case of frontend */
	if (n1 != -1) {
		std::vector<rapidxml::xml_node<>*> rm_nodes;
		for (auto it = fe->first_node("vme_module"); it; 
				it = it->next_sibling("vme_module")) {
			int crate_n = get_crate_n(it);
			if (crate_n < 0)
				return -E_FILE_PAR;
			if (crate_n < n1 || crate_n > n2)
				/* out of range, should remove */
				rm_nodes.push_back(it);
		}
		for (auto it = rm_nodes.begin(); it != rm_nodes.end(); it++) 
			fe->remove_node(*it);
	}

	return 0;
}

int xml_parser::get_crate_n(rapidxml::xml_node<>* vme_mod)
{
	/* Iterate all the global_var nodes in the node of the current
	 * module*/
	struct conf_vme_mod conf;
	for (auto it=vme_mod->first_node("global_var"); it;
			it = it->next_sibling("global_var")) {
		cur_node = it;
		if (get_global_var(&conf))
			return -1;
		if (conf.name == "crate_n")
			return conf.val.val_uint64;
	}
	return -1;
}

std::vector<std::vector<struct conf_vme_mod> > 
xml_parser::get_conf_vme_mod(int& status)
{
	std::vector<std::vector<struct conf_vme_mod> > vec_all;
	int err_code;

	status = 0;
	if (!fe) 
		return vec_all;

	/* Iterate all the nodes in the frontend node*/
	rapidxml::xml_node<>* it;
	for (it=fe->first_node("vme_module"); it; 
			it=it->next_sibling("vme_module")) {
		rapidxml::xml_node<>* it2;
		std::vector<struct conf_vme_mod> tmp;
		struct conf_vme_mod conf;
		/* Iterate all the register nodes in the node of the current
		 * module*/
		for (it2=it->first_node("register"); it2;
				it2=it2->next_sibling("register")) {
			cur_node = it2;
			if (err_code = get_register(&conf))
				status = -E_GENERIC;
			else
				tmp.push_back(conf);
		}
		/* Iterate all the global_var nodes in the node of the current
		 * module*/
		for (it2=it->first_node("global_var"); it2;
				it2=it2->next_sibling("global_var")) {
			cur_node = it2;
			if (err_code = get_global_var(&conf))
				status = -E_GENERIC;
			else
				tmp.push_back(conf);
		}
		/* Iterate all the advance_var nodes in the node of the current
		 * module */
		for (it2=it->first_node("advance_var"); it2;
				it2=it2->next_sibling("advance_var")) {
			cur_node = it2;
			if (err_code = get_adv_var(&conf))
				status = -E_GENERIC;
			else
				tmp.push_back(conf);
		}
		vec_all.push_back(tmp);
	}
	return vec_all;
}

std::vector<struct conf_adv> xml_parser::get_conf_adv(int id, int& status)
{
	std::vector<struct conf_adv> vec_all;
	int err_code;
	rapidxml::xml_node<> * node;
	std::string node_name;

	status = 0;
	if (id == 1) {
		node = fe;
		node_name = "advanced_fe";
	}
	else if (id == 2) {
		node = ebd;
		node_name = "advanced_ebd";
	}
	else if (id == 3) {
		node = ctl;
		node_name = "advanced_ctl";
	}
	else if (id == 4) {
		node = log;
		node_name = "advanced_log";
	}
	else if (id == 5) {
		node = ana;
		node_name = "advanced_ana";
	}
	if (!node)
		return vec_all;

	/* Iterate all the nodes in the parent node*/
	rapidxml::xml_node<>* it;
	for (it=node->first_node(node_name.c_str()); it; 
			it=it->next_sibling(node_name.c_str())) {
		struct conf_adv conf;
		conf.name = it->first_node("name")->value();
		conf.value2 = it->first_node("value")->value();
		conf.value = strtol(conf.value2.c_str(), NULL, 0);
		vec_all.push_back(conf);
	}
	return vec_all;
}

int xml_parser::get_register(struct conf_vme_mod* conf)
{
	uint32_t off;
	uint64_t val;
	rapidxml::xml_node<>* node;
	char* str;

	/* get the offset */
	node = cur_node->first_node("offset");
	if (!node)
		return -E_GENERIC;
	str = node->value();
	if (!str)
		return -E_GENERIC;
	off = strtol(str, NULL, 0);

	/* get the value */
	node = cur_node->first_node("value");
	if (!node)
		return -E_GENERIC;
	str = node->value();
	if (!str)
		return -E_GENERIC;
	val = strtol(str, NULL, 0);

	/* save the offset and value*/
	conf->name = "";
	conf->offset = off;
	conf->val.val_uint64 = val;
	return 0;
}

int xml_parser::get_global_var(struct conf_vme_mod* conf)
{
	rapidxml::xml_node<>* node;
	char* str;

	/* get the name */
	node = cur_node->first_node("name");
	if (!node)
		return -E_GENERIC;
	str = node->value();
	if (!str)
		return -E_GENERIC;
	conf->name = str;

	/* get the value */
	node = cur_node->first_node("value");
	if (!node)
		return -E_GENERIC;
	str = node->value();
	if (!str)
		return -E_GENERIC;
	
	if (conf->name == "type") {
		conf->val.val_char = str[0];
	}
	else if (conf->name == "name") {
		/* I don't think we'll ever have the name of a module longer
		 * than 100 charactors. Here we allocate a memory in the heap
		 * so that this piece of memory will be always accessible even
		 * after the destruction of the xml_document object (I know
		 * this will cause a small memory leakage though. */
		conf->val.val_str = new char[100];
		strcpy(conf->val.val_str, str);
	}
	else {
		conf->val.val_uint64 = strtol(str, NULL, 0);
	}

	return 0;
}

int xml_parser::get_adv_var(struct conf_vme_mod* conf)
{
	rapidxml::xml_node<>* node;
	char* str;

	/* get the name */
	node = cur_node->first_node("name");
	if (!node)
		return -E_GENERIC;
	str = node->value();
	if (!str)
		return -E_GENERIC;
	conf->name = str;

	/* get the value */
	node = cur_node->first_node("value");
	if (!node)
		return -E_GENERIC;
	str = node->value();
	if (!str)
		return -E_GENERIC;
	
	conf->val.val_uint64 = strtol(str, NULL, 0);
	conf->val.val_str = new char[1000];
	//str[999] = 0;
	strncpy(conf->val.val_str, str,999);
	conf->val.val_str[999] = 0;

	return 0;
}

#ifdef MAKE_ANALYZER
std::vector<hist_pars> xml_parser::get_ana_hists()
{
	rapidxml::xml_node<> * node = ana;
	std::string node_name("hist_ana");
	std::vector<hist_pars> vec_all;

	/* Iterate all the nodes in the analyzer node*/
	rapidxml::xml_node<>* it;
	for (it=node->first_node(node_name.c_str()); it; 
			it=it->next_sibling(node_name.c_str())) {
		struct hist_pars hist_pars;
		hist_pars.p_obj = NULL;
		hist_pars.id = strtol(it->first_node("id")->value(), NULL, 10);
		hist_pars.nbinsX = strtol(it->first_node("nbinsX")->value(), NULL, 10);
		hist_pars.nbinsY = strtol(it->first_node("nbinsY")->value(), NULL, 10);
		hist_pars.X_min = strtod(it->first_node("X_min")->value(), NULL);
		hist_pars.Y_min = strtod(it->first_node("Y_min")->value(), NULL);
		hist_pars.X_max = strtod(it->first_node("X_max")->value(), NULL);
		hist_pars.Y_max = strtod(it->first_node("Y_max")->value(), NULL);
		hist_pars.type = it->first_node("type")->value();
		hist_pars.name = it->first_node("name")->value();
		hist_pars.folder = it->first_node("folder")->value();
		vec_all.push_back(hist_pars);
	}
	return vec_all;
}
#endif
