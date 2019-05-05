#include "hist_man.h"
#include "imp_daq.h"
#include "err_code.h"
#include <TH1D.h>
#include <iostream>


int ana_usr_trig(void* p_evt, hist_man& hists)
{
	uint32_t* p_dw = static_cast<uint32_t*>(p_evt);

	uint32_t len_dw;
	uint32_t len_tot = p_dw[0];
	int i, slot, idx;
	if (p_dw[1] != 0)
		return 0;
	idx = 3;
begin:
	if (idx == len_tot)
		goto end;
	slot = p_dw[idx++] & 0xFF;
	len_dw = p_dw[idx];
	for (i = 0; i < len_dw; i++) {
		int ch, val, sig, mod_id;
		/* debug ...*/
		sig = p_dw[i+idx] >> 30;
		if (sig == 3) {
			std::cout<<"mod id = "<<mod_id<<"ts = "<<(p_dw[i]&0x3fffffff)<<" evt len: "<<len_tot<<std::endl;
			continue;
		}
		else if (sig == 1) {
			mod_id = (p_dw[i]>>16) & 0xFF;
			continue;
		}
		/* **********/
		sig = p_dw[i] >> 21;
		if (sig != 0x20)
			continue;
		ch = (p_dw[i] >> 16) & 0x1F;
		val = p_dw[i] & 0x1FFF;
		if (ch != 31)
			continue;
		((TH1D*)hists.get(slot))->Fill(val);
	}
	idx += i;
	goto begin;



end:
	return 0;
}


int ana_usr_scal(void* p_evt, hist_man& hists)
{return 0;}
