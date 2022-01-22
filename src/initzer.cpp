#include "initzer.h"
#include <string>
#include "BigInteger.h"
#include <stdlib.h>
#include <algorithm>
#include <string.h>
#include <unistd.h>

/* This function decodes the raw string to another string. The raw string
 * represents a very long integer like
 * '123456789998748578374847447483837363734638'. This integer is used to
 * represent a string. It is generated according to the ascii codes of the
 * charactors in the string. See file: adv_conf_fe.py (the comment).
 * It's not trivial to deal with big integers. What we do here is to convert
 * the big integer into hexdecimal form (by dividing 16 and get the remainder),
 * then get convert each byte into its charactor value. We do the dividings by
 * using the lib from https://github.com/panks/BigInteger. */
static std::string decode_str(std::string& raw)
{
	std::string ret("");
	std::string hex_int("");
	BigInteger big_int(raw);
	int i;
	
	while (big_int > 0) {
		BigInteger rem = big_int % 16;
		if (rem < 10)
			hex_int += string(rem);
		else if (rem == 10)
			hex_int += "A";
		else if (rem == 11)
			hex_int += "B";
		else if (rem == 12)
			hex_int += "C";
		else if (rem == 13)
			hex_int += "D";
		else if (rem == 14)
			hex_int += "E";
		else if (rem == 15)
			hex_int += "F";
		big_int /= 16;
	}
	/* if the raw string is already in hexdecimal form, give up the above
	 * conversion, and take it from the raw string */
	if (raw[0] == '0' && raw[1] == 'x') {
		hex_int = "";
		for (i = raw.size()-1; i > 1; i--) 
			hex_int += raw[i];
	}
	
	if (hex_int.size() % 2)
		hex_int += "0";
	/* Now the hex_int has even number of charactors, two of which form a
	 * byte */
	char hex_byte[3];
	char char_ascii[2];
	hex_byte[2] = 0;
	char_ascii[1] = 0;
	for (i = 0; i < hex_int.size()/2; i++) {
		hex_byte[0] = hex_int[i*2+1];
		hex_byte[1] = hex_int[i*2];
		char_ascii[0] = strtol(reinterpret_cast<const char*>(hex_byte), 
				NULL, 16);
		ret += reinterpret_cast<const char*>(char_ascii);
	}

	return ret;
}

initzer::initzer()
{
	p_parser = 0;
	vme_mod_inited = false;
	slot_map_inited = false;
	memset(slot_map, 0, MAX_SLOT_MAP);
	clk_map_inited = false;
	clk_off_map_inited = false;
	memset(clk_map, 0, MAX_CLK_MAP*8);
	memset(clk_off_map, 0, MAX_CLK_OFF_MAP*8);
	rb_fe = NULL;
	rb_fe2 = NULL;
	rb_ebd2 = NULL;
	rb_ebd3 = NULL;
	rb_ebd5 = NULL;
	rb_log0 = NULL;
	rb_log1 = NULL;
	rb_log2 = NULL;
	rb_ana0 = NULL;
	rb_ana1 = NULL;
	rb_ana2 = NULL;
}

initzer::~initzer()
{
/*
	int i, n;

	n = p_module.size();
	for (i = 0; i < n; i++) 
		if (p_module[i])
			delete p_module[i];
*/
}

#ifdef MAKE_FRONTEND

int initzer::get_mod_rbs()
{
	std::vector<modules*> list_mod;
	struct mod_rb_par mod_rb_par;
	
	/* check if the ring buffers are already initiated */
	if (rbs_ebd.size() > 0) 
		return 0;

	/* create and initilize the ring buffers */
	if (get_modules('T', list_mod))
		return 0;
	for (auto it = list_mod.begin(); it != list_mod.end(); it++) {
		module** mods = (*it)->get_modules();
		int i;
		for (i = 0; i < MAX_MODULE; i++) {
			module* p_mod = mods[i];
			if (!p_mod)
				continue;
			
			/* a vme module is found! */
			mod_rb_par.crate = p_mod->get_crate();
			mod_rb_par.slot = p_mod->get_slot();
			rbs_ebd.push_back(mod_rb_par);
		}
	}

	return 0;
}
std::string initzer::get_fe_ctl_svr_addr()
{
	bool found;
	std::string name("ctl_svr_addr");
	std::string addr;

	get_fe_adv_var(name, found, &addr);
	if (!found)
		addr = DEF_SVR_CTL_FE;
	else
		addr = decode_str(addr);

	return addr;
}
int initzer::get_fe_on_start_t_max()
{
	bool found;
	std::string name("rd_start_t");
	int port;

	port = get_fe_adv_var(name, found);
	if (found)
		return port;
	else
		return DEF_RD_ON_START; 
}

int initzer::get_fe_ctl_buf_sz()
{
	bool found;
	std::string name("ctl_buf_sz");
	int port;

	port = get_fe_adv_var(name, found);
	if (found)
		return port;
	else
		return DEF_SOCK_BUF_FE_CTL; 
}

int initzer::get_fe_ctl_t_us()
{
	bool found;
	std::string name("ctl_t_us");
	int port;

	port = get_fe_adv_var(name, found);
	if (found)
		return port;
	else
		return DEF_T_US_CTL_FE;

}
int initzer::get_fe_buf_sz(int rb_id)
{
	bool found;
	std::string name("ring_buf_sz");
	int sz;

	switch (rb_id) {
	case 1:
		name = "ring_buf_sz";
		break;
	case 2:
		name = "ring_buf2_sz";
		break;
	}

	sz = get_fe_adv_var(name, found);
	if (found)
		return sz;
	else
		return 0;
}

ring_buf* initzer::get_fe_rb(int rb_id)
{
	uint32_t sz2;
	std::string var_name;
	ring_buf* p_rb;

	sz2 = get_fe_buf_sz(rb_id);
	switch (rb_id) {
	case 1:
		/* first check if the ring buffer object already created */
		RET_IF_NONZERO(rb_fe);
		if (sz2 == 0)
			sz2 = DEF_RB_FE_DATA;
		break;
	case 2:
		/* first check if the ring buffer object already created */
		RET_IF_NONZERO(rb_fe2);
		if (sz2 == 0)
			sz2 = DEF_RB_FE_MSG;
		break;
	default:
		return NULL;
	}
	if (!p_parser)
		return NULL;

	p_rb = new ring_buf;
	if (p_rb->init(sz2)) {
		delete p_rb;
		return NULL;
	}
	
	switch (rb_id) {
	case 1:
		rb_fe = p_rb;
		break;
	case 2:
		rb_fe2 = p_rb;
		break;
	}
	return p_rb;
}

module* initzer::get_trig_mod()
{
	if (!vme_mod_inited) {
		if (init_vme_mod())
			return NULL;
		vme_mod_inited = true;
	}

	for (auto it = p_module.begin(); it != p_module.end(); it++) {
		if ((*it)->get_trig_mod()) 
			return *it;
	}

	return NULL;
}

int initzer::get_fe_ntry()
{
	bool found;
	std::string name("n_try");
	int n_try;

	n_try = get_fe_adv_var(name, found);
	if (found)
		return n_try;
	else
		return DEF_NTRY_FE;
}


int initzer::get_fe_sender_itv()
{
	bool found;
	int itv;
	std::string name("sender_itv");

	itv = get_fe_adv_var(name, found);
	if (found)
		return itv;
	else
		return DEF_ITV_SEND_FE;
}

