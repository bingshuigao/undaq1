#include "hist_man.h"
#include "imp_daq.h"
#include "err_code.h"
#include <TH1D.h>
#include <iostream>


int ana_usr_trig(void* p_evt, hist_man& hists, bool is_bor)
{
	uint32_t* p_dw = static_cast<uint32_t*>(p_evt);

	uint32_t len_frag;
	uint32_t len_tot = p_dw[0];
	int i, slot, idx;
	idx = 3;

	std::cout<<"total len: "<<len_tot<<" ts low: "<<p_dw[2]<<" ts hi: "<<p_dw[1]<<std::endl;
	goto end;
begin:
	if (idx == len_tot)
		goto end;
	slot = p_dw[idx++] & 0xFF;
	len_frag = p_dw[idx];
	for (i = 4; i < len_frag; i++) {
		int ch, val, sig, mod_id;
		/* debug ...*/
		sig = p_dw[i+idx] >> 30;
		if (sig == 3) {
			std::cout<<"slot: "<<slot<<" mod id = "<<mod_id<<" ts = "<<(p_dw[i+idx]&0x3fffffff)<<" evt len: "<<len_tot<<std::endl;
			continue;
		}
		else if (sig == 1) {
			mod_id = (p_dw[i+idx]>>16) & 0xFF;
			continue;
		}
		/* **********/
		sig = p_dw[i+idx] >> 21;
		if (sig != 0x20)
			continue;
		//ch = (p_dw[i+idx] >> 16) & 0x1F;
		//val = p_dw[i+idx] & 0x1FFF;
		//if (ch != 31)
			//continue;
		//((TH1D*)hists.get(slot))->Fill(val);
	}
	idx += i;
	goto begin;



end:
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
