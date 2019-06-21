#include "ebd_ctl.h"
#include "ebd_merge.h"
#include "ebd_recv.h"
#include "ebd_sender.h"
#include "ebd_sort.h"
#include <iostream>
#include <vector>

void handle_err(const char* err, int code)
{
	std::cerr<<err<<" error code: "<<code<<std::endl;
}

int main(int argc, char* argv[])
{
	int ret;
	int* p_ret;
	int i, n_recv;

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
	n_recv = the_init->get_ebd_n_recv();
	ebd_ctl* ctl = new ebd_ctl();
	ebd_sender* sender = new ebd_sender();
	std::vector<ebd_recv*> receivers;
	for (i = 0; i < n_recv; i++) 
		receivers.push_back(new ebd_recv(n_recv, i));
	ebd_merge* merger = new ebd_merge();
	ebd_sort* sorter = new ebd_sort();

	/* init the threads */
	ret = ctl->init(the_init);
	if (ret) {
		handle_err("cannot init thread ctl", ret);
		return ret;
	}
	ret = sender->init(the_init);
	if (ret) {
		handle_err("cannot init thread sender", ret);
		return ret;
	}
	for (auto it = receivers.begin(); it != receivers.end(); it++) {
		ret = (*it)->init(the_init);
		if (ret) {
			handle_err("cannot init thread receiver", ret);
			return ret;
		}
	}
	ret = merger->init(the_init);
	if (ret) {
		handle_err("cannot init thread merger", ret);
		return ret;
	}
	ret = sorter->init(the_init);
	if (ret) {
		handle_err("cannot init thread sorter", ret);
		return ret;
	}

	/* run the threads */
	ret = ctl->run();
	if (ret) {
		handle_err("cannot run thread ctl", ret);
		return ret;
	}
	ret = sender->run();
	if (ret) {
		handle_err("cannot run thread sender", ret);
		return ret;
	}
	for (auto it = receivers.begin(); it != receivers.end(); it++) {
		ret = (*it)->run();
		if (ret) {
			handle_err("cannot run thread receiver", ret);
			return ret;
		}
	}
	ret = merger->run();
	if (ret) {
		handle_err("cannot run thread merger", ret);
		return ret;
	}
	ret = sorter->run();
	if (ret) {
		handle_err("cannot run thread sorter", ret);
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
	if (sender->join((void**)&p_ret))
		std::cerr<<"cannot join thread!"<<std::endl;
	if (*p_ret) {
		std::cerr<<"thread sender exited with error code: ";
		std::cerr<<"error code: "<<(*p_ret)<<std::endl;
	}
	else {
		std::cerr<<"thread sender exited succesfully"<<std::endl;
	}
	for (auto it = receivers.begin(); it != receivers.end(); it++) {
		if ((*it)->join((void**)&p_ret))
			std::cerr<<"cannot join thread!"<<std::endl;
		if (*p_ret) {
			std::cerr<<"thread receiver exited with error code: ";
			std::cerr<<"error code: "<<(*p_ret)<<std::endl;
		}
		else {
			std::cerr<<"thread receiver exited succesfully"<<std::endl;
		}
	}
	if (merger->join((void**)&p_ret))
		std::cerr<<"cannot join thread!"<<std::endl;
	if (*p_ret) {
		std::cerr<<"thread merger exited with error code: ";
		std::cerr<<"error code: "<<(*p_ret)<<std::endl;
	}
	else {
		std::cerr<<"thread merger exited succesfully"<<std::endl;
	}
	if (sorter->join((void**)&p_ret))
		std::cerr<<"cannot join thread!"<<std::endl;
	if (*p_ret) {
		std::cerr<<"thread sorter exited with error code: ";
		std::cerr<<"error code: "<<(*p_ret)<<std::endl;
	}
	else {
		std::cerr<<"thread sorter exited succesfully"<<std::endl;
	}

	return 0;
}