/* allocate new module, return 0 if succeed, otherwise return error code */
static int create_mod(std::string& name, module*& mod)
{
	if (name.find("MADC32") != std::string::npos) {
		mod = new madc32;
	}
	else if (name.find("MQDC32") != std::string::npos) {
		mod = new mqdc32;
	}
	else if (name.find("V1190A") != std::string::npos) {
		mod = new v1190; 
	}
	else if (name.find("V830") != std::string::npos) {
		mod = new v830;
	}
	else if (name.find("V1740") != std::string::npos) {
		mod = new v1740;
	}
	else if (name.find("PIXIE16_MOD") != std::string::npos) {
		mod = new pixie16;
	}
	else if (name.find("V1751") != std::string::npos) {
		mod = new v1751;
	}
	else if (name.find("FAKE_MODULE") != std::string::npos) {
		mod = new fake_module;
	}
	else if (name.find("V775") != std::string::npos) {
		if (name.find("V775N") == std::string::npos)
			mod = new v775;
		else
			mod = new v775n;
	}
	else if (name.find("V785") != std::string::npos) {
		if (name.find("V785N") == std::string::npos)
			mod = new v785;
		else
			mod = new v785n;
	}
	else if (name.find("V792") != std::string::npos) {
		if (name.find("V792N") == std::string::npos)
			mod = new v792;
		else
			return -E_NOT_IMPLE;
//			mod = new v792n;
	}
	else {
		if ((name.find("V2718") == std::string::npos) &&
		    (name.find("TEST_CTL") == std::string::npos) &&
		    (name.find("PIXIE16_CTL") == std::string::npos)) {
			/* unknown module found */
//			std::cout<<"errorxxx"<<std::endl<<name<<std::endl;;
			return -E_UNKOWN_MOD;
		}
		else
			/* vme controller module should not be created here */
			mod = NULL;
	}

	return 0;
}

static int get_v830_reg_dmod(uint32_t off)
{
	if (off == 0x1200)
		return 32;
	if (off == 0x1104)
		return 32;
	if (off == 0x1100)
		return 32;
	if (off <= 0x1080)
		return 32;
	return 16;
}


/* initialize vme module, return 0 if succeed, otherwise return error code */
static int 
do_init_v830(v830* mod, std::vector<struct conf_vme_mod> &the_conf)
{
	uint16_t reg_val = 0;
	/* reset */
	if (mod->write_reg(0x1120, 16, &reg_val))
		return -E_INIT_V830;
	for (auto it = the_conf.begin(); it != the_conf.end(); it++) {
		if ((*it).name != "")
			continue;
		/* this is a register setting */
		uint32_t off = (*it).offset;
		uint16_t val16 = (*it).val.val_uint64;
		uint32_t val32 = (*it).val.val_uint64;
		if ((off >= 0x1000) && (off <= 0x1132)) {
			/* this is a physical register */
			int dw = get_v830_reg_dmod(off);
			void *p_data;
			if (dw == 32) 
				p_data = &val32;
			else
				p_data = &val16;
			if (mod->write_reg(off, dw, p_data))
				return -E_INIT_V830;
		}
		else {
			/* unknown register */
			return -E_INIT_V830;
		}
	}
	
	return 0;
}

/* set v1751 dc offset */
static int set_v1751_dc(v1751* mod, uint32_t off, uint32_t val)
{
	int ret;
	uint32_t val1;
	uint32_t off1 = 0x1088 + (off & 0xf00);
	do {
		ret = mod->read_reg(off1, 32, &val1);
		RET_IF_NONZERO(ret);
	} while (val1 & 0x4);
	ret = mod->write_reg(off, 32, &val);
	RET_IF_NONZERO(ret);

	return 0;
}


/* set v1740 dc offset */
static int set_v1740_dc(v1740* mod, uint32_t off, uint32_t val)
{
	int ret;
	uint32_t val1;
	uint32_t off1 = 0x1088 + (off & 0xf00);
	do {
		ret = mod->read_reg(off1, 32, &val1);
		RET_IF_NONZERO(ret);
	} while (val1 & 0x4);
	ret = mod->write_reg(off, 32, &val);
	RET_IF_NONZERO(ret);

	return 0;
}


/* initialize vme module, return 0 if succeed, otherwise return error code */
static int 
do_init_v785(v785* mod, std::vector<struct conf_vme_mod> &the_conf)
{
	uint16_t dum = 0x80;
	/* First, we need to soft-reset all settings */
	if (mod->write_reg(0x1006, 16, &dum))
		return -E_INIT_V785;
	if (mod->write_reg(0x1008, 16, &dum))
		return -E_INIT_V785;
	/* then init all registers with non-default values */
	for (auto it = the_conf.begin(); it != the_conf.end(); it++) {
		if ((*it).name != "") 
			continue;
		/* this is a register setting */
		uint32_t off = (*it).offset;
		uint16_t val = (*it).val.val_uint64;
		if ((off >= 0x1000) && (off <= 0x10be)) {
			/* this is a physical register */
			if (mod->write_reg(off, 16, &val))
				return -E_INIT_V785;
		}
		else {
			/* unknown register */
			return -E_INIT_V785;
		}
	}

	/* lastly, init other register that use non-default values but not
	 * exposed to the user in the config */
	/* enable BERR_ENABLE */
	dum = 0x20;
	if (mod->write_reg(0x1010, 16, &dum))
		return -E_INIT_V785;

	return 0;
}

/* initialize vme module, return 0 if succeed, otherwise return error code */
static int 
do_init_v792(v792* mod, std::vector<struct conf_vme_mod> &the_conf)
{
	uint16_t dum = 0x80;
	/* First, we need to soft-reset all settings */
	if (mod->write_reg(0x1006, 16, &dum))
		return -E_INIT_V792;
	if (mod->write_reg(0x1008, 16, &dum))
		return -E_INIT_V792;
	/* then init all registers with non-default values */
	for (auto it = the_conf.begin(); it != the_conf.end(); it++) {
		if ((*it).name != "") 
			continue;
		/* this is a register setting */
		uint32_t off = (*it).offset;
		uint16_t val = (*it).val.val_uint64;
		if ((off >= 0x1000) && (off <= 0x10be)) {
			/* this is a physical register */
			if (mod->write_reg(off, 16, &val))
				return -E_INIT_V792;
		}
		else {
			/* unknown register */
			return -E_INIT_V792;
		}
	}

	/* lastly, init other register that use non-default values but not
	 * exposed to the user in the config */
	/* enable BERR_ENABLE */
	dum = 0x20;
	if (mod->write_reg(0x1010, 16, &dum))
		return -E_INIT_V792;

	return 0;
}



/* initialize vme module, return 0 if succeed, otherwise return error code */
static int 
do_init_v775(v775* mod, std::vector<struct conf_vme_mod> &the_conf)
{
	uint16_t dum = 0x80;
	/* First, we need to soft-reset all settings */
	if (mod->write_reg(0x1006, 16, &dum))
		return -E_INIT_V775;
	if (mod->write_reg(0x1008, 16, &dum))
		return -E_INIT_V775;
	/* then init all registers with non-default values */
	for (auto it = the_conf.begin(); it != the_conf.end(); it++) {
		if ((*it).name != "") 
			continue;
		/* this is a register setting */
		uint32_t off = (*it).offset;
		uint16_t val = (*it).val.val_uint64;
		if ((off >= 0x1000) && (off <= 0x10be)) {
			/* this is a physical register */
			if (mod->write_reg(off, 16, &val))
				return -E_INIT_V775;
		}
		else {
			/* unknown register */
			return -E_INIT_V775;
		}
	}

	/* lastly, init other register that use non-default values but not
	 * exposed to the user in the config */
	/* enable BERR_ENABLE */
	dum = 0x20;
	if (mod->write_reg(0x1010, 16, &dum))
		return -E_INIT_V775;

	return 0;
}


/* initialize vme module, return 0 if succeed, otherwise return error code */
static int 
do_init_fake_module(fake_module* mod, std::vector<struct conf_vme_mod> &the_conf)
{
	return 0;
}

/* calibrate v1751 */
static int calib_v1751(v1751* mod)
{
	int ret, i;
	uint32_t val1, calib;
	uint32_t off;

	off = 0x809c;
	calib = 0;
	ret = mod->write_reg(off, 32, &calib);
	calib = 2;
	ret = mod->write_reg(off, 32, &calib);

	off = 0x1088;
	for (i = 0; i < 8; i++) {
		off += 0x100*i;
		do {
			ret = mod->read_reg(off, 32, &val1);
			RET_IF_NONZERO(ret);
		} while (!(val1 & 0x40));
	}
	
	off = 0x809c;
	calib = 0;
	ret = mod->write_reg(off, 32, &calib);

	return 0;
}


