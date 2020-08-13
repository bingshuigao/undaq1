/* convert raw data file into root format. currently it does not surpport
 * multiple DAQs (i.e. the daq number has to be zero).  
 * By B.Gao Jun. 23, 2020
 *
 * */
#include <stdio.h>
#include <TTree.h>
#include <TBranch.h>
#include <TFile.h>
#include <TTree.h>
#include <string.h>
#include "ana_evt_hd.h"
#include "ana_frag_hd.h"
#include "ana_madc32.h"
#include "ana_v1190.h"
#include "ana_v1740.h"
#include "err_code.h"
#include "imp_daq.h"
#include "ana_module.h"
#include "xml_parser.h"
#include <string>

/* max data length */
#define MAX_LEN DEF_EBD_MAX_EVT_LEN


ana_evt_hd evt_hd;
ana_frag_hd frag_hd;


struct br_evt_hd
{
	uint64_t ts;
	uint64_t unix_t;
};

struct br_frag_hd
{
	uint64_t ts;
	uint32_t slot;
	uint32_t crate;
	uint32_t daq;
	uint32_t is_valid;
};

struct br_frag
{
	struct br_frag_hd br_frag_hd;
	ana_module* br_frag_body;
};


struct br_evt_hd br_evt_hd;
std::vector<struct br_frag*> lst_of_br;
uint32_t evt_buf[MAX_LEN];


void parse_and_fill_v1190(ana_v1190* tmp_v1190)
{
	int i;
	for (i = 0; i < 128; i++) {
		int j, n_hit;
		n_hit = tmp_v1190->get_val()[i].size();
		tmp_v1190->get_data_ptr(i)->n_hit = n_hit;
		if (n_hit >= MAX_V1190_HIT) 
			std::cout<<"too many v1190 hits!"<<std::endl;
		for (j = 0; j < n_hit; j++) {
			tmp_v1190->get_data_ptr(i)->tdc_val[j] = tmp_v1190->get_val()[i][j];
			if (j >= MAX_V1190_HIT)
				break;
		}
	}

}

void parse_and_fill_v1740(ana_v1740* tmp_v1740)
{
	int i;
	for (i = 0; i < 64; i++) {
		int j, n;
		n = tmp_v1740->get_n_samp();
		for (j = 0; j < n; j++) {
			tmp_v1740->get_data_ptr(i)[j] = tmp_v1740->get_samp()[i][j];
		}
	}
}

void parse_and_fill(uint32_t* p_raw, int daq, int crate, int slot)
{
	ana_v1190* tmp_v1190;
	ana_v1740* tmp_v1740;
	for (auto it = lst_of_br.begin(); it != lst_of_br.end(); it++) {
		if ((*it)->br_frag_hd.daq != daq)
			continue;
		if ((*it)->br_frag_hd.crate != crate)
			continue;
		if ((*it)->br_frag_hd.slot != slot)
			continue;
		(*it)->br_frag_body->parse_raw(p_raw);
		(*it)->br_frag_hd.is_valid = 1;

		switch ((*it)->br_frag_body->get_mod_id()) {
		case 1:
			/* madc32 */
			break;
		case 2:
			/* v1190 */
			tmp_v1190 = static_cast<ana_v1190*>((*it)->br_frag_body);
			parse_and_fill_v1190(tmp_v1190);
			break;
		case 5:
			/* v1740 */
			tmp_v1740 = static_cast<ana_v1740*>((*it)->br_frag_body);
			parse_and_fill_v1740(tmp_v1740);
			break;
		}
	}
}


