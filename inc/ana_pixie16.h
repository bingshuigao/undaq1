/* this class analyze the raw data from pixie16 modules. 
 *
 * By B.Gao Jan. 22, 2022 
 * */

#ifndef ANA_PIXIE16_HHH
#define ANA_PIXIE16_HHH

#include <stdint.h>
#include "ana_module.h"

struct pixie16_data
{
	uint64_t ts;
	uint64_t ts_ext;
	uint32_t cfd;
	uint32_t energy;
	uint32_t energy_tr;
	uint32_t energy_le;
	uint32_t energy_ga;
	uint32_t qdc0;
	uint32_t qdc1;
	uint32_t qdc2;
	uint32_t qdc3;
	uint32_t qdc4;
	uint32_t qdc5;
	uint32_t qdc6;
	uint32_t qdc7;
	uint16_t wave[MAX_PIXIE16_WAVE];
};

class ana_pixie16 : public ana_module
{
public:
	/* f_dsp is the dsp set file name. */
	ana_pixie16(const char* f_dsp);
	~ana_pixie16();

	/* parse the raw data from pixie16 modules. return 0 if succeed,
	 * otherwise return error code.
	 * */
	int parse_raw(uint32_t* raw_data);

	uint64_t get_ts()         {return pixie16_data.ts;}        
	uint32_t get_cfd()        {return pixie16_data.cfd;}       
	uint32_t get_energy()     {return pixie16_data.energy;}    
	uint32_t get_energy_tr()  {return pixie16_data.energy_tr;} 
	uint32_t get_energy_le()  {return pixie16_data.energy_le;} 
	uint32_t get_energy_ga()  {return pixie16_data.energy_ga;} 
	uint32_t get_qdc0()       {return pixie16_data.qdc0;}      
	uint32_t get_qdc1()       {return pixie16_data.qdc1;}      
	uint32_t get_qdc2()       {return pixie16_data.qdc2;}      
	uint32_t get_qdc3()       {return pixie16_data.qdc3;}      
	uint32_t get_qdc4()       {return pixie16_data.qdc4;}      
	uint32_t get_qdc5()       {return pixie16_data.qdc5;}      
	uint32_t get_qdc6()       {return pixie16_data.qdc6;}      
	uint32_t get_qdc7()       {return pixie16_data.qdc7;}      
	uint16_t* get_wave()      {return pixie16_data.wave;}      

	/* get number of points in a waveform */
	int get_wave_n(int crate, int slot, int ch) {return n_pt[slot][ch];}

	void* get_data() {return &pixie16_data;}

private:

	/* the following functions get the corresponding information from the
	 * header data structure. the results are saved in the corresponding
	 * member varables .*/
	/* get time stamp */
	void _get_ts();
	void _get_ts_ext();
	/* get cfd result */
	void _get_cfd();
	/* get energy */
	void _get_energy();
	/* get energy sum -- trailing */
	void _get_energy_tr();
	/* get energy sum -- leading */
	void _get_energy_le();
	/* get energy sum -- gap */
	void _get_energy_ga();
	/* get qdc0-qdc7 */
	void _get_qdcs();
	
	/* get waveform and fill the array 'wave[MAX_PIXIE16_WAVE]'. if the
	 * waveform length longer than MAX_PIXIE16_WAVE, return error code
	 * (-E_PIXIE_WAVE_TOO_LONG). otherwise return 0. 
	 *  */
	int _get_wave();

	/* parse the dsp set file. Determine the following varables:
	 * 1, the waveform length (number of points) of each channel
	 * 2, the position of pointer for each varable inside the header
	 * (position starts from zero, ts, ts_ext, qdc, energy, etc. set the
	 * pointer to -1 if the corresponding varable is not present in the
	 * header) */
	void parse_dsp(const char* f_dsp);


private:
	int slot;
	int crate;
	int ch;
	uint32_t* hd_ptr;
	
	struct pixie16_data pixie16_data;

	/* n_pt is the number of points in a waveform. 
	 * remember that crate number is slot number for pixie16 cards */
	int n_pt[MAX_CRATE][16];
	int ptr_qdc[MAX_CRATE][16];
	int ptr_energy_ga[MAX_CRATE][16];
	int ptr_energy_le[MAX_CRATE][16];
	int ptr_energy_tr[MAX_CRATE][16];
	int ptr_energy[MAX_CRATE][16];
	int ptr_cfd[MAX_CRATE][16];
	int ptr_ts_ext[MAX_CRATE][16];
	int ptr_ts[MAX_CRATE][16];

};

#endif 