/* initialize vme module, return 0 if succeed, otherwise return error code */
static int 
do_init_v1751(v1751* mod, std::vector<struct conf_vme_mod> &the_conf)
{
	uint32_t dum = 0;
	/* First, we need to soft-reset all settings */
	if (mod->write_reg(0xef24, 32, &dum))
		return -E_INIT_V1751;
	/* then init all registers with non-default values */
	for (auto it = the_conf.begin(); it != the_conf.end(); it++) {
		if ((*it).name != "") 
			continue;
		/* this is a register setting */
		uint32_t off = (*it).offset;
		uint32_t val = (*it).val.val_uint64;
		if ((off >> 12) == 1 && (off & 0xff) == 0x98) {
			/* if off == 0x1n98, additional check should be made
			 * before writing, the same as v1740 */
			if (set_v1751_dc(mod, off, val))
				return -E_INIT_V1751;
		}
		else {
			if (mod->write_reg(off, 32, &val))
				return -E_INIT_V1751;
		}
	}
	/* calibrate */
	usleep(200000);
	calib_v1751(mod);
	return 0;
}



/* initialize vme module, return 0 if succeed, otherwise return error code */
static int 
do_init_v1740(v1740* mod, std::vector<struct conf_vme_mod> &the_conf)
{
	uint32_t dum = 0;
	/* First, we need to soft-reset all settings */
	if (mod->write_reg(0xef24, 32, &dum))
		return -E_INIT_V1740;
	/* then init all registers with non-default values */
	for (auto it = the_conf.begin(); it != the_conf.end(); it++) {
		if ((*it).name != "") 
			continue;
		/* this is a register setting */
		uint32_t off = (*it).offset;
		uint32_t val = (*it).val.val_uint64;
		if ((off >> 12) == 1 && (off & 0xff) == 0x98) {
			/* if off == 0x1n98, additional check should be made
			 * before writing*/
			if (set_v1740_dc(mod, off, val))
				return -E_INIT_V1740;
		}
		else {
			if (mod->write_reg(off, 32, &val))
				return -E_INIT_V1740;
		}
	}
	return 0;
}


/* initialize vme module, return 0 if succeed, otherwise return error code */
static int 
do_init_mqdc32(mqdc32* mod, std::vector<struct conf_vme_mod> &the_conf)
{
	/* special attention should be paid to the 0x6090 register. If set to
	 * 0xc, one must first send a pulse to the fc/res port before writing
	 * 0xc to this register, or else the result of the register will be 8
	 * instead of 0xc (a bug???) */
	uint16_t dum = 0;
	/* First, we need to soft-reset all settings */
	if (mod->write_reg(0x6008, 16, &dum))
		return -E_INIT_MQDC32;
	usleep(1200000);
	/* then init all registers with non-default values */
	for (auto it = the_conf.begin(); it != the_conf.end(); it++) {
		if ((*it).name != "") 
			continue;
		/* this is a register setting */
		uint32_t off = (*it).offset;
		uint16_t val = (*it).val.val_uint64;
		if (((off >= 0x4000) && (off <= 0x4000+32*2)) ||
		    ((off >= 0x6000) && (off <= 0x60b0))) {
			/* this is a physical register */
			if (mod->write_reg(off, 16, &val))
				return -E_INIT_MQDC32;
		}
		else {
			/* unknown register */
			return -E_INIT_MQDC32;
		}
	}
	return 0;
}


/* initialize pixie16 module, return 0 if succeed, otherwise return error code.
 * One should note that in our DAQ framework, each channel of a pixie16 module
 * is defined as a pixie16 module.  */
static int 
do_init_pixie16(pixie16* mod, std::vector<struct conf_vme_mod> &the_conf)
{
	/* The 'the_conf' contains parameters for the present channel as well
	 * as parameters for the pixie16 module. Since the module parameters
	 * are the same for different channels, we have 16 copies of the module
	 * parameter, which (I know) causes some redundancy. These module
	 * parameters will be configured 16 times instead of once, which may
	 * take slightly longer time in the initialization. */

	/* make sure we set the module number correctly before doing other
	 * register read/write operations. Because the module number is used in
	 * many read/write operations. */
	int auto_blcut;
	uint32_t off;
	uint64_t val;
	int dw = 0;

	for (auto it = the_conf.begin(); it != the_conf.end(); it++) {
		if ((*it).name != "") 
			continue;
		/* this is a register setting */
		off = (*it).offset;
		val = (*it).val.val_uint64;
		if (off == 0) {
			mod->set_mod_num(val);
			break;
		}
	}

	/* now configure the parameters */
	for (auto it = the_conf.begin(); it != the_conf.end(); it++) {
		if ((*it).name != "") 
			continue;
		/* this is a register setting */
		off = (*it).offset;
		val = (*it).val.val_uint64;
		if (mod->write_reg(off, dw, &val))
			return -E_INIT_PIXIE16;
		if (off == 87)
			auto_blcut = val;
	}
	/* perform the auto bl cut again. Maybe its not necessary, but it won't
	 * harm anyway. */
	if (auto_blcut) {
		off = 87;
		val = 1;
		if (mod->write_reg(off, dw, &val))
			return -E_INIT_PIXIE16;
	}

	/* need to set slotID/CrateID (which is reported in the data structure)
	 * */
	if (mod->set_dsp_slot_id(mod->get_crate()))
		return -E_INIT_PIXIE16;
	if (mod->set_dsp_crate_id(0))
		return -E_INIT_PIXIE16;


	/* we set the first channel of the first pixie16 module as the trigger
	 * module and set the threshold */
	if ((mod->get_slot() == 0) && (mod->get_mod_num() == 0)) {
		uint32_t thresh = 
			((pixie16_ctl*)mod->get_ctl())->get_fifo_thresh();
		mod->set_trig_mod(1);
		mod->set_fifo_thresh(thresh);
	}

	return 0;
}


/* initialize vme module, return 0 if succeed, otherwise return error code */
static int 
do_init_madc32(madc32* mod, std::vector<struct conf_vme_mod> &the_conf)
{
	/* special attention should be paid to the 0x6090 register. If set to
	 * 0xc, one must first send a pulse to the fc/res port before writing
	 * 0xc to this register, or else the result of the register will be 8
	 * instead of 0xc (a bug???) */
	uint16_t dum = 0;
	/* First, we need to soft-reset all settings */
	if (mod->write_reg(0x6008, 16, &dum))
		return -E_INIT_MADC32;
	/* then init all registers with non-default values */
	for (auto it = the_conf.begin(); it != the_conf.end(); it++) {
		if ((*it).name != "") 
			continue;
		/* this is a register setting */
		uint32_t off = (*it).offset;
		uint16_t val = (*it).val.val_uint64;
		if (((off >= 0x4000) && (off <= 0x4000+32*2)) ||
		    ((off >= 0x6000) && (off <= 0x60b0))) {
			/* this is a physical register */
			if (mod->write_reg(off, 16, &val))
				return -E_INIT_MADC32;
		}
		else {
			/* unknown register */
			return -E_INIT_MADC32;
		}
	}
	return 0;
}

static int get_v1190_reg_dmod(uint32_t off)
{
	if (off == 0x101c)
		return 32;
	if (off == 0x1028)
		return 32;
	if (off == 0x1038)
		return 32;
	if (off == 0x1200)
		return 32;
	return 16;
}

/* set the micro registers of the v1190 module. The meanings of each offset can
 * be found in file v1190.py.
 * Return 0 if succeed, otherwise return error code. */
