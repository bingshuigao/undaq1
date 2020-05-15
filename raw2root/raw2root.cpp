#include <stdio.h>
#include <TTree.h>
#include <TBranch.h>
#include <TFile.h>
#include <TTree.h>
#include <string.h>
#include "ana_evt_hd.h"
#include "ana_frag_hd.h"
#include "ana_madc32.h"

/* max data length */
#define MAX_LEN 1000


ana_evt_hd evt_hd;
ana_frag_hd frag_hd;
ana_madc32 evt_madc;


struct evt_trig_hd
{
	uint32_t tot_len;
	uint32_t ts_hi;
	uint32_t ts_lo;
};
struct evt_trig_frag
{
	uint32_t slot;
	uint32_t len;
	uint32_t ts_hi;
	uint32_t ts_lo;
	uint32_t adc[32];
};
struct evt_trig_hd evt_trig_hd;
struct evt_trig_frag evt_trig_yy1;
uint32_t evt_buf[MAX_LEN];

void decompose()
{
	uint32_t* p_dw = evt_buf;
	uint32_t* p_dw1= evt_buf;
	int slot, len_tot, len_frag;
	uint32_t madc_val[32];

	memset(&evt_trig_hd, 0, sizeof(evt_trig_hd));
	memset(&evt_trig_yy1,0, sizeof(evt_trig_yy1));

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
		if (slot == 2) {
			/* madc */
			int i;
			evt_madc.parse_raw(p_dw, madc_val);
			for (i = 0; i < 32; i++) 
				evt_trig_yy1.adc[i] = madc_val[i];
		}
		else if (slot == 3) {
			/* v1190, for example */
		}

		p_dw -= 5;
		p_dw += len_frag+1;
	} 
}

int main(int argc, char* argv[])
{
	FILE* fp;
	uint64_t f_size, proc_size, n_read;
	TFile* f_out;
	TTree* tree;
	char f_name[100];

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
	f_out = new TFile(f_name, "CREATE");
	if (f_out->IsZombie()) {
		fprintf(stderr, "cannot open root file for output!\n");
		delete f_out;
		return 0;
	}

	/* set branch addresses */
	tree = new TTree("trigger", "trigger");
	tree->Branch("trig_head", &evt_trig_hd,  "len/i:ts_hi:ts_lo");
	tree->Branch("trig_yy1",  &evt_trig_yy1, "slot/i:len:ts_hi:ts_lo:adc[32]");
	
	/* do the loop */
	proc_size = 0;
	n_read = 0;
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
