#include "hist_man.h"
#include "imp_daq.h"
#include "err_code.h"
#include <TH1D.h>


int ana_usr_trig(void* p_evt, hist_man& hists)
{
	uint32_t* p_dw = static_cast<uint32_t*>(p_evt);

	uint32_t len_dw = p_dw[0];
	int i;
	if (p_dw[1] != 0)
		return 0;
	for (i = 4; i < len_dw; i++) {
		int ch, val;
		int sig = p_dw[i] >> 21;
		if (sig != 0x20)
			continue;
		ch = (p_dw[i] >> 16) & 0x1F;
		val = p_dw[i] & 0x1FFF;
		((TH1D*)hists.get(ch))->Fill(val);
	}

	return 0;
}


int ana_usr_scal(void* p_evt, hist_man& hists)
{return 0;}
