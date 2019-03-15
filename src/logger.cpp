#include "log_ctl.h"
#include "log_recv.h"
#include "log_save.h"
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
	log_ctl* ctl = new log_ctl();
	log_recv* receiver = new log_recv();
	log_save* save = new log_save();

	/* init the threads */
	ret = ctl->init(the_init);
	if (ret) {
		handle_err("cannot init thread ctl", ret);
		return ret;
	}
	ret = save->init(the_init);
	if (ret) {
		handle_err("cannot init thread save", ret);
		return ret;
	}
	ret = receiver->init(the_init);
	if (ret) {
		handle_err("cannot init thread receiver", ret);
		return ret;
	}

	/* run the threads */
	ret = ctl->run();
	if (ret) {
		handle_err("cannot run thread ctl", ret);
		return ret;
	}
	ret = save->run();
	if (ret) {
		handle_err("cannot run thread save", ret);
		return ret;
	}
	ret = receiver->run();
	if (ret) {
		handle_err("cannot run thread receiver", ret);
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
		std::cerr<<"thread ctl exited succesfully"<<std::endl;
	}
	if (save->join((void**)&p_ret))
		std::cerr<<"cannot join thread!"<<std::endl;
	if (*p_ret) {
		std::cerr<<"thread save exited with error code: ";
		std::cerr<<"error code: "<<(*p_ret)<<std::endl;
	}
	else {
		std::cerr<<"thread save exited succesfully"<<std::endl;
	}
	if (receiver->join((void**)&p_ret))
		std::cerr<<"cannot join thread!"<<std::endl;
	if (*p_ret) {
		std::cerr<<"thread receiver exited with error code: ";
		std::cerr<<"error code: "<<(*p_ret)<<std::endl;
	}
	else {
		std::cerr<<"thread receiver exited succesfully"<<std::endl;
	}

	return 0;
}