static int set_v1190_micro(v1190* mod, uint32_t off, uint64_t val)
{
	uint16_t ope_code;
	uint16_t p_pars[41];
	int ret;

	switch (off) {
	case 0:
		if (val == 0)
			ope_code = 0x0000;
		else
			ope_code = 0x0100;
		ret = mod->write_micro(ope_code, p_pars, 0);
		RET_IF_NONZERO(ret);
		break;
	case 1:
		ope_code = 0x1000;
		p_pars[0] = val;
		ret = mod->write_micro(ope_code, p_pars, 1);
		RET_IF_NONZERO(ret);
		break;
	case 2:
		ope_code = 0x1100;
		p_pars[0] = val;
		ret = mod->write_micro(ope_code, p_pars, 1);
		RET_IF_NONZERO(ret);
		break;
	case 3:
		ope_code = 0x1200;
		p_pars[0] = val;
		ret = mod->write_micro(ope_code, p_pars, 1);
		RET_IF_NONZERO(ret);
		break;
	case 4:
		ope_code = 0x1300;
		p_pars[0] = val;
		ret = mod->write_micro(ope_code, p_pars, 1);
		RET_IF_NONZERO(ret);
		break;
	case 5:
		if (val == 0)
			ope_code = 0x1400;
		else
			ope_code = 0x1500;
		ret = mod->write_micro(ope_code, p_pars, 0);
		RET_IF_NONZERO(ret);
		break;
	case 6:
		ope_code = 0x2200;
		p_pars[0] = val;
		ret = mod->write_micro(ope_code, p_pars, 1);
		RET_IF_NONZERO(ret);
		break;
	case 7:
		ope_code = 0x2400;
		p_pars[0] = val;
		ret = mod->write_micro(ope_code, p_pars, 1);
		RET_IF_NONZERO(ret);
		break;
	case 8:
		ope_code = 0x2500;
		p_pars[0] = val;
		ret = mod->write_micro(ope_code, p_pars, 1);
		RET_IF_NONZERO(ret);
		break;
	case 9:
		ope_code = 0x2800;
		p_pars[0] = val;
		ret = mod->write_micro(ope_code, p_pars, 1);
		RET_IF_NONZERO(ret);
		break;
	case 10:
		if (val == 0)
			ope_code = 0x3000;
		else
			ope_code = 0x3100;
		ret = mod->write_micro(ope_code, p_pars, 0);
		RET_IF_NONZERO(ret);
		break;
	case 11:
		ope_code = 0x3300;
		p_pars[0] = val;
		ret = mod->write_micro(ope_code, p_pars, 1);
		RET_IF_NONZERO(ret);
		break;
	case 12:
		if (val == 0)
			ope_code = 0x3500;
		else
			ope_code = 0x3600;
		ret = mod->write_micro(ope_code, p_pars, 0);
		RET_IF_NONZERO(ret);
		break;
	case 13:
		if (val == 0)
			ope_code = 0x3700;
		else
			ope_code = 0x3800;
		ret = mod->write_micro(ope_code, p_pars, 0);
		RET_IF_NONZERO(ret);
		break;
	case 14:
		ope_code = 0x3900;
		p_pars[0] = val;
		ret = mod->write_micro(ope_code, p_pars, 1);
		RET_IF_NONZERO(ret);
		break;
	case 15:
		ope_code = 0x3b00;
		p_pars[0] = val;
		ret = mod->write_micro(ope_code, p_pars, 1);
		RET_IF_NONZERO(ret);
		break;
	case 16:
		ope_code = 0x4500;
		ret = mod->read_micro(ope_code, p_pars, 8);
		RET_IF_NONZERO(ret);
		p_pars[0] = val & 0xFFFF;
		p_pars[1] = val >> 16;
		ope_code = 0x4400;
		ret = mod->write_micro(ope_code, p_pars, 8);
		RET_IF_NONZERO(ret);
	case 17:
		ope_code = 0x4500;
		ret = mod->read_micro(ope_code, p_pars, 8);
		RET_IF_NONZERO(ret);
		p_pars[2] = val & 0xFFFF;
		p_pars[3] = val >> 16;
		ope_code = 0x4400;
		ret = mod->write_micro(ope_code, p_pars, 8);
		RET_IF_NONZERO(ret);
	case 18:
		ope_code = 0x4500;
		ret = mod->read_micro(ope_code, p_pars, 8);
		RET_IF_NONZERO(ret);
		p_pars[4] = val & 0xFFFF;
		p_pars[5] = val >> 16;
		ope_code = 0x4400;
		ret = mod->write_micro(ope_code, p_pars, 8);
		RET_IF_NONZERO(ret);
	case 19:
		ope_code = 0x4500;
		ret = mod->read_micro(ope_code, p_pars, 8);
		RET_IF_NONZERO(ret);
		p_pars[6] = val & 0xFFFF;
		p_pars[7] = val >> 16;
		ope_code = 0x4400;
		ret = mod->write_micro(ope_code, p_pars, 8);
		RET_IF_NONZERO(ret);
	case 20:
		ope_code = 0x5000;
		p_pars[0] = val & 0xFFFF;
		p_pars[1] = val >> 16;
		ret = mod->write_micro(ope_code, p_pars, 2);
		RET_IF_NONZERO(ret);
		break;
	case 100 ... 227:
		ope_code = 0x5200;
		ope_code += off - 100;
		p_pars[0] = val;
		ret = mod->write_micro(ope_code, p_pars, 1);
		RET_IF_NONZERO(ret);
		break;
	default:
		/* unknown offset value */
		return -E_V1190_OFFSET;
	}

	return 0;
}

/* initialize vme module, return 0 if succeed, otherwise return error code */
static int 
do_init_v1190(v1190* mod, std::vector<struct conf_vme_mod> &the_conf)
{
	/* first reset all registers */
	uint16_t reg_val = 0;
	if (mod->write_reg(0x1014, 16, &reg_val))
		return -E_INIT_V1190;
	/* then init all the registers based on config file settings */
	for (auto it = the_conf.begin(); it != the_conf.end(); it++) {
		if ((*it).name != "") 
			continue;
		/* this is a register setting */
		uint32_t off = (*it).offset;
		uint64_t val = (*it).val.val_uint64;
		if ((off >= 0x1000) && (off <= 0x1204)) {
			/* this is a physical register */
			int dw = get_v1190_reg_dmod(off);
			uint32_t val_32 = val;
			uint16_t val_16 = val;
			void *p_data;
			if (dw == 32) 
				p_data = &val_32;
			else
				p_data = &val_16;
			if (mod->write_reg(off, dw, p_data))
				return -E_INIT_V1190;
		}
		else if ((off >= 0) && (off <= 228)){
			/* this is a micro register setting */
			if (set_v1190_micro(mod, off, val))
				return -E_INIT_V1190;
		}
		else if (off == 500) {
			/* control bit set */
			uint16_t ctrl_reg;
			if (mod->read_reg(0x1000, 16, &ctrl_reg))
				return -E_INIT_V1190;
			ctrl_reg |= val;
			if (mod->write_reg(0x1000, 16, &ctrl_reg))
				return -E_INIT_V1190;
		}
		else if (off == 501) {
			/* control bit clear */
			uint16_t short ctrl_reg;
			if (mod->read_reg(0x1000, 16, &ctrl_reg))
				return -E_INIT_V1190;
			ctrl_reg &= (~val);
			if (mod->write_reg(0x1000, 16, &ctrl_reg))
				return -E_INIT_V1190;
		}
		else {
			/* unknown register */
			return -E_INIT_V1190;
		}
	}

	return 0;
}


/* initialize vme module, return 0 if succeed, otherwise return error code */
static int do_init_mod(module* mod, std::vector<struct conf_vme_mod> &the_conf)
{
	std::string name = mod->get_name();
	if (name == "madc32")
		return do_init_madc32(static_cast<madc32*>(mod), the_conf);
	if (name == "mqdc32")
		return do_init_mqdc32(static_cast<mqdc32*>(mod), the_conf);
	if (name == "v1190")
		return do_init_v1190(static_cast<v1190*>(mod), the_conf);
	if (name == "v830")
		return do_init_v830(static_cast<v830*>(mod), the_conf);
	if (name == "v1740")
		return do_init_v1740(static_cast<v1740*>(mod), the_conf);
	if (name == "v1751")
		return do_init_v1751(static_cast<v1751*>(mod), the_conf);
	if (name == "fake_module")
		return do_init_fake_module(static_cast<fake_module*>(mod), the_conf);
	if (name == "v775" || name == "v775n")
		return do_init_v775(static_cast<v775*>(mod), the_conf);
	if (name == "v785" || name == "v785n")
		return do_init_v785(static_cast<v785*>(mod), the_conf);
	if (name == "v792" || name == "v792n")
		return do_init_v792(static_cast<v792*>(mod), the_conf);
	if (name == "pixie16")
		return do_init_pixie16(static_cast<pixie16*>(mod), the_conf);
	return -E_UNKOWN_MOD;
}


