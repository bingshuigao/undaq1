#include "hist_man.h"
#include "imp_daq.h"
#include "err_code.h"
#include "ana_evt_hd.h"
#include "ana_frag_hd.h"
#include "ana_madc32.h"
#include <TH1D.h>
#include <iostream>


ana_evt_hd evt_hd;
ana_frag_hd frag_hd;
ana_madc32 evt_madc;

int ana_usr_trig(void* p_evt, hist_man& hists, bool is_bor)
{
	uint32_t* p_dw1 = static_cast<uint32_t*>(p_evt);
	uint32_t* p_dw = p_dw1;
	uint32_t len_tot, len_frag;
	uint32_t madc_val[32];
	int slot;

	/* parse event header */
	evt_hd.parse_hd(p_dw);
	len_tot = evt_hd.get_len();

	/* loop the fragments */
	p_dw += 3;
	while (p_dw - p_dw1 != len_tot) {
		/* parse fragment header */
		frag_hd.parse_hd(p_dw);
		p_dw += 5;
		slot = frag_hd.get_slot();
		len_frag = frag_hd.get_len();
		if (slot == 4) {
			/* madc */
			evt_madc.parse_raw(p_dw, madc_val);
			((TH1D*)hists.get(0))->Fill(madc_val[0]);

		}
		else if (slot == 3) {
			/* v1190, for example */
		}

		p_dw -= 5;
		p_dw += len_frag+1;
	} 
	
	return 0;
}


int ana_usr_scal(void* p_evt, hist_man& hists, bool is_bor)
{
	uint32_t* p_dw = static_cast<uint32_t*>(p_evt);
	uint32_t len_w = p_dw[0]/4;
	//std::cout<<"lenth: "<<p_dw[1]<<" ts: "<<p_dw[5]<<std::endl;
	int i;
	std::cout<<"header-----------"<<std::endl;
	for (i = 0; i < 6; i++) {
		std::cout<<i<<"   "<<p_dw[i]<<std::endl;
	}

	std::cout<<"-------------------"<<std::endl;
	if (len_w == 41) {
		for (i = 0; i < 32; i++) {
			std::cout<<"ch: "<<i<<" counts: "<<p_dw[9+i]<<std::endl;
		}
	}

	return 0;
}
