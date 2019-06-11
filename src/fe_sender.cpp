#include "fe_sender.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>


fe_sender::fe_sender()
{
	thread_id = 3;
	svr = new my_tcp_svr;
	sock = -1;
	itv = 0;
	timer_itv.reset();
	sock_buf = NULL;
	init_fun.push_back(&fe_sender_init);
}

fe_sender::~fe_sender()
{
	delete svr;
	if (sock_buf)
		delete sock_buf;
}

int fe_sender::fe_sender_init(my_thread* ptr, initzer* the_initzer)
{
	fe_sender* This = static_cast<fe_sender*>(ptr);
	int port = the_initzer->get_fe_sender_port();
	if (This->svr->init(port) == -1)
		return -E_SYSCALL;

	This->itv = the_initzer->get_fe_sender_itv();
	This->sock_buf_sz = the_initzer->get_fe_sender_buf_sz();
	This->sock_buf = new unsigned char[This->sock_buf_sz];
	This->slot_map = the_initzer->get_slot_map();
	This->clk_map = the_initzer->get_clk_map();

	return 0;
}

int fe_sender::handle_msg(uint32_t* msg_body)
{
         /* The message type of the current thread are defined as following:
          * msg_type == 1 --> run status transition.                        
          * msg_type == 2 --> to be defined yet */                          

	uint32_t msg_type = msg_body[0] & 0xFFFFFF;

	switch (msg_type) {
	case 1:
		/* run status transition */
		return switch_run(msg_body[1]);
	default:
		return -E_MSG_TYPE;
	}
}

int fe_sender::start()
{
	int ret;
	acq_stat = 1;

	/* establish the connection between event builder (tcp client) and the
	 * current thread(tcp server). */
	if (sock == -1) {
		sock = svr->accept();
		svr->destroy();
		if (sock == -1)
			return -E_SYSCALL;
		/* the first thing after connection established is to send the
		 * ring buffer parameters and slot map and others */
		ret = send_mod_rb_par();
		RET_IF_NONZERO(ret);
		ret = send_slot_map();
		RET_IF_NONZERO(ret);
		ret = send_clk_map();
		RET_IF_NONZERO(ret);
	}

	/* proporgate the start message to thread 2*/
	return send_msg(2, 1, &acq_stat, 4);
}

int fe_sender::send_mod_rb_par()
{
	int ret;
	int n, slot, crate;
	
	/* first, send the number of ring buffers */
	n = rbs_ebd.size();
	ret = do_send(sock, &n, 4, 0);
	RET_IF_NONZERO(ret);

	/* then send the parameters one by one */
	for (auto it = rbs_ebd.begin(); it != rbs_ebd.end(); it++) {
		slot = (*it).slot;
		crate = (*it).crate;
		ret = do_send(sock, &slot, 4, 0);
		RET_IF_NONZERO(ret);
		ret = do_send(sock, &crate, 4, 0);
		RET_IF_NONZERO(ret);
	}
	return 0;
}

int fe_sender::send_slot_map()
{
	return do_send(sock, slot_map, MAX_SLOT_MAP, 0);
}
int fe_sender::send_clk_map()
{
	return do_send(sock, clk_map, MAX_CLK_MAP*8, 0);
}

int fe_sender::stop()
{
	int ret;
	int n = 0;

	acq_stat = 0;
	ret = flush_buf();
	RET_IF_NONZERO(ret);

	/* Now we also need to send the client a zero length to indicate the
	 * stop event. */
	if (send(sock, &n, 4, 0) == -1)
		return -E_SYSCALL;

	return send_msg(4, 1, &acq_stat, 4);
}

int fe_sender::quit()
{
	acq_stat = 2;
	close(sock);
	sock = -1;
	return 0;
}

int fe_sender::main_proc()
{
	/* check the data ring buffer and send the data (if any) to client. The
	 * data are not event-aligned, meaning the events may be split into two
	 * buffers. The client should take care of this. */
	while (true) {
		int32_t sz_in, ret;

		if (timer_itv.time_out(itv)) {
			timer_itv.reset();
			break;
		}

		if (rb_data->get_lock())
			return -E_SYSCALL;
		sz_in = rb_data->get_used1();
		if (sz_in == 0) {
			/* no data available */
			if (rb_data->rel_lock())
				return -E_SYSCALL;
			usleep(120);
			continue;
		}

		/* Now we have some data, let's send them out (remember that
		 * the lock is released in the send_data() function */
		ret = send_data();
		RET_IF_NONZERO(ret);
	}

	return 0;
}

int fe_sender::send_data()
{
	int sz_out, ret;
	
	/* first, send the size */
	sz_out = rb_data->read1(sock_buf, sock_buf_sz);
	ret = rb_data->rel_lock();
	RET_IF_NONZERO(ret);
	if (sz_out == -1) 
		return -E_RING_BUF_DATA;
	ret = send(sock, &sz_out, 4, 0);
	if (ret == -1) 
		return -E_SYSCALL;
	/* then send the data */
	return do_send(sock, sock_buf, sz_out, 0);
}

int fe_sender::flush_buf()
{
	while (true) {
		int32_t sz_in, sz_out, ret;
		unsigned char* p_buf;

		if (rb_data->get_lock())
			return -E_SYSCALL;
		sz_in = rb_data->get_used1();
		if (sz_in == 0) {
			/* no data available */
			if (rb_data->rel_lock())
				return -E_SYSCALL;
			return 0;
		}

		/* Now we have some data, let's send them out (remember that
		 * the lock is released in the send_data() function */
		ret = send_data();
		RET_IF_NONZERO(ret);
	}

	return 0;
}
