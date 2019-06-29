/* Common functions across all source files are defined here 
 *
 * By B.Gao Dec. 2018
 * */

#ifndef IMP_DAQ_HHH
#define IMP_DAQ_HHH

#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include "err_code.h"
#include <iostream>
#include <errno.h>
#include <unistd.h>

/* default sizes for buffers, please note that the actual size of the ring
 *  * buffer is only half of the defined values below.  */
/********** event builder buffers *************/
/* raw data received from frontend */
#define DEF_RB_EBD_RAW (1024*1024*40)
/* built event */
#define DEF_RB_EBD_BLD (1024*1024*80)
/* individual ring buffers for each vme module */
#define DEF_RB_EBD_MOD (1024*1024*4)
/* message sharing buffer */
#define DEF_RB_EBD_MSG (1024*2)
/* message sharing buffer */
#define DEF_RB_EBD_SCL (1024*1024)
/* event builder sender socket buffer size */
#define DEF_SOCK_BUF_EBD_SEND (1024*1024)
/* merged event buffer size */
#define DEF_EBD_MERGED_BUF_SZ (1024*1024*8)
/* max single event length */
#define DEF_EBD_MAX_EVT_LEN (1024*1024)
/********** frontend buffers *************/
/* data sharing */
#define DEF_RB_FE_DATA (1024*1024*40)
/* message sharing */
#define DEF_RB_FE_MSG (1024*2)
/* control thread socket buffer size */
#define DEF_SOCK_BUF_FE_CTL (1024*1024)
/* frontend sender socket buffer size */
#define DEF_SOCK_BUF_FE_SEND (1024*1024)
/* frontend blt buffer size */
#define DEF_BLT_BUF_FE (1024*1024*10)
/********** logger buffers *************/
/* trigger data sharing */
#define DEF_RB_LOG_TRIG (1024*1024*10)
/* message sharing */
#define DEF_RB_LOG_MSG (1024*2)
/* scaler data */
#define DEF_RB_LOG_SCAL (1024*1024*2)
/* logger save buf */
#define DEF_LOG_SAVE_BUF (1024*1024*2)
/* analyzer buffer */
#define DEF_RB_ANA (1024*1024*10)
#define DEF_MAX_EVT_LEN (1024*1024*2);


/* default socket settings */
/* listening port of the GUI controller  */
#define DEF_PORT_CTL 9090;
/* server address of the frontend control thread */
#define DEF_SVR_CTL_FE "127.0.0.1" 
/* frontend sender linsening port */
#define DEF_PORT_SEND_FE 9092
/* event builder sender linsening port */
#define DEF_PORT_SEND_EBD 9093
/* The analyzer roody server port*/
#define DEF_ANA_ROODY_SVR_PORT 9094
/* server address of the event builder receiver thread */
#define DEF_SVR_RECV_EBD "127.0.0.1"


/* default timeouts */
/* time out (micro sec) of the select syscall of the frontend control thread */
#define DEF_T_US_CTL_FE 10000
/* time out of the frontend sender thread to check data ring buffer */
#define DEF_ITV_SEND_FE 100000
/* time out of the event builder thread to check the incoming data packet from
 *  * socket */
#define DEF_T_US_RECV_EBD 10000
/* The max number of seconds allowed to spend on the on_start() functions */
#define DEF_RD_ON_START 3

/* default clock frequencies */
#define DEF_MADC32_CLK 16000000
#define DEF_V1190_CLK 40000000
#define DEF_V1740_CLK 125000000


/* run status */
#define DAQ_STOP 0
#define DAQ_RUN 1
#define DAQ_EXIT 2

/* array lengths */
#define MAX_CRATE 10
#define MAX_MODULE_ID 50
#define MAX_GEO 32
#define MAX_MODULE 21 
#define MAX_SLOT_MAP (MAX_CRATE*MAX_MODULE_ID*MAX_GEO)
#define MAX_CLK_MAP (MAX_CRATE*MAX_MODULE)
#define SLOT_MAP_IDX(crate, mod_id, geo) ((crate)*MAX_MODULE_ID*MAX_GEO + (mod_id)*MAX_GEO + (geo))
#define CLK_MAP_IDX(crate, slot) ((crate)*MAX_MODULE + slot)

/* thread id numbers */
#define EBD_RECV 1
#define EBD_SORT 2
#define EBD_MERG 3
#define EBD_SEND 4
#define EBD_CTRL 5



/* Others */
#define DEF_NTRY_FE 1000 
#define DEF_EBD_SORT_HZ 10000000
#define DEF_EBD_MERGE_GLOM 10
#define DEF_SAVE_PATH_LOG "."

static inline int do_send(int sock, void* buf, int sz, int flag)
{
	unsigned char* p_buf = reinterpret_cast<unsigned char*>(buf);
	while (sz > 0) {
		int ret = send(sock, p_buf, sz, flag);
		if (ret == -1) {
			/* check error number */
			if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
				usleep(20);
				continue;
			}
			return -E_SYSCALL;
		}
		sz -= ret;
		p_buf += ret;
	}
	return 0;
}




#endif 
