#include "ana_ctl.h"
#include "ana_main.h"
#include "ana_recv.h"
#include "ana_roody_svr.h"
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
	ana_ctl* ctl = new ana_ctl();
	ana_recv* receiver = new ana_recv();
	ana_main* main_proc = new ana_main();
	ana_roody_svr* roody_svr = new ana_roody_svr();

	/* init the threads */
	ret = ctl->init(the_init);
	if (ret) {
		handle_err("cannot init thread ctl", ret);
		return ret;
	}
	ret = main_proc->init(the_init);
	if (ret) {
		handle_err("cannot init thread main_proc", ret);
		return ret;
	}
	ret = receiver->init(the_init);
	if (ret) {
		handle_err("cannot init thread receiver", ret);
		return ret;
	}
	ret = roody_svr->init(the_init);
	if (ret) {
		handle_err("cannot init thread roody_svr", ret);
		return ret;
	}

	/* run the threads */
	ret = ctl->run();
	if (ret) {
		handle_err("cannot run thread ctl", ret);
		return ret;
	}
	ret = main_proc->run();
	if (ret) {
		handle_err("cannot run thread main_proc", ret);
		return ret;
	}
	ret = receiver->run();
	if (ret) {
		handle_err("cannot run thread receiver", ret);
		return ret;
	}
	ret = roody_svr->run();
	if (ret) {
		handle_err("cannot run thread roody_svr", ret);
		return ret;
	}

	/* join the threads and check the return value */
	if (ctl->join((void**)&p_ret))
		std::cerr<<"cannot join thread!"<<std::endl;
	if (*p_ret) {
		std::cerr<<"thread ctl exited with error code: ";
		std::cerr<<"error code: "<<(*p_ret)<<std::endl;
	}
	else {
		std::cerr<<"thread ctl exited succesfully";
	}
	if (main_proc->join((void**)&p_ret))
		std::cerr<<"cannot join thread!"<<std::endl;
	if (*p_ret) {
		std::cerr<<"thread main_proc exited with error code: ";
		std::cerr<<"error code: "<<(*p_ret)<<std::endl;
	}
	else {
		std::cerr<<"thread main_proc exited succesfully";
	}
	if (receiver->join((void**)&p_ret))
		std::cerr<<"cannot join thread!"<<std::endl;
	if (*p_ret) {
		std::cerr<<"thread receiver exited with error code: ";
		std::cerr<<"error code: "<<(*p_ret)<<std::endl;
	}
	else {
		std::cerr<<"thread receiver exited succesfully";
	}
	if (roody_svr->join((void**)&p_ret))
		std::cerr<<"cannot join thread!"<<std::endl;
	if (*p_ret) {
		std::cerr<<"thread roody_svr exited with error code: ";
		std::cerr<<"error code: "<<(*p_ret)<<std::endl;
	}
	else {
		std::cerr<<"thread roody_svr exited succesfully";
	}

	return 0;
}

