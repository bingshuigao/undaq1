#include "ebd_sender.h"

ebd_sender::ebd_sender()
{
	thread_id = 4;
	svr = new my_tcp_svr;
	sock_log = -1;
	sock_ana = -1;
	sock_buf = NULL;
	init_fun.push_back(&ebd_sender_init);
}

ebd_sender::~ebd_sender()
{
	delete svr;
	if (sock_buf)
		delete sock_buf;
}

int ebd_sender::ebd_sender_init(my_thread* ptr, initzer* the_initzer)
{
	ebd_sender* This = static_cast<ebd_sender*>(ptr);
	int port = the_initzer->get_ebd_sender_port();
	if (This->svr->init(port) == -1)
		return -E_SYSCALL;

	This->sock_buf_sz = the_initzer->get_ebd_sender_buf_sz();
	This->sock_buf = new unsigned char[This->sock_buf_sz];
	return 0;
}


int ebd_sender::handle_msg(uint32_t* msg_body)
{
	/* The message type of the current thread are defined as following 
	 * 1 --> run status transition
	 * 2 --> to be defined
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


int ebd_sender::start()
{
	acq_stat = DAQ_RUN;

	/* establish the connection between logger and analyzer (tcp
	 * client) and the current thread(tcp server). */
	if (sock_ana == -1) {
		sock_log = svr->accept();
		sock_ana = svr->accept();
		svr->destroy();
		if ((sock_log == -1) || (sock_ana == -1))
			return -E_SYSCALL;
	}

	/* proporgate the message to the next thread */
	return send_msg(3, 1, &acq_stat, 4);
	
}

int ebd_sender::stop()
{
	int ret;
	int n = 0;

	acq_stat = 0;
	ret = flush_buf();
	RET_IF_NONZERO(ret);

	/* Now we also need to send the client a zero length to indicate the
	 * stop event. */
	if (send(sock_log, &n, 4, 0) == -1)
		return -E_SYSCALL;
	if (send(sock_ana, &n, 4, 0) == -1)
		return -E_SYSCALL;

	/* proporgate the stop message to the next thread */
	return send_msg(5, 1, &acq_stat, 4);
}

int ebd_sender::quit()
{
	acq_stat = DAQ_EXIT;
	close(sock_ana);
	close(sock_log);
	return 0;
}

int ebd_sender::main_proc()
{
	/* check the data ring buffer and send the data (if any) to client. The
	 * data are not event-aligned, meaning the events may be split into two
	 * buffers. The client should take care of this. */
	int32_t sz_in, ret;

	/* check the scaler */
	if (rb_scal->get_lock())
		return -E_SYSCALL;
	sz_in = rb_scal->get_used1();
	if (sz_in == 0) {
		/* no data available */
		rb_scal->rel_lock();
	}
	else {
		ret = send_data(rb_scal);
		RET_IF_NONZERO(ret);
	}

	/* check the trigger */
	if (rb_evt->get_lock())
		return -E_SYSCALL;
	sz_in = rb_evt->get_used1();
	if (sz_in == 0) {
		/* no data available */
		rb_evt->rel_lock();
		usleep(120);
		return 0;
	}
	else {
		/* Now we have some data, let's send them out (remember that
		 * the lock is released in the send_data() function */
		ret = send_data(rb_evt);
		RET_IF_NONZERO(ret);
	}

	return 0;

}


int ebd_sender::send_data(ring_buf* the_rb)
{
	int sz_out, ret;
	
	/* we need to let the receiver know if this is a scaler or a trigger
	 * data. So we first send a size, then a type, then the data. if type
	 * is 1, it is scaler data, if type is 2, it is trigger data.  */
	sz_out = the_rb->read1(sock_buf+1, sock_buf_sz);
	the_rb->rel_lock();
	if (sz_out == -1) 
		return -E_RING_BUF_DATA;

	if ((send(sock_log, &sz_out, 4, 0) == -1) || 
	    (send(sock_ana, &sz_out, 4, 0) == -1))
		return -E_SYSCALL;
	if (the_rb == rb_evt)
		sock_buf[0] = 2;
	else if (the_rb == rb_scal)
		sock_buf[0] = 1;

	/* then send the type and data */
	sz_out++;
	ret = do_send(sock_log, sock_buf, sz_out, 0);
	RET_IF_NONZERO(ret);
	ret = do_send(sock_ana, sock_buf, sz_out, 0);
	RET_IF_NONZERO(ret);

	return 0;
}

int ebd_sender::flush_buf()
{
	bool flag1, flag2;

	flag1 = true;
	flag2 = true;
	while (flag1 || flag2) {
		int32_t sz_in, sz_out, ret;
		unsigned char* p_buf;

		/* flush the scaler data */
		if (rb_scal->get_lock())
			return -E_SYSCALL;
		sz_in = rb_scal->get_used1();
		if (sz_in == 0) {
			/* no data available */
			if (rb_scal->rel_lock())
				return -E_SYSCALL;
			flag1 = false;
		}
		else {
			/* Now we have some data, let's send them out (remember
			 * that the lock is released in the send_data()
			 * function */
			ret = send_data(rb_scal);
			RET_IF_NONZERO(ret);
		}

		/* flush the trigger data */
		if (rb_evt->get_lock())
			return -E_SYSCALL;
		sz_in = rb_evt->get_used1();
		if (sz_in == 0) {
			/* no data available */
			if (rb_evt->rel_lock())
				return -E_SYSCALL;
			flag2 = false;
		}
		else {
			/* Now we have some data, let's send them out (remember
			 * that the lock is released in the send_data()
			 * function */
			ret = send_data(rb_evt);
			RET_IF_NONZERO(ret);
		}
	}

	return 0;
}
