/* This class represents the pixie16 controller ,which unlike vme controllers,
 * does not physically exists. We also derive this class from vme_ctl class
 * though it sounds weird. But their lowest level principle of operations are
 * similar: open, read, write, close.
 * By B.Gao Nov. 2021 */

#ifndef PIXIE16_CTL_HHH
#define PIXIE16_CTL_HHH
#include "vme_ctl.h"
#include <stdint.h>

#ifdef DAQ_XIA
#include "pixie16app_export.h"

/* struct defines copied from pixie16app_globals.h */
#define ROUND(x)    ((x) < 0.0 ? ceil((x) - 0.5) : floor((x) + 0.5))
struct Pixie_Configuration
{
	
	// DSP I/O parameter values
	unsigned int DSP_Parameter_Values[N_DSP_PAR];

};
struct Module_Info
{
	// Module information
	unsigned short Module_Rev;
	unsigned int   Module_SerNum;
	unsigned short Module_ADCBits;
	unsigned short Module_ADCMSPS;
	unsigned short Module_OfflineVariant;
};

struct pixie16_ctl_open_par
{
	unsigned short mod_num;        /* total number of modules */
	unsigned short* pxi_slot_map;  /* an array containing the slot number
					  of each Pixie-16 module */
	const char *ComFPGAConfigFile; /* name of ComFPGA configuration file */
	const char *SPFPGAConfigFile;  /* name of SPFPGA configuration file */ 
	const char *TrigFPGAConfigFile;/* name of trigger FPGA file */         
	const char *DSPCodeFile;       /* name of DSP code file */             
	const char *DSPParFile;        /* name of DSP parameter file */        
	const char *DSPVarFile;        /* name of DSP variable names file */   
};

class pixie16_ctl : public vme_ctl
{
public:
	pixie16_ctl(); 
	~pixie16_ctl() {};

	void set_max_evt_sz(unsigned int sz) {evt_max_sz = sz;}
	/* open the device 
	 * @par a pointer of parameters 
	 * @return 0 if succeed, non-zero error codes are returned if error */
	int open(void* par);

	/* close the device 
	 * @return 0 if succeed, non-zero error codes are returned if error */
	int close();

	/* I must admit that I misunderstood the meanings of read/write and
	 * read_reg/write/reg. In the case of vme controllers, read/write are
	 * related to vme modules while the read_reg/write_reg are related to
	 * the controller itself. However, here in the pixie16, there meanings
	 * are opposite: read/write are for controller and read_reg/write_reg
	 * are for modules. */
	
	/* read/write controller register. Since the pixie16 controller does
	 * not physically exist at all, here the register is just regarded as
	 * parameters with address as their identifier */
	int read(unsigned long addr, void* data);
	int write(unsigned long addr, void* data);

	/* in the case of pixie16, reg is the parameter offset, am is the
	 * module number, dw is the channel number. */
	int read_reg(long reg, void* data);
	int write_reg(long reg, void* data);

	/* read event data from pixie16 modules. We use the
	 * Pixie16ReadDataFromExternalFIFO function. Since this function is not
	 * event-aligned readout, we check if the data read out truncated the
	 * last event and, if necessary, do a second read out to restore the
	 * 'event-aligned' feature.
	 * @param addr module number
	 * @param buf buffer where the data will be read out to
	 * @param sz_in size of data to read (in bytes)
	 * @param sz_out size of data actually read (in bytes).
	 * @return 0 if succeed, non-zero error codes in case of error. */
	int blt_read(unsigned long addr, void* buf, int sz_in, int* sz_out);
	int mblt_read(unsigned long addr, void* buf, int sz_in, int* sz_out);
	
	/* the out port 1 (the second port) will be used */
	virtual int send_pulse(bool invt);
	/* the out port 0 (the first port) will be used */
	virtual int send_pulse1(bool invt);
	/* the out port 2 (the third port) will be used */
	virtual int send_pulse2(bool invt);