void clear_buf()
{
	int i, n_samp;
	ana_v1740* tmp_v1740;
	ana_v1190* tmp_v1190;
	ana_madc32* tmp_madc;
	
	for (auto it = lst_of_br.begin(); it != lst_of_br.end(); it++) {
		(*it)->br_frag_hd.is_valid = 0;
		switch ((*it)->br_frag_body->get_mod_id()) {
		case 1:
			/* madc32 */
			tmp_madc = static_cast<ana_madc32*>((*it)->br_frag_body);
			memset(tmp_madc->get_adc_val(), 0, 32*4);
			break;
		case 2:
			/* v1190 */
			tmp_v1190 = static_cast<ana_v1190*>((*it)->br_frag_body);
			for (i = 0; i < 128; i++) {
				memset(tmp_v1190->get_data_ptr(i), 0,
					sizeof(struct v1190_channel_data));
			}
			break;
		case 5:
			/* v1740 */
			tmp_v1740 = static_cast<ana_v1740*>((*it)->br_frag_body);
			n_samp = tmp_v1740->get_n_samp();
			for (i = 0; i < 64; i++) {
				memset(tmp_v1740->get_data_ptr(i), 0,
						n_samp*sizeof(uint16_t));
			}
			break;
		}
	}
}

void decompose()
{
	uint32_t* p_dw = evt_buf;
	uint32_t* p_dw1= evt_buf;
	int slot, crate, daq, len_tot, len_frag;

	clear_buf();


	/* parse event header */
	evt_hd.parse_hd(p_dw);
	len_tot = evt_hd.get_len();

	/* loop the fragments */
	p_dw += 5;
	while (p_dw - p_dw1 != len_tot) {
		/* parse fragment header */
		frag_hd.parse_hd(p_dw);
		p_dw += 5;
		
		slot = frag_hd.get_slot();
		crate =frag_hd.get_crate();
		daq   =frag_hd.get_daq();
		len_frag = frag_hd.get_len();
		parse_and_fill(p_dw, daq, crate, slot);
	
		p_dw -= 5;
		p_dw += len_frag+1;
	} 
}

/* get list of branches. each module in the vme crate corresponds to a branch.
 * we get those information from the config.xml(hard-coded). return 0 if
 * success, otherwise return error code. */
static int get_lst_branches()
{
	int ret;

	xml_parser* parser = new xml_parser();
	ret = parser->parse("config.xml");
	RET_IF_NONZERO(ret);

	auto vme_modules = parser->get_conf_vme_mod(ret);
	RET_IF_NONZERO(ret);
	for (auto it = vme_modules.begin(); it != vme_modules.end(); it++) {
		struct br_frag* tmp = new br_frag;
		tmp->br_frag_hd.slot = get_conf_val_u64(*it, "slot_n");
		tmp->br_frag_hd.crate = get_conf_val_u64(*it, "crate_n");
		tmp->br_frag_hd.daq = 0;
		std::string name = get_conf_val_str(*it, "name");
//		std::cout<<name<<std::endl;

		if (name.find("MADC32") != std::string::npos) {
			tmp->br_frag_body = new ana_madc32();
			lst_of_br.push_back(tmp);
		}
		else if (name.find("V1190") != std::string::npos) {
			tmp->br_frag_body = new ana_v1190();
			lst_of_br.push_back(tmp);
		}
		else if (name.find("V1740") != std::string::npos) {
			bool is_found;
			int n_buf = get_conf_val_reg((*it), 0x800c, is_found);
			if (!is_found) {
				std::cout<<"do not know v1740 number of points, pls define it in the code"<<std::endl;
				return -E_V1740_N_PT;
			}
			tmp->br_frag_body = new ana_v1740(192<<(10-n_buf));
			lst_of_br.push_back(tmp);
		}
		else if (name.find("V2718") != std::string::npos) {
			delete tmp;
		}
		else if (name.find("TEST_CTL") != std::string::npos) {
			delete tmp;
		}
		else {
			delete tmp;
			return -E_NOT_IMPLE;
		}
	}

	return 0;
}


