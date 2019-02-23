#include "fe_ctl.h"
#include "fe_sender.h"
#include "rd_scal.h"
#include "rd_trig.h"
#include <iostream>

void handle_err(const char* err, int code)
{
	std::cerr<<err<<" error code: "<<code<<std::endl;
}

int main(int argc, char* argv[])
{
	int ret;
	int* p_ret;

	/* First, we create the initzer and parser */
	xml_parser* parser = new xml_parser();
	ret = parser->parse("config.xml");
	if (ret) {
		handle_err("cannot parse config file!", ret);
		return ret;
	}
	initzer* the_init = new initzer();
	the_init->set_parser(parser);
	
	/* create the threads */
	fe_ctl* ctl = new fe_ctl();
	fe_sender* sender = new fe_sender();
	rd_trig* rd_t = new rd_trig();
	rd_scal* rd_s = new rd_scal();

	/* init the threads */
	ret = rd_t->init(the_init);
	if (ret) {
		handle_err("cannot init thread rd_trig", ret);
		return ret;
	}
	ret = rd_s->init(the_init);
	if (ret) {
		handle_err("cannot init thread rd_scal", ret);
		return ret;
	}
	ret = sender->init(the_init);
	if (ret) {
		handle_err("cannot init thread sender", ret);
		return ret;
	}
	ret = ctl->init(the_init);
	if (ret) {
		handle_err("cannot init thread ctl", ret);
		return ret;
	}

	/* run the threads */
	ret = rd_t->run();
	if (ret) {
		handle_err("cannot run thread rd_trig", ret);
		return ret;
	}
	ret = rd_s->run();
	if (ret) {
		handle_err("cannot run thread rd_scal", ret);
		return ret;
	}
	ret = sender->run();
	if (ret) {
		handle_err("cannot run thread sender", ret);
		return ret;
	}
	ret = ctl->run();
	if (ret) {
		handle_err("cannot run thread ctl", ret);
		return ret;
	}

	/* join the threads and check the return value */
	if (rd_t->join(reinterpret_cast<void**>(&p_ret)))
		std::cerr<<"cannot join thread!"<<std::endl;
	if (*p_ret) {
		std::cerr<<"thread rd_trig exited with error code: ";
		std::cerr<<"error code: "<<(*p_ret)<<std::endl;
	}
	else {
		std::cerr<<"thread rd_trig exited succesfully";
	}
	if (rd_s->join(reinterpret_cast<void**>(&p_ret)))
		std::cerr<<"cannot join thread!"<<std::endl;
	if (*p_ret) {
		std::cerr<<"thread rd_scal exited with error code: ";
		std::cerr<<"error code: "<<(*p_ret)<<std::endl;
	}
	else {
		std::cerr<<"thread rd_scal exited succesfully";
	}
	if (sender->join(reinterpret_cast<void**>(&p_ret)))
		std::cerr<<"cannot join thread!"<<std::endl;
	if (*p_ret) {
		std::cerr<<"thread sender exited with error code: ";
		std::cerr<<"error code: "<<(*p_ret)<<std::endl;
	}
	else {
		std::cerr<<"thread sender exited succesfully";
	}
	if (ctl->join(reinterpret_cast<void**>(&p_ret)))
		std::cerr<<"cannot join thread!"<<std::endl;
	if (*p_ret) {
		std::cerr<<"thread ctl exited with error code: ";
		std::cerr<<"error code: "<<(*p_ret)<<std::endl;
	}
	else {
		std::cerr<<"thread ctl exited succesfully";
	}

	return 0;
}

