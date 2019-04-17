#include "ana_main.h"
#include <unistd.h>
#include <iostream>

ana_main::ana_main()
{
	thread_id = 2;
	p_evt = NULL;
	init_fun.push_back(&ana_main_init);
}

ana_main::~ana_main()
{
	if (p_evt)
		delete p_evt;
}

int ana_main::ana_main_init(my_thread* This, initzer* the_initzer)
{
	ana_main* ptr = reinterpret_cast<ana_main*>(This);

	TFolder *all_folder = NULL;      // Container for all histograms
	TFolder *subfolder = NULL;
	TApplication *ana_app;
	int argn = 1;
	char *argp = new char[10];
	argp[0] = 0;
	ana_app = new TApplication("ana_app", &argn, &argp, 0, 0);
	delete argp;

	/* Create the parent folder */
	all_folder = gROOT->GetRootFolder()->AddFolder("histos", "histos");
	gROOT->GetListOfBrowsables()->Add(all_folder, "histos");

	/* create the histograms */
	auto hists = the_initzer->get_ana_hists();
	for (auto it = hists.begin(); it != hists.end(); it++) {
		const char* folder_name = (*it).folder.c_str();
		/* first, create the hist */
		if ((*it).type == "TH1D")
			(*it).p_obj = new TH1D((*it).name.c_str(),
					(*it).name.c_str(), (*it).nbinsX,
					(*it).X_min, (*it).X_max);
		else if ((*it).type == "TH2D")
			(*it).p_obj = new TH2D((*it).name.c_str(),
					(*it).name.c_str(), (*it).nbinsX,
					(*it).X_min, (*it).X_max, (*it).nbinsY,
					(*it).Y_min, (*it).Y_max);
		/* then add it to the hist manager */
		ptr->hists.push((*it));
		/* then add it to the folder */
		subfolder = static_cast<TFolder*>
			(all_folder->FindObjectAny(folder_name));
		if (!subfolder)
			subfolder = all_folder->AddFolder(folder_name,
					folder_name);
		subfolder->Add((*it).p_obj);
	}

	int evt_max_len = the_initzer->get_ana_main_buf_sz();
	ptr->p_evt = new unsigned char[evt_max_len];
	
	return 0;
}

int ana_main::handle_msg(uint32_t* msg_body)
{
	/* The message type of the current thread are defined as following 
	 * 1 --> run status transition
	 * 2 --> To be defined yet 
	 * */
	uint32_t msg_type = msg_body[0] & 0xFFFFFF;
	switch (msg_type) {
	case 1:
		/* run status transition
		 * */
		return switch_run(msg_body[1]);
	default:
		return -E_MSG_TYPE;
	}

}

int ana_main::start()
{
	acq_stat = DAQ_RUN;
	return send_msg(1, 1, &acq_stat, 4);
}

int ana_main::stop()
{
	acq_stat = DAQ_STOP;
	return send_msg(3, 1, &acq_stat, 4);
}

int ana_main::quit()
{
	acq_stat = DAQ_EXIT;
	return 0;
}

int ana_main::try_do_analysis(int id, bool& success)
{
	int buf_sz, evt_sz, ret;
	ring_buf* rb = NULL;

	success = false;
	if (id == 1)
		rb = rb_evt;
	if (id == 2)
		rb = rb_scal;

	if (rb->get_lock())
		return -E_SYSCALL;
	buf_sz = rb->get_used1();
	if (buf_sz >= 4) {
		ret = rb->read1(&evt_sz, 4, true);
		evt_sz *= 4;
		/* debug ... */
//		std::cout<<"evt size: "<<evt_sz<<std::endl;
		if (ret < 4) {
			rb->rel_lock();
			return -E_RING_BUF_DATA;
		}
		if (evt_sz <= buf_sz) {
			/* a complete event is in the ring buffer */
			if (evt_sz != rb->read1(p_evt, evt_sz)) {
				rb->rel_lock();
				return -E_RING_BUF_DATA;
			}
			rb->rel_lock();
			ret = do_analysis(p_evt, hists, id);
			success = true;
			RET_IF_NONZERO(ret);
		}
		else {
			rb->rel_lock();
		}
	}
	else {
		rb->rel_lock();
	}

	return 0;
}

int ana_main::main_proc()
{
	/* In the main process, we keep checking the scaler and trigger ring
	 * buffers. If a *complete* event is found, read it out from the ring
	 * buffer and call the user code to analyz.*/
	bool success;
	int ret;

	/* First, let's check the scaler ring buffer */
	ret = try_do_analysis(2, success);
	RET_IF_NONZERO(ret);

	/* sencond, let's check the trigger ring buffer */
	ret = try_do_analysis(1, success);
	RET_IF_NONZERO(ret);
	if (!success)
		usleep(120);

	return 0;
}