	void set_fifo_thresh(uint32_t thresh) {fifo_rd_thresh = thresh;}
	uint32_t get_fifo_thresh() {return fifo_rd_thresh;}

	/* load dsp set parameters from file. this is only needed if reg_acc ==
	 * 1. return 0 if succeed, otherwise return error code  */
	int try_load_dsp();

private:
	/* we need the artifical_write and artifical_read because the pixie16
	 * read event buffer always read >=3 32-bit words, even the n_word = 1
	 * or 2 is specified. Therefore, we need to temporarily store the extra
	 * words for the next readout.*/
	unsigned int artifical_read(unsigned int* buf)
	{
		unsigned int ret;
		if (n_word_art == 1) {
			buf[0] = art_data[0];
		}
		else if (n_word_art == 2) {
			buf[0] = art_data[0];
			buf[1] = art_data[1];
		}
		ret = n_word_art;
		n_word_art = 0;
		return ret;
	}
	void artifical_write(unsigned int* buf, int n_word) 
	{
		if (n_word >= 3) {
			n_word_art = 0;
		}
		else if (n_word == 2) {
			n_word_art = 1;
			art_data[0] = buf[2];
		}
		else if (n_word == 1) {
			n_word_art = 2;
			art_data[0] = buf[1];
			art_data[1] = buf[2];
		}
	}
	/* check if the events in the buf are event aligned.
	 * @param buf buffer containing the events
	 * @param n_word length of the events in the buffer (number of 32-bit
	 * words)
	 * @return the number of remaining number of words should be readout to
	 * make it event-aligned.
	 * */
	unsigned int check_evt_align(uint32_t* buf, unsigned int n_word);

	/* set the module CSRB register bit 
	 * @param mod_n module number
	 * @param bit_n bit number to set
	 * @param val bit value to set 
	 * @return 0 if succeed, otherwise return error code. 
	 * */
	int set_modCSRB_bit(unsigned short mod_n, uint16_t bit_n, uint16_t val);

	/* set module/channel register bit 
	 * @param mod_n module number
	 * @param ch_n channel number
	 * @param par_name parameter name to set
	 * @param start_bit starting bit number to set
	 * @param n_bit number of bit to set
	 * @param bit_val new bit value to set 
	 * @return 0 if succeed, otherwise return error code. 
	 * */
	int set_modreg_bit(unsigned short mod_n, const char* par_name, uint16_t
			start_bit, uint16_t n_bit, uint32_t bit_val);
	int set_chreg_bit(unsigned short mod_n, unsigned short ch_n, const
			char* par_name, uint16_t start_bit, uint16_t n_bit,
			uint32_t bit_val);

	/* set the channel CSRA register bit 
	 * @param mod_n module number
	 * @param ch_n channel number
	 * @param bit_n bit number to set
	 * @param val bit value to set 
	 * @return 0 if succeed, otherwise return error code. 
	 * */
	int set_chCSRA_bit(unsigned short mod_n, unsigned short ch_n, uint16_t
			bit_n, uint16_t val);

	/* write the filter parameters into dsp. because those parameters are
	 * not independent, they have to be written in some certain order,
	 * therefore, in this function we collect all those parameters and once
	 * they are all there written into dsp in some certain order.
	 * @param mod_n the module number 
	 * @param ch_n  the channel number
	 * @return 0 if succeed, otherwise return error code. */
	int do_conf_filter_pars(uint16_t mod_n, uint16_t ch_n);

	/* similarly as do_conf_filter_pars, here we config the baseline cut */
	int do_conf_blcut(uint16_t mod_n, uint16_t ch_n);

	/* similarly as do_conf_filter_pars, here we config the group trigger
	 * selections (TrigConfig1 bit0-31, TrigConfig2 bit0-23) for group
	 * 'grp_n' of module 'mod_n'*/
	int do_conf_group_trig(int grp_n, uint16_t mod_n, uint16_t ch_n);