int initzer::init_pixie16_ctl()
{
	/* first, find out how many pixie16 modules are included in the system
	 * and their slot maps.
	 * */
	int n_pixie_mod;
	unsigned short pxi_slot_map[16];
	unsigned short pxi_slot_map1[16];
	int i;
	memset(pxi_slot_map, 0, sizeof(unsigned short)*16);
	for (auto it = vme_conf.begin(); it != vme_conf.end(); it++) {
		std::string name = get_mod_name(*it);
		if (name.find("PIXIE16_MOD") != std::string::npos) {
			pxi_slot_map[get_mod_crate(*it)] = 1;
		}
	}
	n_pixie_mod = 0;
	for (i = 0; i < 16; i++) {
		if (pxi_slot_map[i]) {
			pxi_slot_map1[n_pixie_mod] = i;
			n_pixie_mod++;
		}
	}

	/* now we initialize the pixie16 controller */
	for (auto it = vme_conf.begin(); it != vme_conf.end(); it++) {
		std::string name = get_mod_name(*it);
		if (name.find("PIXIE16_CTL") != std::string::npos) {
			pixie16_ctl* x = do_init_pixie16_ctl(*it, n_pixie_mod,
					pxi_slot_map1);
			if (!x) 
				return -E_INIT_PIXIE16_CTL;
			p_pixie16_ctl.push_back(x);
			/* only one pixie16_ctl should exists */
			break;
		}
	}
	return 0;
}

pixie16_ctl* initzer::do_init_pixie16_ctl(std::vector<struct conf_vme_mod>
		&the_conf, int mod_n, unsigned short* pxi_slot_map)
{
	pixie16_ctl* tmp_pixie16_ctl = new pixie16_ctl;
	std::string dsp_par_f = get_conf_val_str(the_conf, "dsp set file");
	std::string dsp_par_f1 = decode_str(dsp_par_f);

	/* Now let's try to open it. */
	struct pixie16_ctl_open_par par;
	par.ComFPGAConfigFile = "./pixie16_firmware/firmware/syspixie16_revfgeneral_adc100mhz_r33338.bin";
	par.SPFPGAConfigFile  = "./pixie16_firmware/firmware/fippixie16_revfgeneral_14b100m_r36263.bin";
	par.TrigFPGAConfigFile = "";
	par.DSPCodeFile = "./pixie16_firmware/firmware/Pixie16DSP_revfgeneral_14b100m_r38896.ldr";
	//par.DSPParFile = "./pixie16_firmware/initial_par.set";
	par.DSPParFile = dsp_par_f1.c_str();
	printf("dsp set file: %s\n", par.DSPParFile);
	//return 0;
	par.DSPVarFile = "./pixie16_firmware/firmware/Pixie16DSP_revfgeneral_14b100m_r38896.var";
	par.pxi_slot_map = pxi_slot_map;
	par.mod_num = mod_n;
	if (tmp_pixie16_ctl->open(&par)) 
		goto fail;
		
	/* Now it's open, let initialize the registers  based on the config
	 * file */
	for (auto it = the_conf.begin(); it != the_conf.end(); it++) {
		if ((*it).name == "") {
			/* this is a register setting */
			uint64_t off = (*it).offset;
			uint64_t val = (*it).val.val_uint64;
			if (tmp_pixie16_ctl->write(off, &val))
				goto fail;
		}
	}

	return tmp_pixie16_ctl;

fail:
	delete tmp_pixie16_ctl;
	return NULL;
}


int initzer::init_vme_mod()
{
	int ret;

	if (!p_parser)
		return -E_PARSER;

	vme_conf = p_parser->get_conf_vme_mod(ret);
	RET_IF_NONZERO(ret);

	/* First thing we should do it to init(open) the vme controllers */
	/* try v2718 */
	ret = init_v2718();
	RET_IF_NONZERO(ret);
	/* try virtual vme controller supported vme controller here */
	ret = init_test_ctl();
	RET_IF_NONZERO(ret);
	/* try pixie16 controller */
	ret = init_pixie16_ctl();
	RET_IF_NONZERO(ret);
	/* try another supported vme controller here */


	/* Now we can initialize the vme modules */
	for (auto it = vme_conf.begin(); it != vme_conf.end(); it++) {
		std::string name = get_mod_name(*it);
		module* tmp;

		ret = create_mod(name, tmp);
		RET_IF_NONZERO(ret);
		if (!tmp)
			continue;
		if (init_global_var(tmp, *it)) {
			delete tmp;
			return -E_INIT_MODULE;
		}
		ret = do_init_mod(tmp, *it);
		RET_IF_NONZERO(ret);
		ret = fill_slot_map(tmp);
		RET_IF_NONZERO(ret);
		ret = fill_clk_map(tmp);
		RET_IF_NONZERO(ret);
		ret = fill_clk_off_map(tmp);
		RET_IF_NONZERO(ret);
		p_module.push_back(tmp);
	}

	return 0;
}


int initzer::init_test_ctl()
{
	/* We need to know the range of crate numbers. Because there exists an
	 * offset between the board numbers and crate numbers. */
	int min_crate = 100000;
	int crate;
	for (auto it = vme_conf.begin(); it != vme_conf.end(); it++) {
		std::string name = get_mod_name(*it);
		if (name.find("TEST_CTL") != std::string::npos) {
			crate = get_mod_crate(*it);
			min_crate = (crate < min_crate) ? crate : min_crate;
		}
	}

	/* now we initialize the test_ctl one by one */
	for (auto it = vme_conf.begin(); it != vme_conf.end(); it++) {
		std::string name = get_mod_name(*it);
		if (name.find("TEST_CTL") != std::string::npos) {
			test_ctl* tmp = do_init_test_ctl(*it, min_crate);
			if (!tmp) 
				return -E_INIT_TEST_CTL;
			p_test_ctl.push_back(tmp);
		}
	}
	return 0;
}

int initzer::init_v2718()
{
	/* We need to know the range of crate numbers. Because there exists an
	 * offset between the board numbers and crate numbers. */
	int min_crate = 100000;
	int crate;
	for (auto it = vme_conf.begin(); it != vme_conf.end(); it++) {
		std::string name = get_mod_name(*it);
		if (name.find("V2718") != std::string::npos) {
			crate = get_mod_crate(*it);
			min_crate = (crate < min_crate) ? crate : min_crate;
		}
	}

	/* now we initialize the v2718s one by one */
	for (auto it = vme_conf.begin(); it != vme_conf.end(); it++) {
		std::string name = get_mod_name(*it);
		if (name.find("V2718") != std::string::npos) {
			v2718* tmp = do_init_v2718(*it, min_crate);
			if (!tmp) 
				return -E_INIT_V2718;
			p_v2718.push_back(tmp);
		}
	}
	return 0;
}

test_ctl* initzer::do_init_test_ctl(std::vector<struct conf_vme_mod> &the_conf, 
		int crate_off)
{
	test_ctl* tmp_test_ctl = new test_ctl;
	int crate = -1;
	uint16_t reg_val;

	/* get the crate number */
	for (auto it = the_conf.begin(); it != the_conf.end(); it++) {
		if ((*it).name == "crate_n") {
			crate = (*it).val.val_uint64;
			tmp_test_ctl->set_crate(crate);
			break;
		}
	}
	if (crate == -1) 
		/* cannot get the crate number */
		goto fail;

	/* Now let's try to open it. */
	if (tmp_test_ctl->open(NULL)) 
		goto fail;
		
	return tmp_test_ctl;
fail:
	delete tmp_test_ctl;
	return NULL;
}


