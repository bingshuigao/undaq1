#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define MAX_LEN 1000
uint32_t evt_buf[MAX_LEN];

void decompose(FILE* fp)
{
	uint64_t ts_hi, ts_lo;

	if (evt_buf[0] == 32) 
		/* empty event */
		return;

	ts_hi = evt_buf[4];
	ts_lo = evt_buf[5];
	fprintf(fp, "%-20d%-20d%-20d%-20d%-20d\n",
			(ts_hi<<32) + ts_lo,
			evt_buf[9+1],
			evt_buf[9+2],
			evt_buf[9+4],
			evt_buf[9+5]);
}

int main(int argc, char* argv[])
{
	FILE* fp_in, *fp_out;
	char f_name[100];
	char run_title[128];

	if (argc != 2) {
		fprintf(stderr, "usage: ./xxx <input file>\n");
		return 0;
	}
	fp_in = fopen(argv[1], "rb");
	if (!fp_in) {
		fprintf(stderr, "cannot open input file!\n");
		return 0;
	}
	sprintf(f_name, "%s.txt", argv[1]);
	fp_out = fopen(f_name, "w");
	if (!fp_out) {
		fprintf(stderr, "cannot open output file!\n");
		return 0;
	}


	fprintf(fp_out, "%-20s%-20s%-20s%-20s%-20s\n",
			"unix_time", "collimator", "HPGe", "target", "NaI");
	/* do the loop */
	fread(run_title, 1, 128, fp_in);
	while (!feof(fp_in)) {
		if (1 != fread(evt_buf, 4, 1, fp_in)) 
			break;
		//printf("%d\n", evt_buf[0]);
		if ((evt_buf[0] - 4) != fread(evt_buf+1, 1, evt_buf[0]-4, fp_in)) 
			break;
		decompose(fp_out);
	}

	fclose(fp_in);
	fclose(fp_out);
	return 0;
}
