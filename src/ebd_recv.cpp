#include "ebd_recv.h"
#include "my_tcp_clt.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <unistd.h>
#include <iostream>

ebd_recv::ebd_recv(int total, int n)
{
	thread_id = 1 + 10*n;
	total_thread = total;
	init_fun.push_back(&ebd_recv_init);
}

ebd_recv::~ebd_recv()
{
}

int ebd_recv::ebd_recv_init(my_thread* This, initzer* the_initzer)
{
	ebd_recv* ptr = reinterpret_cast<ebd_recv*>(This);

	ptr->port = the_initzer->get_fe_sender_port();
	ptr->svr_addr = the_initzer->get_ebd_recv_svr_addr((ptr->thread_id-1)/10);
	if (ptr->svr_addr.empty())
		return -E_GET_SVR_ADDR;
	/* debug ...*/
//	std::cout<<ptr->svr_addr<<std::endl;
	/* ***************/
	ptr->recv_buf_sz = the_initzer->get_fe_sender_buf_sz();
	ptr->sock_buf = new unsigned char[ptr->recv_buf_sz];
	ptr->t_us = the_initzer->get_ebd_recv_t_us();

	return 0;
}

int ebd_recv::handle_msg(uint32_t* msg_body)
{
	/* The message type of the current thread are defined as following 
	 * 1 --> run status transition
	 * 2 --> To be defined yet 
	 * */
	uint32_t msg_type = msg_body[0] & 0xFFFFFF;
	switch (msg_type) {
	case 1:
		/* run status transition
		 * */
		return switch_run(msg_body[1]);
	default:
		return -E_MSG_TYPE;
	}

}

int ebd_recv::start()
{
	acq_stat = DAQ_RUN;
	int flag = sock;
	int ret = recv_start();

	RET_IF_NONZERO(ret);
	
	n_stops = 0;
	/* receive the slot map (and others )and ring buffer data if not yet.*/
	if (flag == -1) {
		int sz;
		char* p_slot_map[2];
		uint64_t* p_clk_map[2];
		uint64_t* p_clk_off_map[2];
		
		/* ring buffer */
		ret = init_rb_data();
		RET_IF_NONZERO(ret);
		/* tell the ebd_merge that the rb_data (for this frontend) is
		 * ready */
		ret = send_msg(EBD_MERG, 2, &total_thread, 4);
		RET_IF_NONZERO(ret);
		
		/* slot map */
		sz = MAX_SLOT_MAP;
		ret = recv(sock, slot_map, sz, MSG_WAITALL);
		if (ret != sz)
			return -E_SYSCALL;
		/* tell the ebd_sort the address of the slot map */
		p_slot_map[0] = slot_map;
		p_slot_map[1] = reinterpret_cast<char*>(total_thread);
		ret = send_msg(EBD_SORT, 2, p_slot_map, sizeof(char*)*2);
		RET_IF_NONZERO(ret);
		
		/* clock map */
		sz = MAX_CLK_MAP*8;
		ret = recv(sock, clk_map, sz, MSG_WAITALL);
		if (ret != sz)
			return -E_SYSCALL;
		/* tell the ebd_sort the address of the clk map */
		p_clk_map[0] = clk_map;
		p_clk_map[1] = reinterpret_cast<uint64_t*>(total_thread);
		ret = send_msg(EBD_SORT, 3, p_clk_map, sizeof(uint64_t*)*2);
		RET_IF_NONZERO(ret);

		/* clock offset map */
		sz = MAX_CLK_OFF_MAP*8;
		ret = recv(sock, clk_off_map, sz, MSG_WAITALL);
		if (ret != sz)
			return -E_SYSCALL;
		/* tell the ebd_sort the address of the clk map */
		p_clk_off_map[0] = clk_off_map;
		p_clk_off_map[1] = reinterpret_cast<uint64_t*>(total_thread);
		ret = send_msg(EBD_SORT, 4, p_clk_off_map, sizeof(uint64_t*)*2);
		RET_IF_NONZERO(ret);
	}

	/* Now propagate the start request to the next receiver thread (if any)
	 * */
	if (!is_last_thread())
		return send_msg(next_thread(), 1, &acq_stat, 4);
	else
		return 0;
}

bool ebd_recv::is_last_thread()
{
	if (thread_id == ((total_thread-1)*10 + 1))
		return true;
	else
		return false;
}

int ebd_recv::next_thread()
{
	if (is_last_thread())
		return 0;
	return thread_id + 10;
}

int ebd_recv::init_rb_data()
{
	int ret, n, i, slot, crate;
	ring_buf* p_rb;

	ret = recv(sock, &n, 4, MSG_WAITALL);
	if (ret != 4)
		return -E_SYSCALL;
	/* debug ...*/
//	std::cout<<n<<std::endl;
	for (i = 0; i < n; i++) {
		ret = recv(sock, &slot, 4, MSG_WAITALL);
		if (ret != 4)
			return -E_SYSCALL;
		ret = recv(sock, &crate, 4, MSG_WAITALL);
		if (ret != 4)
			return -E_SYSCALL;
		p_rb = new ring_buf;
		if (p_rb->init(rb_data_sz)) {
			delete p_rb;
			p_rb = NULL;
		}
		if (p_rb) {
			 char* p_data = p_rb->get_usr_data();
			 uint32_t *p_int32 = reinterpret_cast<uint32_t*>(p_data);
			 p_data[0] = crate;
			 p_data[1] = slot;
			 p_data[2] = 0; /* a marker used by ebd_merge
                                               (can build or not) */
			 p_int32[1] = 0; /* the cur_rd number */

		}
		rb_data.push_back(p_rb);
	}
	/* debug ...*/
//	std::cout<<"printed from receiver  "<<rb_data.size()<<std::endl;
//	std::cout<<rb_data[0]<<std::endl;
	return 0;
}

int ebd_recv::stop()
{
	int ret, next_th_id;

	if (thread_id != 1) {
		if (++n_stops < 2)
			return 0;
	}
	
	ret = recv_stop();
	RET_IF_NONZERO(ret);
	acq_stat = DAQ_STOP;

	/* proporgate the stop message to the next thread */
	if (is_last_thread())
		next_th_id = 2;
	else 
		next_th_id = next_thread();
	return send_msg(next_th_id, 1, &acq_stat, 4);
}

int ebd_recv::quit()
{
	int ret = recv_quit();
	RET_IF_NONZERO(ret);

	acq_stat = DAQ_EXIT;

	/* propagate the quit message to the next thread if any */
	if (!is_last_thread())
		return send_msg(next_thread(), 1, &acq_stat, 4);
	else
		return 0;
}

int ebd_recv::main_proc()
{
	int sz, ret;
	if (has_sock_data()) {
		ret = recv(sock, &sz, 4, MSG_WAITALL);
		if (ret != 4)
			return -E_SYSCALL;
		if (sz == 0) {
			/* the frontend sends a 'stop' signal, we should stop */
			int status = DAQ_STOP;
			return send_msg(thread_id, 1, &status, 4);
		}

		/* we expect a following data packets with total length of sz */
		return read_sock_data(rb_fe[thread_id/10], sz);
	}

	/* no data available from socket */
	return 0;
}