	double set_bits_double(double val, uint16_t start_bit, uint16_t n_bit,
			uint32_t bit_val)
	{
		uint32_t val32 = (uint32_t)val;
		return (double)set_bits_int(val32, start_bit, n_bit, bit_val);
	}

	uint32_t set_bits_int(uint32_t val32, uint16_t start_bit, uint16_t n_bit,
			uint32_t bit_val)
	{
		uint32_t tmp32 = ((1L<<n_bit) - 1) << start_bit;
		val32 &= ~tmp32;
		val32 |= bit_val << start_bit;
		return val32;
	}

	/* read dsp parameters from ./pixie16_firmware/initial_par.set and
	 * initialize the Pixie_Devices array. return 0 if succeed, otherwise return
	 * error code.  */
	int read_dsp_par();

	/* boot pixie16 modules, return 0 if succeed, otherwise return error
	 * code */
	int boot_modules();

	/* The following myPixie16* functions are analogous to the Pixie16*
	 * functions from the sdk. However, the myPixie16* functions modify
	 * only the dsp parameters stored in the array Pixie_Devices, no io
	 * operations. The implementation of those functions are based on the
	 * corresponding functions in the sdk. 
	 * always return 0 since no io operations. */
	int myPixie16WriteSglChanPar(const char* par_name, double par_data, int
			mod_n, int ch_n);
	int myPixie16ReadSglChanPar(const char* par_name, double* par_data, int
			mod_n, int ch_n);
	int myPixie16ReadSglModPar(const char* par_name, uint32_t* par_data,
			int mod_n);
	int myPixie16WriteSglModPar(const char* par_name, uint32_t par_data,
			int mod_n);
	int myPixie_ComputeFIFO(unsigned int tracedelay, int mod_n, int ch_n);
	int myPixie_Init_DSPVarAddress(const char* DSPVarFile, int mod_n);
	int myPixie_Copy_DSPVarAddress(int mod_n, int mod_n1);


private:
	/* artifical data */
	unsigned int art_data[2];
	int n_word_art;
	/* total number of pixie16 modules installed in the system */
	unsigned short mod_num;
	/* max size of single event (number of 32-bit words) */
	unsigned int evt_max_sz;

	/* Booting module parameters */
	char ComFPGAConfigFile[1000];
	char SPFPGAConfigFile[1000];
	char TrigFPGAConfigFile[1000];
	char DSPCodeFile[1000];
	char DSPParFile[1000];
	char DSPVarFile[1000];

	/* below are a list of dsp parameters.  We store those parameters
	 * because their values are not independent, so they have to be written
	 * into the pixie16 module in some certain order.*/
	int    T_Filter_Range[MAX_CRATE][16];
	int    E_Filter_Range[MAX_CRATE][16];
	int    auto_blcut[MAX_CRATE][16];
	int    group0_trigLMR[MAX_CRATE][16];
	int    group1_trigLMR[MAX_CRATE][16];
	int    group2_trigLMR[MAX_CRATE][16];
	int    group3_trigLMR[MAX_CRATE][16];
	int    group0_trigCh[MAX_CRATE][16];
	int    group1_trigCh[MAX_CRATE][16];
	int    group2_trigCh[MAX_CRATE][16];
	int    group3_trigCh[MAX_CRATE][16];
	double trace_length[MAX_CRATE][16];
	double trace_delay[MAX_CRATE][16];
	double T_Risetime_us[MAX_CRATE][16];
	double T_FlatTop_us[MAX_CRATE][16];
	double T_Threshold[MAX_CRATE][16];
	double E_FlatTop_us[MAX_CRATE][16];
	double E_Risetime_us[MAX_CRATE][16];
	double bl_cut[MAX_CRATE][16];

	/* fifo threshold (see comments in pixie16.cpp). This parameter is not
	 * used by the controller, but by the trigger module. The varable here
	 * is just used to pass the value to the trigger module because this
	 * varable is defined as a parameter of the controller. */
	uint32_t fifo_rd_thresh;