TTree* set_br_addr(char* run_title)
{
	int i, n_samp;
	char buf1[100], buf2[100];
	ana_v1740* tmp_v1740;
	ana_v1190* tmp_v1190;
	ana_madc32*  tmp_madc;
	/* set branch addresses */
	TTree* tree = new TTree("trigger", run_title);
	tree->Branch("evt_head", &br_evt_hd,  "ts/l:unix_t");
	for (auto it = lst_of_br.begin(); it != lst_of_br.end(); it++) {
		tree->Branch("frag_hd", &((*it)->br_frag_hd), "ts/l:slot/i:crate:daq:is_valid");
		switch ((*it)->br_frag_body->get_mod_id()) {
		case 1:
			/* madc32 */
			tmp_madc = static_cast<ana_madc32*>((*it)->br_frag_body);
			tree->Branch("frag_madc", tmp_madc->get_adc_val(), "adc[32]/i");
			break;
		case 2:
			/* v1190 */
			tmp_v1190 = static_cast<ana_v1190*>((*it)->br_frag_body);
			for (i = 0; i < 128; i++) {
				sprintf(buf1, "frag_v1190_ch%03d", i);
				sprintf(buf2, "n_dw%03d/i:tdc[n_dw%03d]", i, i);
				tmp_v1190->set_data_ptr(i, new struct v1190_channel_data);
				tree->Branch(buf1, tmp_v1190->get_data_ptr(i), "n_hit/i:hit[n_hit]");
			}
			break;
		case 5:
			/* v1740 */
			tmp_v1740 = static_cast<ana_v1740*>((*it)->br_frag_body);
			n_samp = tmp_v1740->get_n_samp();
			for (i = 0; i < 64; i++) {
				sprintf(buf1, "frag_v1740_ch%02d", i);
				sprintf(buf2, "pt[%d]", n_samp);
				tmp_v1740->set_data_ptr(i, new uint16_t[n_samp]);
				tree->Branch(buf1, tmp_v1740->get_data_ptr(i), buf2);
			}
			break;
		default:
			std::cout<<"not supported module!"<<std::endl;
			break;
		}
	}

	return tree;
}

int main(int argc, char* argv[])
{
	FILE* fp;
	uint64_t f_size, proc_size, n_read;
	int ret;
	TFile* f_out;
	TTree* tree;
	char f_name[100];
	char run_title[128];

	if (argc != 2) {
		fprintf(stderr, "usage: ./xxx <raw file>\n");
		return 0;
	}

	/* open raw file */
	fp = fopen(argv[1], "rb");
	if (!fp) {
		fprintf(stderr, "cannot open input file!\n");
		return 0;
	}
	fseek(fp, 0L, SEEK_END);
	f_size = ftell(fp);
	rewind(fp);

	/* open root file for output */
	sprintf(f_name, "%s.root", argv[1]);
	f_out = new TFile(f_name, "RECREATE");
	if (f_out->IsZombie()) {
		fprintf(stderr, "cannot open root file for output!\n");
		delete f_out;
		return 0;
	}

	ret = get_lst_branches();
	if (ret) {
		std::cout<<"cannot parse config file, error code: "<<ret<<std::endl;
		return ret;
	}

	fread(run_title, 1, 128, fp);
	tree = set_br_addr(run_title);
	
	/* do the loop */
	proc_size = 0;
	n_read = 0;
	proc_size += 128;
	while (!feof(fp)) {
		if (1 != fread(evt_buf, 4, 1, fp))
			break;
		if (evt_buf[0] >= MAX_LEN) {
			fprintf(stderr, "event too large, please increase MAX_LEN\n");
			break;
		}
		if ((evt_buf[0] - 1) != fread(evt_buf+1, 4, evt_buf[0]-1, fp))
			break;
		proc_size += evt_buf[0]*4;
		decompose();
		tree->Fill();
		if (((n_read++) % 100000) == 0)
			printf("%%%f finished\n", 100.*proc_size/f_size);
	}
	printf("processed/total: %ld/%ld\n", proc_size, f_size);

	tree->Write();
	f_out->Close();
	fclose(fp);
}