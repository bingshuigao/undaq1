.PHONY: all
all: frontend event_builder logger analyzer

frontend: frontend.cpp fe_ctl.cpp fe_sender.cpp rd_trig.cpp rd_scal.cpp rd_fe.cpp fe_thread.cpp daq_thread.cpp xml_parser.cpp initzer.cpp BigInteger.cpp v2718.cpp madc32.cpp modules.cpp  v1190.cpp ring_buf.cpp ctl_thread.cpp v830.cpp my_tcp_svr.cpp my_tcp_clt.cpp fe_ctl.h fe_sender.h rd_trig.h rd_scal.h rd_fe.h fe_thread.h daq_thread.h xml_parser.h initzer.h BigInteger.h v2718.h madc32.h modules.h  v1190.h ring_buf.h ctl_thread.h v830.h my_tcp_svr.h my_tcp_clt.h
	g++ --std=c++11 -o frontend `root-config --cflags --libs` frontend.cpp fe_ctl.cpp fe_sender.cpp rd_trig.cpp rd_scal.cpp rd_fe.cpp fe_thread.cpp daq_thread.cpp xml_parser.cpp initzer.cpp BigInteger.cpp v2718.cpp madc32.cpp modules.cpp  v1190.cpp ring_buf.cpp ctl_thread.cpp v830.cpp my_tcp_svr.cpp my_tcp_clt.cpp -lCAENVME

event_builder: event_builder.cpp ebd_sender.cpp ebd_recv.cpp ebd_merge.cpp ebd_sort.cpp ebd_ctl.cpp ebd_thread.cpp recv_thread.cpp daq_thread.cpp xml_parser.cpp initzer.cpp BigInteger.cpp v2718.cpp madc32.cpp modules.cpp  v1190.cpp ring_buf.cpp ctl_thread.cpp v830.cpp my_tcp_svr.cpp my_tcp_clt.cpp ebd_sender.h ebd_recv.h ebd_merge.h ebd_sort.h ebd_ctl.h ebd_thread.h recv_thread.h daq_thread.h xml_parser.h initzer.h BigInteger.h v2718.h madc32.h modules.h  v1190.h ring_buf.h ctl_thread.h v830.h my_tcp_svr.h my_tcp_clt.h 

	g++ --std=c++11 -o event_builder `root-config --cflags --libs` event_builder.cpp ebd_sender.cpp ebd_recv.cpp ebd_merge.cpp ebd_sort.cpp ebd_ctl.cpp ebd_thread.cpp recv_thread.cpp daq_thread.cpp xml_parser.cpp initzer.cpp BigInteger.cpp v2718.cpp madc32.cpp modules.cpp  v1190.cpp ring_buf.cpp ctl_thread.cpp v830.cpp my_tcp_svr.cpp my_tcp_clt.cpp -lCAENVME

logger: logger.cpp log_save.cpp log_recv.cpp   log_ctl.cpp log_thread.cpp recv_thread.cpp daq_thread.cpp xml_parser.cpp initzer.cpp BigInteger.cpp v2718.cpp madc32.cpp modules.cpp  v1190.cpp ring_buf.cpp ctl_thread.cpp v830.cpp my_tcp_svr.cpp my_tcp_clt.cpp log_save.h log_recv.h   log_ctl.h log_thread.h recv_thread.h daq_thread.h xml_parser.h initzer.h BigInteger.h v2718.h madc32.h modules.h  v1190.h ring_buf.h ctl_thread.h v830.h my_tcp_svr.h my_tcp_clt.h
	g++ --std=c++11 -o logger `root-config --cflags --libs` logger.cpp log_save.cpp log_recv.cpp   log_ctl.cpp log_thread.cpp recv_thread.cpp daq_thread.cpp xml_parser.cpp initzer.cpp BigInteger.cpp v2718.cpp madc32.cpp modules.cpp  v1190.cpp ring_buf.cpp ctl_thread.cpp v830.cpp my_tcp_svr.cpp my_tcp_clt.cpp -lCAENVME

analyzer: analyzer.cpp ana_recv.cpp ana_main.cpp ana_ctl.cpp ana_roody_svr.cpp ana_usr.cpp ana_thread.cpp recv_thread.cpp daq_thread.cpp xml_parser.cpp initzer.cpp BigInteger.cpp v2718.cpp madc32.cpp modules.cpp  v1190.cpp ring_buf.cpp ctl_thread.cpp v830.cpp my_tcp_svr.cpp my_tcp_clt.cpp ana_recv.h ana_main.h ana_ctl.h ana_roody_svr.h ana_thread.h recv_thread.h daq_thread.h xml_parser.h initzer.h BigInteger.h v2718.h madc32.h modules.h  v1190.h ring_buf.h ctl_thread.h v830.h my_tcp_svr.h my_tcp_clt.h
	g++ --std=c++11 -o analyzer `root-config --cflags --libs` analyzer.cpp ana_recv.cpp ana_main.cpp ana_ctl.cpp ana_roody_svr.cpp ana_usr.cpp ana_thread.cpp recv_thread.cpp daq_thread.cpp xml_parser.cpp initzer.cpp BigInteger.cpp v2718.cpp madc32.cpp modules.cpp  v1190.cpp ring_buf.cpp ctl_thread.cpp v830.cpp my_tcp_svr.cpp my_tcp_clt.cpp -lCAENVME

.PHONY: clean
clean:
	rm -f logger analyzer frontend event_builder