	/* if reset the clock for new runs */
	uint32_t reset_clk;

	/* if or not the register access should be performed. If the dsp
	 * settings are initiated when booting the module, then reg_acc should
	 * be 0, otherwise reg_acc should be 1.*/
	int reg_acc;
	
	/* slot map */
	uint32_t pxi_slot_map[MAX_CRATE];

	/* dsp parameters and module information (for up to MAX_CRATE pixie16
	 * modules). The name and structure of this parameter are defined to be
	 * the same as those used in Pixie16* functions (provided by the
	 * Pixie16 sdk). So that the code from the sdk can be directly copied
	 * and used in the functions myPixie16*  */
	struct Pixie_Configuration Pixie_Devices[MAX_CRATE];
	struct Module_Info Module_Information[MAX_CRATE];

	/* The following variables are also copied from pixie16 sdk */
	unsigned int ModNum_Address[PRESET_MAX_MODULES];         // ModNum
	unsigned int ModCSRA_Address[PRESET_MAX_MODULES];        // ModCSRA
	unsigned int ModCSRB_Address[PRESET_MAX_MODULES];        // ModCSRB
	unsigned int ModFormat_Address[PRESET_MAX_MODULES];      // ModFormat
	unsigned int RunTask_Address[PRESET_MAX_MODULES];        // RunTask
	unsigned int ControlTask_Address[PRESET_MAX_MODULES];    // ControlTask
	unsigned int MaxEvents_Address[PRESET_MAX_MODULES];      // MaxEvents
	unsigned int CoincPattern_Address[PRESET_MAX_MODULES];   // CoincPattern
	unsigned int CoincWait_Address[PRESET_MAX_MODULES];      // CoincWait
	unsigned int SynchWait_Address[PRESET_MAX_MODULES];      // SynchWait
	unsigned int InSynch_Address[PRESET_MAX_MODULES];        // InSynch
	unsigned int Resume_Address[PRESET_MAX_MODULES];         // Resume
	unsigned int SlowFilterRange_Address[PRESET_MAX_MODULES];// SlowFilterRange
	unsigned int FastFilterRange_Address[PRESET_MAX_MODULES];// FastFilterRange
	unsigned int ChanNum_Address[PRESET_MAX_MODULES];        // ChanNum
	unsigned int HostIO_Address[PRESET_MAX_MODULES];         // HostIO
	unsigned int UserIn_Address[PRESET_MAX_MODULES];         // UserIn
	unsigned int U00_Address[PRESET_MAX_MODULES];            // U00
	unsigned int FastTrigBackplaneEna_Address[PRESET_MAX_MODULES]; // Enabling sending fast trigger to backplane
	unsigned int CrateID_Address             [PRESET_MAX_MODULES]; // CrateID
	unsigned int SlotID_Address              [PRESET_MAX_MODULES]; // SlotID
	unsigned int ModID_Address               [PRESET_MAX_MODULES]; // ModID
	unsigned int TrigConfig_Address          [PRESET_MAX_MODULES]; // General configuration registers
	unsigned int HRTP_Address[PRESET_MAX_MODULES];           // HostRunTimePreset

	//--------------------
	//	Channel parameters
	//--------------------

	unsigned int ChanCSRa_Address[PRESET_MAX_MODULES];       // ChanCSRa
	unsigned int ChanCSRb_Address[PRESET_MAX_MODULES];       // ChanCSRb

	unsigned int GainDAC_Address[PRESET_MAX_MODULES];        // GainDAC

	unsigned int OffsetDAC_Address[PRESET_MAX_MODULES];      // OffsetDAC

	unsigned int DigGain_Address[PRESET_MAX_MODULES];        // DigGain

	unsigned int SlowLength_Address[PRESET_MAX_MODULES];     // SlowLength
	unsigned int SlowGap_Address[PRESET_MAX_MODULES];        // SlowGap
	unsigned int FastLength_Address[PRESET_MAX_MODULES];     // FastLength
	unsigned int FastGap_Address[PRESET_MAX_MODULES];        // FastGap
	unsigned int PeakSample_Address[PRESET_MAX_MODULES];     // PeakSample
	unsigned int PeakSep_Address[PRESET_MAX_MODULES];        // PeakSep