v2718* initzer::do_init_v2718(std::vector<struct conf_vme_mod> &the_conf, 
		int crate_off)
{
	v2718* tmp_v2718 = new v2718;
	int crate = -1;
	uint16_t reg_val;

	/* get the crate number */
	for (auto it = the_conf.begin(); it != the_conf.end(); it++) {
		if ((*it).name == "crate_n") {
			crate = (*it).val.val_uint64;
			tmp_v2718->set_crate(crate);
			break;
		}
	}
	if (crate == -1) 
		/* cannot get the crate number */
		goto fail;

	/* Now let's try to open it. */
	struct v2718_open_par par;
	par.bd_type = cvV2718;
	par.link = 0;
	par.bd_num = crate - crate_off;
	if (tmp_v2718->open(&par)) 
		goto fail;
		
	/* Now it's open, let initialize the registers */
	/* first, reset all registers */
	if (tmp_v2718->read_reg(0x1, &reg_val))
			goto fail;
	reg_val &= 0x80;
	if (tmp_v2718->write_reg(0x1, &reg_val))
		goto fail;
	/* init the register values based on the config file */
	for (auto it = the_conf.begin(); it != the_conf.end(); it++) {
		if ((*it).name == "") {
			/* this is a register setting */
			uint32_t off = (*it).offset;
			unsigned short val = (*it).val.val_uint64;
			if (off <= 0x2c) {
				/* this is a physical register */
				if (tmp_v2718->write_reg(off, &val))
					goto fail;
			}
			else if (off == 500) {
				/* control bit set */
				unsigned short ctrl_reg;
				if (tmp_v2718->read_reg(0x01, &ctrl_reg))
					goto fail;
				ctrl_reg |= val;
				if (tmp_v2718->write_reg(0x01, &ctrl_reg))
					goto fail;
			}
			else if (off == 501) {
				/* control bit clear */
				unsigned short ctrl_reg;
				if (tmp_v2718->read_reg(0x01, &ctrl_reg))
					goto fail;
				ctrl_reg &= (~val);
				if (tmp_v2718->write_reg(0x01, &ctrl_reg))
					goto fail;
			}
		}
	}

	return tmp_v2718;

fail:
	delete tmp_v2718;
	return NULL;
}


/* initialize the global variables (including the advanced settings...)of the
 * given module.
 * return 0 if succeed, otherwise return error code. */
int initzer::init_global_var(module* mod, 
		std::vector<struct conf_vme_mod> &the_conf)
{
	int crate = -1;

	/* get the crate number */
	crate = get_conf_val_u64(the_conf, "crate_n");
	mod->set_crate(crate);
	if (crate == -1) 
		/* cannot get the crate number */
		return -E_CRATE_NUM;

	/* get the slot number */
	mod->set_slot(get_conf_val_u64(the_conf, "slot_n"));

	/* get base address */
	mod->set_base(get_conf_val_u64(the_conf, "base"));

	/* get am */
	mod->set_am(get_conf_val_u64(the_conf, "am_reg"));

	/* get trigger/scaler type */
	mod->set_type(get_conf_val_ch(the_conf, "type"));

	/* get peroid (only makes sense for scaler-type modules */
	mod->set_period(get_conf_val_u64(the_conf, "period"));
	
	/* get if is trigger module */
	if (get_conf_val_u64(the_conf, "is_trig_mod") != 0xFFFFFFFFFFFFFFFF)
		mod->set_trig_mod(get_conf_val_u64(the_conf, "is_trig_mod"));

	/* get clock frequency (to be used in event builder)*/
	if (get_conf_val_u64(the_conf, "clk_fre") != 0xFFFFFFFFFFFFFFFF)
		mod->set_clk_freq(get_conf_val_u64(the_conf, "clk_fre"));
	
	/* get clock offset (to be used in event builder)*/
	if (get_conf_val_u64(the_conf, "clk_off") != 0xFFFFFFFFFFFFFFFF)
		mod->set_clk_off(get_conf_val_u64(the_conf, "clk_off"));

	/* assign the correct vme controller to the module */
	mod->set_ctl(NULL);
	/* first, try v2718 */
	for (auto it = p_v2718.begin(); it != p_v2718.end(); it++) {
		if (crate == (*it)->get_crate()) {
			mod->set_ctl(*it);
			break;
		}
	}
	/* then try other supported controllers (test_ctl)*/
	for (auto it = p_test_ctl.begin(); it != p_test_ctl.end(); it++) {
		if (crate == (*it)->get_crate()) {
			mod->set_ctl(*it);
			break;
		}
	}

	/* then try other supported controllers (pixie16_ctl)*/
	for (auto it = p_pixie16_ctl.begin(); it != p_pixie16_ctl.end(); it++) {
		mod->set_ctl(*it);
		break;
	}


	/* make sure the module has a vme controller assigned */
	if (!mod->get_ctl())
		return -E_VME_CTRL;

	return 0;
}


int initzer::fill_slot_map(module* mod)
{
	int crate = mod->get_crate();
	int slot = mod->get_slot();
#ifndef DAQ_XIA
	int geo = mod->get_geo();
#else
	int geo = slot;
#endif
	int mod_id = mod->get_mod_id();

	if (crate >= MAX_CRATE)
		return -E_MAX_CRATE;
	if (geo >= MAX_GEO)
		return -E_MAX_GEO;
	if (mod_id >= MAX_MODULE_ID)
		return -E_MAX_MODULE;

	slot_map[SLOT_MAP_IDX(crate, mod_id, geo)] = slot;
	slot_map_inited = true;
	return 0;
}

int initzer::fill_clk_off_map(module* mod)
{
	int crate = mod->get_crate();
	int slot = mod->get_slot();
	uint64_t clk_off = mod->get_clk_off();

	if (crate >= MAX_CRATE)
		return -E_MAX_CRATE;

	clk_off_map[CLK_OFF_MAP_IDX(crate,slot)] = clk_off;
	clk_off_map_inited = true;
	return 0;
}
int initzer::fill_clk_map(module* mod)
{
	int crate = mod->get_crate();
	int slot = mod->get_slot();
	uint64_t clk_freq = mod->get_clk_freq();

	if (crate >= MAX_CRATE)
		return -E_MAX_CRATE;

	clk_map[CLK_MAP_IDX(crate,slot)] = clk_freq;
	clk_map_inited = true;
	return 0;
}

/* get the number of trigger/scaler-type of modules from the vector of modules*/
static int get_num_mod(std::vector<module*>& mods, char type)
{
	int sum = 0;
	for (auto it = mods.begin(); it != mods.end(); it++) {
		if ((*it)->get_type() == type)
			sum++;
	}
	return sum;
}

/* get one modules object from the vector of modules */
static modules* get_one_modules(std::vector<module*>& mods, char type)
{
	int crate, ret;
	uint32_t t;
	std::string name;
	modules* the_modules = new modules;

	for (auto it = mods.begin(); it != mods.end(); it++) {
		if ((*it)->get_type() == type) {
			crate = (*it)->get_crate();
			name = (*it)->get_name();
			break;
		}
	}
	for (auto it = mods.begin(); it != mods.end(); it++) {
		if ((*it)->get_type() == 'S') {
			t = (*it)->get_period();
			break;
		}
	}

	
	for (auto it = mods.begin(); it != mods.end();) {
		if ((*it)->get_type() == type && (*it)->get_crate() == crate &&
				(*it)->get_name() == name) {
			/* For scaler-type, addtional requirements exist */
			if (type == 'S') {
				if ((*it)->get_period() == t) {
					ret = the_modules->add_mod(*it);
					if (ret) {
						printf("exit with error code: %d",ret);
						exit(ret);
					}
					mods.erase(it);
					continue;
				}
			}
			else {
				ret = the_modules->add_mod(*it);
				if (ret) {
					printf("exit with error code: %d",ret);
					exit(ret);
				}
				mods.erase(it);
				continue;
			}
		}
		/* if erased, the iterator automatically points to the next
		 * item, no it++ needed. */
		it++;
	}

	return the_modules;
}




int initzer::get_modules(char type, std::vector<modules*>& list_modules)
{
	int ret;

	if (!vme_mod_inited) {
		ret = init_vme_mod();
		vme_mod_inited = true;
		RET_IF_NONZERO(ret);
	}

	std::vector<module*> tmp = p_module;
	while (get_num_mod(tmp, type)) 
		list_modules.push_back(get_one_modules(tmp, type));

	return 0;
}

#endif

char* initzer::get_mod_name(std::vector<struct conf_vme_mod> &the_conf)
{
	for (auto it = the_conf.begin(); it != the_conf.end(); it++) {
		if ((*it).name == "name")
			return (*it).val.val_str;
	}
	return NULL;
}
int64_t initzer::get_mod_crate(std::vector<struct conf_vme_mod> &the_conf)
{
	for (auto it = the_conf.begin(); it != the_conf.end(); it++) {
		if ((*it).name == "crate_n")
			return (*it).val.val_uint64;
	}
	return -1;
}


