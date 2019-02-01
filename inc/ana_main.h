#ifndef ANA_MAIN_HHH
#define ANA_MAIN_HHH

/* This is the main analysis thread of the analyzer . In this thread, it reads
 * data from ring buffer and call the user code to do the analysis. (In the
 * user code, the histograms are filled, however, the histograms are created by
 * the framework. The user can get the coresponding histogram via the class
 * method hist_man::get(int id) or hist_main::get(const char* name))
 *
 * By B.Gao Jan. 2019 */

#include "ana_thread.h"
#include "hist_man.h"
#include <string>

/* root includes */
#include <assert.h>
#include <TApplication.h>
#include <TKey.h>
#include <TROOT.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TF1.h>
#include <TFile.h>
#include <TTree.h>
#include <TLeaf.h>
#include <TSocket.h>
#include <TServerSocket.h>
#include <TMessage.h>
#include <TObjString.h>
#include <TSystem.h>
#include <TFolder.h>
#include <TRint.h>
#include <TCutG.h>
#include <TThread.h>


/* declear the user code function, it should be implemented somewhere in a user
 * code source file. */
int ana_usr_trig(void* p_evt, hist_man& hists);
int ana_usr_scal(void* p_evt, hist_man& hists);

class ana_main : public ana_thread
{
public:
	ana_main();
	~ana_main();

private:
	static int ana_main_init(my_thread* This, initzer* the_initzer); 

	/* implementation of virtual functions declared in base classes. See
	 * comments in base classes for explainations */
	int handle_msg(uint32_t* msg_body);
	int start();
	int stop();
	int quit();
	int main_proc();

	/* this function calls the user code and do the analysis
	 * return 0 if succeed, otherwise return error code 
	 * @ p_evt pointer to the buffer containing the event data
	 * @ hists use this to get the histograms to fill.
	 * @ id 1-> trigger data, 2-> scaler data.
	 * */
	int do_analysis(void* p_evt, hist_man& hists, int id) 
	{
		if (id == 1)
			return ana_usr_trig(p_evt, hists);
		else if (id == 2)
			return ana_usr_scal(p_evt, hists);
		return -E_GENERIC;
	}

	/* try to analysis the event from ring buffers, it may fail which
	 * doesn't mean error. if the ring buffer is empty it fails.
	 * @param id :1->trigger data, 2->scaler data;
	 * @param success: true if succeed, false if failed (because of empty
	 * ring buffer).
	 * return 0 if succeed, otherwise return error code 
	 * */
	int try_do_analysis(int id, bool& success);


private:
	/* the histogram manager, to let the user get their histograms from the
	 * user code function. */
	hist_man hists;

	/* the buffer containing the event data */
	unsigned char* p_evt;
};



#endif 
