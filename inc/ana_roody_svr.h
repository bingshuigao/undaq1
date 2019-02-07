/* This is the server thread for the roody (histogram displayer). To make
 * things easier, I note the following simplification :
 *     1, The current thread reads the histograms, the ana_main thread writes
 *     the histograms. But the two threads has NO syncronizetion mechenism.
 *     2, The current thread will accept only one roody client to connect. 
 *     3, The current thread never 'stops', it is always in the 'running' (or
 *     exit) status. 
 * Most part of the code is copied from MIDAS.
 *
 * By B.Gao Feb. 2019*/

#ifndef ANA_ROODY_SVR_HHH
#define ANA_ROODY_SVR_HHH

#include "ana_thread.h"

/* root include */
#include <assert.h>
#include <TApplication.h>
#include <TKey.h>
#include <TROOT.h>
#include <TH1.h>
#include <TH2.h>
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

class ana_roody_svr : public ana_thread
{
public:
	ana_roody_svr();
	~ana_roody_svr();

private:
	static int ana_roody_svr_init(my_thread* This, initzer* the_initzer); 
	
	/* implementation of virtual functions declared in base classes. See
	 * comments in base classes for explainations */
	int handle_msg(uint32_t* msg_body);
	int start();
	int stop();
	int quit();
	int main_proc();


	/* connect the roody (do not block). If no connect request is
	 * found, send another message to this thread (type 2). 
	 * Return 0 if succeed, otherwise return error code.
	 * */
	int conn_roody();

private:
	/* listening port of the server */
	int port;

	/* the same as in contrl threads, we need a variable to represent the
	 * 'real' status of the thread. */
	int real_stat;

	/* The connection socket between roody and the current thread */
	TSocket* tsock;

	/* The listening socket. */
	TServerSocket *tlsock;


};




#endif 