	unsigned int CFDThresh_Address[PRESET_MAX_MODULES];      // CFDThresh

	unsigned int FastThresh_Address[PRESET_MAX_MODULES];     // FastThresh
	unsigned int ThreshWidth_Address[PRESET_MAX_MODULES];    // ThreshWidth
	unsigned int PAFlength_Address[PRESET_MAX_MODULES];      // PAFlength
	unsigned int TriggerDelay_Address[PRESET_MAX_MODULES];   // TriggerDelay
	unsigned int ResetDelay_Address[PRESET_MAX_MODULES];     // ResetDelay
	unsigned int ChanTrigStretch_Address[PRESET_MAX_MODULES];// ChanTrigStretch
	unsigned int TraceLength_Address[PRESET_MAX_MODULES];    // TraceLength
	unsigned int TrigOutLen_Address[PRESET_MAX_MODULES];     // TrigOutLen
	unsigned int EnergyLow_Address[PRESET_MAX_MODULES];      // EnergyLow
	unsigned int Log2Ebin_Address[PRESET_MAX_MODULES];       // Log2Ebin

	unsigned int MultiplicityMaskL_Address[PRESET_MAX_MODULES]; // Multiplicity contribution mask - low 32-bit

	unsigned int PSAoffset_Address[PRESET_MAX_MODULES];      // PSAoffset
	unsigned int PSAlength_Address[PRESET_MAX_MODULES];      // PSAlength
	unsigned int Integrator_Address[PRESET_MAX_MODULES];     // Integrator

	unsigned int BLcut_Address[PRESET_MAX_MODULES];          // BLcut
	unsigned int BaselinePercent_Address[PRESET_MAX_MODULES];// BaselinePercent

	unsigned int FtrigoutDelay_Address[PRESET_MAX_MODULES];  // Fast trigger output delay for system synchronization

	unsigned int Log2Bweight_Address[PRESET_MAX_MODULES];    // Log2Bweight
	unsigned int PreampTau_Address[PRESET_MAX_MODULES];      // PreampTau

	unsigned int MultiplicityMaskH_Address[PRESET_MAX_MODULES]; // Multiplicity contribution mask - high 32-bit

	unsigned int FastTrigBackLen_Address[PRESET_MAX_MODULES];	// FastTrigBackLen

	unsigned int CFDDelay_Address      [PRESET_MAX_MODULES];	// CFD delay
	unsigned int CFDScale_Address      [PRESET_MAX_MODULES];	// CFD scale
	unsigned int ExternDelayLen_Address[PRESET_MAX_MODULES];	// delay length for each channel's input signal
	unsigned int ExtTrigStretch_Address[PRESET_MAX_MODULES];	// external trigger stretch
	unsigned int VetoStretch_Address   [PRESET_MAX_MODULES];	// veto stretch
	unsigned int QDCLen0_Address       [PRESET_MAX_MODULES];	// QDC #0 length
	unsigned int QDCLen1_Address       [PRESET_MAX_MODULES];	// QDC #1 length
	unsigned int QDCLen2_Address       [PRESET_MAX_MODULES];	// QDC #2 length
	unsigned int QDCLen3_Address       [PRESET_MAX_MODULES];	// QDC #3 length
	unsigned int QDCLen4_Address       [PRESET_MAX_MODULES];	// QDC #4 length
	unsigned int QDCLen5_Address       [PRESET_MAX_MODULES];	// QDC #5 length
	unsigned int QDCLen6_Address       [PRESET_MAX_MODULES];	// QDC #6 length
	unsigned int QDCLen7_Address       [PRESET_MAX_MODULES];	// QDC #7 length

	unsigned int Xwait_Address[PRESET_MAX_MODULES];				// Xwait


