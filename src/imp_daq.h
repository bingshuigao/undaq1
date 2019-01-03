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


/* default socket settings */
/* listening port of the GUI controller for frontend */
#define DEF_PORT_CTL_FE 9090;
/* listening port of the GUI controller for event builder */
#define DEF_PORT_CTL_EBD 9091;
/* server address of the frontend control thread */
#define DEF_SVR_CTL_FE "127.0.0.1" 
/* frontend sender linsening port */
#define DEF_PORT_SEND_FE 9092
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

/* run status */
#define DAQ_STOP 0
#define DAQ_RUN 1
#define DAQ_EXIT 2

/* array lengths */
#define MAX_CRATE 10
#define MAX_MODULE_ID 50
#define MAX_GEO 32
#define MAX_MODULE 21 
#define MAX_SLOT_MAP (MAX_CRATE*MAX_MODULE*MAX_GEO)
#define SLOT_MAP_IDX(crate, mod_id, geo) ((crate)*MAX_MODULE_ID*MAX_GEO + (mod_id)*MAX_GEO + (geo))

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

static inline int do_send(int sock, void* buf, int sz, int flag)
{
	unsigned char* p_buf = reinterpret_cast<unsigned char*>(buf);
	while (sz > 0) {
		int ret = send(sock, p_buf, sz, flag);
		if (ret == -1) 
			return -E_SYSCALL;
		sz -= ret;
		p_buf += ret;
	}
	return 0;
}




#endif 
