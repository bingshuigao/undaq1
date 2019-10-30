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
#define E_SYNC_CLOCK           35 /* clock related errors (this clock is used to build events) */
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




#endif 