	//--------------------
	//	Results parameters
	//--------------------

	unsigned int RealTimeA_Address[PRESET_MAX_MODULES];    // RealTimeA
	unsigned int RealTimeB_Address[PRESET_MAX_MODULES];    // RealTimeB
	unsigned int RunTimeA_Address[PRESET_MAX_MODULES];     // RunTimeA
	unsigned int RunTimeB_Address[PRESET_MAX_MODULES];     // RunTimeB
	unsigned int GSLTtime_Address[PRESET_MAX_MODULES];     // GSLTtime
	unsigned int NumEventsA_Address[PRESET_MAX_MODULES];   // NumEventsA
	unsigned int NumEventsB_Address[PRESET_MAX_MODULES];   // NumEventsB
	unsigned int DSPerror_Address[PRESET_MAX_MODULES];     // DSPerror
	unsigned int SynchDone_Address[PRESET_MAX_MODULES];    // SynchDone
	unsigned int BufHeadLen_Address[PRESET_MAX_MODULES];   // BufHeadLen
	unsigned int EventHeadLen_Address[PRESET_MAX_MODULES]; // EventHeadLen
	unsigned int ChanHeadLen_Address[PRESET_MAX_MODULES];  // ChanHeadLen
	unsigned int UserOut_Address[PRESET_MAX_MODULES];      // UserOut
	unsigned int AOutBuffer_Address[PRESET_MAX_MODULES];   // AOutBuffer
	unsigned int LOutBuffer_Address[PRESET_MAX_MODULES];   // LOutBuffer
	unsigned int AECorr_Address[PRESET_MAX_MODULES];       // AECorr
	unsigned int LECorr_Address[PRESET_MAX_MODULES];       // LECorr
	unsigned int HardwareID_Address[PRESET_MAX_MODULES];   // HardwareID
	unsigned int HardVariant_Address[PRESET_MAX_MODULES];  // HardVariant
	unsigned int FIFOLength_Address[PRESET_MAX_MODULES];   // FIFOLength
	unsigned int FippiID_Address[PRESET_MAX_MODULES];      // FippiID
	unsigned int FippiVariant_Address[PRESET_MAX_MODULES]; // FippiVariant
	unsigned int DSPrelease_Address[PRESET_MAX_MODULES];   // DSPrelease
	unsigned int DSPbuild_Address[PRESET_MAX_MODULES];     // DSPbuild
	unsigned int DSPVariant_Address[PRESET_MAX_MODULES];   // DSPVariant
	unsigned int U20_Address[PRESET_MAX_MODULES];          // U20
	unsigned int LiveTimeA_Address[PRESET_MAX_MODULES];    // LiveTimeA
	unsigned int LiveTimeB_Address[PRESET_MAX_MODULES];    // LiveTimeB
	unsigned int FastPeaksA_Address[PRESET_MAX_MODULES];   // FastPeaksA
	unsigned int FastPeaksB_Address[PRESET_MAX_MODULES];   // FastPeaksB
	unsigned int OverflowA_Address[PRESET_MAX_MODULES];    // OverflowA
	unsigned int OverflowB_Address[PRESET_MAX_MODULES];    // OverflowB
	unsigned int InSpecA_Address[PRESET_MAX_MODULES];      // InSpecA
	unsigned int InSpecB_Address[PRESET_MAX_MODULES];      // InSpecB
	unsigned int UnderflowA_Address[PRESET_MAX_MODULES];   // UnderflowA
	unsigned int UnderflowB_Address[PRESET_MAX_MODULES];   // UnderflowB
	unsigned int ChanEventsA_Address[PRESET_MAX_MODULES];  // ChanEventsA
	unsigned int ChanEventsB_Address[PRESET_MAX_MODULES];  // ChanEventsB
	unsigned int AutoTau_Address[PRESET_MAX_MODULES];      // AutoTau
	unsigned int U30_Address[PRESET_MAX_MODULES];          // U30


};
#endif

#endif