#ifdef MAKE_EVENT_BUILDER
int initzer::get_ebd_pixie_clk_src()
{
	bool found;
	std::string name("pixie_clk");
	int port;

	port = get_ebd_adv_var(name, found);
	if (found)
		return (port == 1) ? PIXIE_CLK_INT : PIXIE_CLK_EXT;
	else
		return PIXIE_CLK_INT;
}

uint32_t initzer::get_ebd_sort_clock_hz()
{
	bool found;
	std::string name("sort_freq");
	int port;

	port = get_ebd_adv_var(name, found);
	if (found)
		return port;
	else
		return DEF_EBD_SORT_HZ;
}

uint32_t initzer::get_ebd_merge_glom()
{
	bool found;
	std::string name("merge_glom");
	int port;

	port = get_ebd_adv_var(name, found);
	if (found)
		return port;
	else
		return DEF_EBD_MERGE_GLOM;

}

uint32_t initzer::get_ebd_merge_type()
{
	bool found;
	std::string name("merge_type");
	int port;

	port = get_ebd_adv_var(name, found);
	if (found)
		return port;
	else
		return DEF_EBD_MERGE_TYPE;

}

uint32_t initzer::get_ebd_merge_merged_buf_sz()
{
	bool found;
	std::string name("merge_buf_sz");
	int port;

	port = get_ebd_adv_var(name, found);
	if (found)
		return port;
	else
		return DEF_EBD_MERGED_BUF_SZ;

}
std::string initzer::get_ebd_ctl_svr_addr()
{
	bool found;
	std::string name("ctl_svr_addr");
	std::string addr;

	get_ebd_adv_var(name, found, &addr);
	if (!found)
		addr = DEF_SVR_CTL_FE;
	else
		addr = decode_str(addr);

	return addr;
}
int initzer::get_ebd_buf_sz(int id)
{
	bool found;
	std::string name("ring_buf_sz");
	int port;

	switch (id) {
	case 1:
		name = "ring_buf_sz";
		break;
	case 2:
		name = "ring_buf2_sz";
		break;
	case 3:
		name = "ring_buf3_sz";
		break;
	case 4:
		name = "ring_bufs_sz";
		break;
	case 5:
		name = "ring_buf5_sz";
		break;
	}

	port = get_ebd_adv_var(name, found);
	if (found)
		return port;
	else
		return 0;

}
int initzer::get_ebd_ctl_buf_sz()
{
	bool found;
	std::string name("ctl_buf_sz");
	int port;

	port = get_ebd_adv_var(name, found);
	if (found)
		return port;
	else
		return DEF_SOCK_BUF_FE_CTL; 
}
int initzer::get_ebd_recv_t_us()
{
	bool found;
	std::string name("recv_t_us");
	int port;

	port = get_ebd_adv_var(name, found);
	if (found)
		return port;
	else
		return DEF_T_US_RECV_EBD;
}
int initzer::get_ebd_ctl_t_us()
{
	bool found;
	std::string name("ctl_t_us");
	int port;

	port = get_ebd_adv_var(name, found);
	if (found)
		return port;
	else
		return DEF_T_US_CTL_FE;

}
int initzer::get_ebd_max_evt_len()
{
	bool found;
	std::string name("max_evt_len");
	int port;

	port = get_ebd_adv_var(name, found);
	if (found)
		return port;
	else
		return DEF_EBD_MAX_EVT_LEN;
	
}

std::vector<ring_buf*> initzer::get_ebd_rb()
{
	uint32_t sz2 = get_ebd_buf_sz(1);
	int num_rb, i;
	ring_buf* p_rb;
	
	if (sz2 == 0)
		sz2 = DEF_RB_EBD_RAW;
	if (rb_ebd.size())
		/* already initiated */
		return rb_ebd;
	
	num_rb = get_ebd_n_recv();
	for (i = 0; i < num_rb; i++) {
		p_rb = new ring_buf;
		if (p_rb->init(sz2)) {
			delete p_rb;
			goto err;
		}
		rb_ebd.push_back(p_rb);
	}
	return rb_ebd;

err:
	for (auto it = rb_ebd.begin(); it != rb_ebd.end(); it++) 
		delete (*it);
	rb_ebd.clear();
	return rb_ebd;
}

ring_buf* initzer::get_ebd_rb(int rb_id)
{
	uint32_t sz2;
	ring_buf* p_rb;

	sz2 = get_ebd_buf_sz(rb_id);
	switch (rb_id) {
	case 2:
		/* first check if the ring buffer object already created */
		RET_IF_NONZERO(rb_ebd2);
		if (sz2 == 0)
			sz2 = DEF_RB_EBD_MSG;
		break;
	case 3:
		RET_IF_NONZERO(rb_ebd3);
		if (sz2 == 0)
			sz2 = DEF_RB_EBD_BLD;
		break;
	case 5:
		RET_IF_NONZERO(rb_ebd5);
		if (sz2 == 0)
			sz2 = DEF_RB_EBD_SCL;
		break;
	default:
		return NULL;
	}
	if (!p_parser)
		return NULL;

	p_rb = new ring_buf;
	if (p_rb->init(sz2)) {
		delete p_rb;
		return NULL;
	}
	
	switch (rb_id) {
	case 2:
		rb_ebd2 = p_rb;
		break;
	case 3:
		rb_ebd3 = p_rb;
		break;
	case 5:
		rb_ebd5 = p_rb;
	}
	return p_rb;
}
#endif

#ifdef MAKE_ANALYZER
std::string initzer::get_ana_ctl_svr_addr()
{
	bool found;
	std::string name("ctl_svr_addr");
	std::string addr;

	get_ana_adv_var(name, found, &addr);
	if (!found)
		addr = DEF_SVR_CTL_FE;
	else
		addr = decode_str(addr);

	return addr;
}
int initzer::get_ana_buf_sz(int id)
{
	bool found;
	std::string name("trig_buf_sz");
	int port;

	switch (id) {
	case 0:
		name = "trig_buf_sz";
		break;
	case 1:
		name = "msg_buf_sz";
		break;
	case 2:
		name = "scal_buf_sz";
		break;
	}

	port = get_ana_adv_var(name, found);
	if (found)
		return port;
	else
		return 0;
}
int initzer::get_ana_ctl_buf_sz()
{
	bool found;
	std::string name("ctl_buf_sz");
	int port;

	port = get_ana_adv_var(name, found);
	if (found)
		return port;
	else
		return DEF_SOCK_BUF_FE_CTL; 
}
int initzer::get_ana_roody_svr_port()
{
	bool found;
	std::string name("roody_svr_port");
	int port;

	port = get_ana_adv_var(name, found);
	if (found)
		return port;
	else
		return DEF_ANA_ROODY_SVR_PORT;
}
int initzer::get_ana_main_buf_sz()
{
	bool found;
	std::string name("main_buf_sz");
	int port;

	port = get_ana_adv_var(name, found);
	if (found)
		return port;
	else
		return DEF_MAX_EVT_LEN;
}
int initzer::get_ana_recv_t_us()
{
	bool found;
	std::string name("recv_t_us");
	int port;

	port = get_ana_adv_var(name, found);
	if (found)
		return port;
	else
		return DEF_T_US_RECV_EBD;
}
int initzer::get_ana_ctl_t_us()
{
	bool found;
	std::string name("ctl_t_us");
	int port;

	port = get_ana_adv_var(name, found);
	if (found)
		return port;
	else
		return DEF_T_US_CTL_FE;

}
ring_buf* initzer::get_ana_rb(int rb_id)
{
	uint32_t sz2;
	ring_buf* p_rb;

	sz2 = get_ana_buf_sz(rb_id);
	switch (rb_id) {
	case 0:
		/* first check if the ring buffer object already created */
		RET_IF_NONZERO(rb_ana0);
		if (sz2 == 0)
			sz2 = DEF_RB_ANA;
		break;
	case 1:
		/* first check if the ring buffer object already created */
		RET_IF_NONZERO(rb_ana1);
		if (sz2 == 0)
			sz2 = DEF_RB_LOG_MSG;
		break;
	case 2:
		RET_IF_NONZERO(rb_ana2);
		if (sz2 == 0)
			sz2 = DEF_RB_LOG_SCAL;
		break;
	default:
		return NULL;
	}
	if (!p_parser)
		return NULL;

	p_rb = new ring_buf;
	if (p_rb->init(sz2)) {
		delete p_rb;
		return NULL;
	}
	
	switch (rb_id) {
	case 0:
		rb_ana0 = p_rb;
		break;
	case 1:
		rb_ana1 = p_rb;
		break;
	case 2:
		rb_ana2 = p_rb;
	}
	return p_rb;
}
#endif

