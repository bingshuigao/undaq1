#include "modules.h"
#include "err_code.h"
#include <string.h>
#include <assert.h>

modules::modules()
{
	int i;
	for (i = 0; i < MAX_MODULE; i++) 
		mods[i] = 0;
	name = "";
	crate_n = 0;
}

int get_am_mblt(int am)
{
	if (am == VME_A24_S_DATA)
		return VME_A24_S_MBLT;
	if (am == VME_A24_U_DATA)
		return VME_A24_U_MBLT;
	if (am == VME_A32_S_DATA)
		return VME_A32_S_BLT;
	if (am == VME_A32_U_DATA)
		return VME_A32_U_MBLT;

	return -1;
}

int modules::add_mod(module* mod)
{
	int n;

	/* get the am to be used in (c)mblt */
	int am_mblt = get_am_mblt(mod->get_am());
	if (am_mblt < 0 )
	return -E_AM;

	n = mod->get_slot();
	if (n >= MAX_MODULE)
		return -E_MAX_MODULE;
	mods[n] = mod;
	am_mblts[n] = am_mblt;
	am_cmblts[n] = am_mblt;

	name = mod->get_name();
	crate_n = mod->get_crate();
	return update_read_list();
}

/* get the position of the first zon-zero bit.
 * for example 0x04 ---> 2
 * for example 0x06 ---> 1*/
static inline int get_1st_bit(uint32_t msk)
{
	assert(msk);
	return ffsll(msk) - 1;
}

/* Read the event buffers of all the modules. 
 * @param buf Buffer where the events will be saved.
 * @param sz_in Maximum requested size to read (bytes)
 * @param sz_out Actual size of data read (bytes)
 * @return 0 if succeed, non-zero error code if error */
int modules::read_evtbuf(void* buf, int sz_in, int* sz_out)
{
	int i, ret;
	char* buf1 = static_cast<char*>(buf);
	int tmp_out;
	uint32_t* p_head;


	*sz_out = 0;
	for (i = 0; i < mod_n.size(); i++) {
		p_head = reinterpret_cast<uint32_t*>(buf1);
		buf1 += 8;
		sz_in -= 8;
		switch (read_fun[i]) {
		case 0:
			/* there is really no reason to use the slow and
			 * complecated single word read out, so we don't
			 * implement it by now... */
			return -E_NOT_SUPPORT;
		case 1:
			ret = mods[get_1st_bit(mod_n[i])]->read_evt_blt(am[i], 
				buf1, sz_in, &tmp_out, blt_addr[i], chain[i]);
			break;
		case 2:
			ret = mods[get_1st_bit(mod_n[i])]->read_evt_mblt(am[i], 
				buf1, sz_in, &tmp_out, blt_addr[i], chain[i]);
			break;
		default:
			break;
		}
		buf1 += tmp_out;
		sz_in -= tmp_out;
		p_head[0] = tmp_out + 8;
		p_head[1] = mod_n[i];
		*sz_out += p_head[0];
		/* Bus error is not really an error in case of blt */
		if (ret && ret != -E_VME_BUS)
			return ret;

		if (sz_in <= 0)
			break;
	}

	return on_readout();
}


/* Update the list of parameters used for reading out. These values
 * are determined by the settings of the module such
 * that the fastest way is chosen when reading event buffer.
 * @return 0 if succeed, non-zero error code if error. */
int modules::update_read_list()
{
	/* Clear all the vectors to have a fresh start */
	am.clear();
	dw.clear();
	mod_n.clear();
	read_fun.clear();
	blt_addr.clear();
	chain.clear();

	/* If the vme controller is v2718 */
	if (get_ctl_name() == "v2718") 
		return update_read_list_v2718();
	else
		/* up to now no other vme controllers are supported */
		return -E_NOT_SUPPORT;
}

/* similar as the update_read_list, but this function dues with the
 * case when v2718 is the vme controller, and this function is called
 * by the update_read_list() */
int modules::update_read_list_v2718()
{
	int i, ret;

	for (i = 0; i < MAX_MODULE; i++) {
		int is_first, len;
		
		if (!mods[i])
			continue;
		ret = mods[i]->get_cblt_conf(0, 0, &is_first, 0);
		RET_IF_NONZERO(ret);
		
		/* Not the first module of a cmblt chain, then a normla mblt
		 * readout should be performed */
		if (!is_first) {
			add_mblt(i);
			continue;
		}
		
		/* It is the first module of a cmblt chain, then try to find a
		 * complete chain and get the length.*/
		ret = get_chain_len(i, &len);
		RET_IF_NONZERO(ret);
		if (len > 0) {
			/* a complete chain is found, a cmblt readout should be
			 * performed. */
			uint32_t addr;
			uint16_t addr16;
			ret = mods[i]->get_cblt_conf(&addr16, 0, 0, 0);
			RET_IF_NONZERO(ret);
			addr = addr16;
			addr <<= 24;
			add_cmblt(i, addr, len);
			i += (len-1);
		}
		else {
			/* a complete chain is not found, a
			 * normal mblt should be performed for
			 * the current module. */
			add_mblt(i);
		}
	}
	return 0;
}


/* Add elements to the vectors (am, dw ...) for a mblt readout.
 * @param n Slot number
 **/
void modules::add_mblt(int n)
{
	uint32_t msk = 1;

	am.push_back(am_mblts[n]);
	dw.push_back(64);
	mod_n.push_back(msk<<n);
	read_fun.push_back(2);
	blt_addr.push_back(mods[n]->get_base());
	chain.push_back(0);
}

void modules::add_cmblt(int n, uint32_t addr, int len)
{
	uint32_t msk = 0;
	int i;
	for (i = 0; i < len; i++) {
		uint32_t tmp = 1;
		msk |= tmp << (n+i);
	}


	am.push_back(am_cmblts[n]);
	dw.push_back(64);
	mod_n.push_back(msk);
	read_fun.push_back(2);
	blt_addr.push_back(addr);
	chain.push_back(1);
}

int modules::get_chain_len(int n, int* len)
{
	int i, ret;
	uint32_t addr, addrx;
	uint16_t reg;

	ret = mods[n]->get_cblt_conf(&reg, 0, 0, 0);
	RET_IF_NONZERO(ret);
	addr = reg;
	addr <<= 24;
	
	*len = 1;
	for (i = n+1; i < MAX_MODULE; i++) {
		int cblt_enable, cblt_first, cblt_last;
		/* Check if there are empty slot */
		if (!mods[i]) 
			goto bad_chain;
			
		ret = mods[i]->get_cblt_conf(&reg, &cblt_enable,
				&cblt_first, &cblt_last);
		RET_IF_NONZERO(ret);
		addrx = reg;
		addrx <<= 24;
		if (addrx != addr || !cblt_enable || cblt_first) 
			goto bad_chain;
		/* Check if the module is the last module (if yes, then we've
		 * got a good chain!) */
		if (cblt_last) {
			(*len)++;
			return 0;
		}

		/* If none of the above happens, it is in the middle of a chain
		 * */
		(*len)++;
	}

bad_chain:
	*len = -1;
	return 0;
}
