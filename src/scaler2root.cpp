/* NOTE: this program works ONLY for one v830 module as a scaler 
 * The v830 has to be configured to have the header in its data block
 *
 * By B.Gao Nov. 16, 2020
 *
 * */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <TFile.h>
#include <TTree.h>

#define MAX_LEN 1000000
uint32_t evt_buf[MAX_LEN];
uint32_t cnts[34];

void decompose(FILE* fp, TTree* t)
{
	uint64_t ts_hi, ts_lo;
	int i, n_ch, len;

	if (evt_buf[0] == 32) 
		/* empty event */
		return;

	cnts[0] = evt_buf[4];
	cnts[1] = evt_buf[5];
	n_ch = (evt_buf[8] >> 18) & 0x3f;
	len = evt_buf[6];
	len -= 2*4;

	while (len != 0) {
		for (i = 0; i < n_ch; i++) {
			cnts[i+2] = evt_buf[9+i];
		}
		t->Fill();
		len -= (1+n_ch)*4;
	}
}

int main(int argc, char* argv[])
{
	FILE* fp_in;
	char f_name[100];
	char leaf_list[1000];
	char run_title[128];
	TFile* f_out;
	TTree* t;
	int i;
	uint64_t f_size, proc_size;

	if (argc != 2) {
		fprintf(stderr, "usage: ./xxx <input file>\n");
		return 0;
	}

	fp_in = fopen(argv[1], "rb");
	if (!fp_in) {
		fprintf(stderr, "cannot open input file!\n");
		return 0;
	}
	fseek(fp_in, 0L, SEEK_END);
	f_size = ftell(fp_in);
	rewind(fp_in);
	proc_size = 0;

	sprintf(f_name, "%s.root", argv[1]);
	f_out = new TFile(f_name, "create");
	if (f_out->IsZombie()) {
		fprintf(stderr, "cannot open output root file\n");
		return 0;
	}

	t = new TTree("scaler", "scaler");
	sprintf(leaf_list, "ts_lo/i:ts_hi");
	for (i = 0; i < 32; i++)
		sprintf(leaf_list, "%s:cnt%02d", leaf_list, i);
	t->Branch("v830_counts", cnts, leaf_list);
	
	/* do the loop */
	fread(run_title, 1, 128, fp_in);
	proc_size += 128;
	while (!feof(fp_in)) {
		if (1 != fread(evt_buf, 4, 1, fp_in)) 
			break;
		if (evt_buf[0] >= MAX_LEN*4) {
			fprintf(stderr, 
				"event too large, please increase MAX_LEN\n");
			break;
		}
		//printf("%d\n", evt_buf[0]);
		if ((evt_buf[0] - 4) != fread(evt_buf+1, 1, evt_buf[0]-4, fp_in)) 
			break;
		decompose(fp_in, t);
		proc_size += evt_buf[0];
	}

	fclose(fp_in);
	t->Write();
	f_out->Close();

	printf("processed/total = %ld/%ld (%.2f%%) \n", proc_size, f_size, 
			100.*proc_size/f_size);
	printf("root file %s has been created!\n", f_name);
	return 0;
}
