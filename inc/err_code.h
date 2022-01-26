/* a list of error code with different meanings indicating different errors,
 * should be used throughout the DAQ.
 * By B.Gao Jul. 2018 */

#ifndef ERR_CODE_HHH
#define ERR_CODE_HHH

#define RET_IF_NONZERO(x) if (x) return x

#define E_NOT_IMPLE            1 /* function not implemented */
#define E_VME_GENERIC          2 /* unspecified error during the vme cycle */
#define E_VME_BUS              3 /* VME buss error */
#define E_VME_COMM             4 /* VME communication error */
#define E_GENERIC              5 /* generic error */
#define E_FILE_O               6 /* cannot open file */
#define E_FILE_PAR             7 /* cannot parse the file (xml file) */
#define E_MEMORY               8 /* not enough memroy*/
#define E_UNKOWN_MOD           9 /* unknown module */
#define E_PARSER               10
#define E_INIT_MADC32          11
#define E_INIT_V1190           12
#define E_INIT_V830            13
#define E_INIT_V2718           14
#define E_CRATE_NUM            15
#define E_VME_CTRL             16
#define E_V1190_OFFSET         17
#define E_AM                   18
#define E_MAX_MODULE           19
#define E_IO                   20
#define E_SYSCALL              21 /* system call error */
#define E_RING_BUF_MSG         22 /* error communicating using the message ring buffer */
#define E_MSG_TOO_BIG          23 /* message too big in the message ring buffer */
#define E_MSG_TYPE             24 /* unknown message type in the message ring buffer */
#define E_TRIG_MOD             25 /* trigger module not found */
#define E_RING_BUF_DATA        26 /* error communicating using the data ring buffer */
#define E_ON_START_TOUT        27 /* time out in the on_start() functions */
#define E_EVT_TYPE             28 /* unknown event type */
#define E_DATA_MADC32          29 /* corrupted madc32 data */
#define E_MAX_CRATE            30 /* MAX_CRATE too small */
#define E_MAX_GEO              31 /* MAX_GEO too small */
#define E_INIT_MODULE          32 /* error in the module initialization */
#define E_DATA_V1190           33 /* corrupted v1190 data */
#define E_V1190_CLOCK          34 /* The v1190 has wrong clock freqency (not 40MHz)*/
#define E_SYNC_CLOCK           35 /* clock related errors (this clock is used
				     to build events) */
#define E_MERGE_BUF_SZ         36 /* the merged buffer size too small */
#define E_OPEN_FILE            37 /* cannot open file*/
#define E_CLOSE_FILE           38 /* cannot close file*/
#define E_NOT_SUPPORT          39 /* not supported */
#define E_EBD_EVT_BUF_SZ       40 /* single event buffer for ebd is too small */
#define E_DATA_V1740           41 /* corrupted v1740 data */
#define E_INIT_V1740           42 /* error initialization of v1740 */
#define E_GET_SVR_ADDR         43 /* error getting server address */
#define E_EBD_TYPE             44 /* wrong event builder type (ts/evt cnt) */
#define E_INIT_V775            45 /* error in the v775 initialization */
#define E_DATA_V775            46 /* corrupted v775 data */
#define E_DATA_V775N           47 /* corrupted v775n data */
#define E_DATA_V785            48 /* corrupted v785 data */
#define E_INIT_V785            49 /* error in the v775 initialization */
#define E_DATA_V785N           50 /* corrupted v785 data */
#define E_DATA_EVT             51 /* corrupted whole event */
#define E_INIT_TEST_CTL        52 /* error initializing the test_ctl module */
#define E_DATA_FAKE_MODULE     53 /* data error of the fake module */
#define E_V1740_N_PT           54 /* unknow v1740 number of points */
#define E_DATA_V830            55 /* corrupted v830 data */
#define E_INIT_V1751           56 /* error initializing v1751 */
#define E_V1751_N_PT           57 /* unknow v1751 number of points */
#define E_DATA_V1751           58 /* corrupted v1751 data */
#define E_INIT_MQDC32          59 
#define E_DATA_MQDC32          60 /* corrupted mqdc32 data */
#define E_INIT_V792            61 
#define E_DATA_V792            62 /* corrupted v792 data */
#define E_DONT_CALL            63 /* function should not be called */
#define E_PIXIE_IO             64 /* pixie I/O error */
#define E_PIXIE_START_RUN      65 /* error to start pixie new run */
#define E_PIXIE_STOP_RUN       66 /* error to start pixie new run */
#define E_PIXIE_CHECK_FIFO_STAT 67 /* error to check pixie fifo status */
#define E_PIXIE_EXT_CLK        68 /* error to get externel ts from pixie data */
#define E_XIA_SLOW_MODE        69 /* DAQ_XIA and USE_SLOW_MODE are both defined
				     (they are not compatible with each other
				     */
#define E_PIXIE_INIT           70 /* error init pixie system */
#define E_ComFPGAConfigFile    71 /* cannot open or invalid ComFPGAConfigFile */
#define E_SPFPGAConfigFile     72 /* cannot open or invalid SPFPGAConfigFile */
#define E_DSPCodeFile          73 /* cannot open or invalid DSPCodeFile      */
#define E_DSPParFile           74 /* cannot open or invalid DSPParFile       */
#define E_DSPVarFile           75 /* cannot open or invalid DSPVarFile       */
#define E_PIXIE_GENERAL        76 /* general pixie error (check Pixie16msg.txt) */
#define E_PIXIE_MOD_NUM        77 /* invalid pixie module number */
#define E_BLT_BUFF_SZ          78 /* blt buffer size too small */
#define E_INIT_PIXIE16_CTL     79 /* error in the pixie16 controller init */
#define E_INIT_PIXIE16         80 /* error in the pixie16 module init */
#define E_PIXIE_UNKNOW_REG     81 /* unknown pixei16 register */
#define E_PIXIE_SYNC           82 /* synchronization problems with pixei16
				     modules */
#define E_PIXIE_WAVE_TOO_LONG  83 /* pixei16 waveform too long, need to
				     increase MAX_PIXIE16_WAVE */
#define E_PIXIE_LOAD_DSP       84 /* cannot load dsp parameters from file */




#endif 