#ifdef MAKE_LOGGER
uint32_t initzer::get_log_save_buf_len()
{
	bool found;
	std::string name("save_buf_sz");
	int port;

	port = get_log_adv_var(name, found);
	if (found)
		return port;
	else
		return DEF_LOG_SAVE_BUF;

}
std::string initzer::get_log_ctl_svr_addr()
{
	bool found;
	std::string name("ctl_svr_addr");
	std::string addr;

	get_log_adv_var(name, found, &addr);
	if (!found)
		addr = DEF_SVR_CTL_FE;
	else
		addr = decode_str(addr);

	return addr;
}
std::string initzer::get_log_save_path()
{
	bool found;
	std::string name("save_path");
	std::string addr;

	get_log_adv_var(name, found, &addr);
	if (!found)
		addr = DEF_SAVE_PATH_LOG;
	else
		addr = decode_str(addr);

	return addr;

}
int initzer::get_log_buf_sz(int id)
{
	bool found;
	std::string name("trig_buf_sz");
	int port;

	switch (id) {
	case 0:
		name = "trig_buf_sz";
		break;
	case 1:
		name = "msg_buf_sz";
		break;
	case 2:
		name = "scal_buf_sz";
		break;
	}

	port = get_log_adv_var(name, found);
	if (found)
		return port;
	else
		return 0;
}
int initzer::get_log_ctl_buf_sz()
{
	bool found;
	std::string name("ctl_buf_sz");
	int port;

	port = get_log_adv_var(name, found);
	if (found)
		return port;
	else
		return DEF_SOCK_BUF_FE_CTL; 
}
int initzer::get_log_recv_t_us()
{
	bool found;
	std::string name("recv_t_us");
	int port;

	port = get_log_adv_var(name, found);
	if (found)
		return port;
	else
		return DEF_T_US_RECV_EBD;
}
int initzer::get_log_ctl_t_us()
{
	bool found;
	std::string name("ctl_t_us");
	int port;

	port = get_log_adv_var(name, found);
	if (found)
		return port;
	else
		return DEF_T_US_CTL_FE;

}
ring_buf* initzer::get_log_rb(int rb_id)
{
	uint32_t sz2;
	ring_buf* p_rb;

	sz2 = get_log_buf_sz(rb_id);
	switch (rb_id) {
	case 0:
		/* first check if the ring buffer object already created */
		RET_IF_NONZERO(rb_log0);
		if (sz2 == 0)
			sz2 = DEF_RB_LOG_TRIG;
		break;
	case 1:
		/* first check if the ring buffer object already created */
		RET_IF_NONZERO(rb_log1);
		if (sz2 == 0)
			sz2 = DEF_RB_LOG_MSG;
		break;
	case 2:
		RET_IF_NONZERO(rb_log2);
		if (sz2 == 0)
			sz2 = DEF_RB_LOG_SCAL;
		break;
	default:
		return NULL;
	}
	if (!p_parser)
		return NULL;

	p_rb = new ring_buf;
	if (p_rb->init(sz2)) {
		delete p_rb;
		return NULL;
	}
	
	switch (rb_id) {
	case 0:
		rb_log0 = p_rb;
		break;
	case 1:
		rb_log1 = p_rb;
		break;
	case 2:
		rb_log2 = p_rb;
	}
	return p_rb;
}
#endif



int initzer::get_adv_var(int id, std::string& var_name, bool& found, 
		std::string* value2)
{
	int ret;

	if (!p_parser) {
		found = false;
		return 0;
	}

	auto adv_conf = p_parser->get_conf_adv(id, ret);
	for (auto it = adv_conf.begin(); it != adv_conf.end(); it++) {
		if ((*it).name == var_name) {
			ret = (*it).value;
			if (value2)
				*value2 = (*it).value2;
			goto found;
		}
	}

	/* not found: */
	found = false;
	return 0;

found:
	found = true;
	return ret;
}

int initzer::get_ebd_sender_port()
{
	bool found;
	std::string name("sender_port");
	int port;

	port = get_ebd_adv_var(name, found);
	if (found)
		return port;
	else
		return DEF_PORT_SEND_EBD;
}
int initzer::get_ctl_port()
{
	bool found;
	std::string name("port");
	int port;

	port = get_ctl_adv_var(name, found);
	if (found)
		return port;
	else
		return DEF_PORT_CTL;
}







/* get the n'th address from the addresses (separated by commas), if the number
 * of addresses <= n, return empty string, indicating errors . */
static std::string get_nth_addr(int n, std::string addrs)
{
	std::string addr("");
	int num_addr = std::count(addrs.begin(), addrs.end(), ',') + 1;
	int i;
	
	if (num_addr <= n)
		return addr;
	addrs.insert(0, 1, 'x');
	addrs.append(1, ',');
	for (i = 0; i < n; i++) 
		addrs.replace(addrs.find_first_of(','), 1, 1, 'x');
	int pos1 = addrs.find_last_of('x') + 1;
	int pos2 = addrs.find_first_of(',');
	return addrs.substr(pos1, pos2 - pos1);
}


std::string initzer::get_ebd_recv_svr_addr(int n)
{
	bool found;
	std::string name("fe_server_addr");
	std::string addr;

	get_ebd_adv_var(name, found, &addr);
	if (!found)
		addr = DEF_SVR_RECV_EBD;
	else
		addr = decode_str(addr);

	return get_nth_addr(n, addr);
}

std::string initzer::get_ana_recv_svr_addr()
{
	bool found;
	std::string name("ebd_server_addr");
	std::string addr;

	get_ana_adv_var(name, found, &addr);
	if (!found)
		addr = DEF_SVR_RECV_EBD;
	else
		addr = decode_str(addr);

	return addr;
}

std::string initzer::get_log_recv_svr_addr()
{
	bool found;
	std::string name("ebd_server_addr");
	std::string addr;

	get_log_adv_var(name, found, &addr);
	if (!found)
		addr = DEF_SVR_RECV_EBD;
	else
		addr = decode_str(addr);

	return addr;
}

int initzer::get_fe_sender_buf_sz()
{
	bool found;
	std::string name("sender_buf_sz");
	int port;

	port = get_fe_adv_var(name, found);
	if (found)
		return port;
	else
		return DEF_SOCK_BUF_FE_SEND;

}

int initzer::get_ebd_n_recv()
{
	bool found;
	std::string name("fe_server_addr");
	std::string addr;

	get_ebd_adv_var(name, found, &addr);
	if (!found)
		addr = DEF_SVR_RECV_EBD;
	else
		addr = decode_str(addr);

	return std::count(addr.begin(), addr.end(), ',') + 1;
}

int initzer::get_ebd_sender_buf_sz()
{
	bool found;
	std::string name("sender_buf_sz");
	int port;

	port = get_ebd_adv_var(name, found);
	if (found)
		return port;
	else
		return DEF_SOCK_BUF_EBD_SEND;

}
int initzer::get_fe_blt_buf_sz()
{
	bool found;
	std::string name("blt_buf_sz");
	int sz;

	sz = get_fe_adv_var(name, found);
	if (found)
		return sz;
	else
		return DEF_BLT_BUF_FE;
}
int initzer::get_fe_sender_port()
{
	bool found;
	std::string name("sender_port");
	int port;

	port = get_fe_adv_var(name, found);
	if (found)
		return port;
	else
		return DEF_PORT_SEND_FE;
}
