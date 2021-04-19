CFLAGS = -g -O0 -DDEBUG___

.PHONY: all
#all: frontend event_builder logger analyzer 
all: frontend event_builder logger analyzer raw2root scaler2root

frontend: frontend.cpp fe_ctl.cpp fe_sender.cpp rd_trig.cpp rd_scal.cpp rd_fe.cpp fe_thread.cpp daq_thread.cpp xml_parser.cpp initzer.cpp BigInteger.cpp v2718.cpp test_ctl.cpp madc32.cpp mqdc32.cpp v1740.cpp v1751.cpp v775.cpp v775n.cpp v785.cpp v785n.cpp fake_module.cpp modules.cpp  v1190.cpp ring_buf.cpp ctl_thread.cpp v830.cpp my_tcp_svr.cpp my_tcp_clt.cpp fe_ctl.h fe_sender.h rd_trig.h rd_scal.h rd_fe.h fe_thread.h daq_thread.h xml_parser.h initzer.h BigInteger.h v2718.h test_ctl.h madc32.h mqdc32.h v1740.h v1751.h v785.h v785n.h v775.h v775n.h modules.h  v1190.h ring_buf.h ctl_thread.h v830.h my_tcp_svr.h my_tcp_clt.h fake_module.h
	g++ ${CFLAGS} --std=c++11 -DMAKE_FRONTEND -o frontend frontend.cpp fe_ctl.cpp fe_sender.cpp rd_trig.cpp rd_scal.cpp rd_fe.cpp fe_thread.cpp daq_thread.cpp xml_parser.cpp initzer.cpp BigInteger.cpp v2718.cpp test_ctl.cpp madc32.cpp mqdc32.cpp v1740.cpp v1751.cpp v785.cpp v785n.cpp v775.cpp v775n.cpp modules.cpp  v1190.cpp ring_buf.cpp ctl_thread.cpp v830.cpp fake_module.cpp my_tcp_svr.cpp my_tcp_clt.cpp -lCAENVME -lpthread

event_builder: event_builder.cpp ebd_sender.cpp ebd_recv.cpp ebd_merge.cpp ebd_sort.cpp ebd_ctl.cpp ebd_thread.cpp recv_thread.cpp daq_thread.cpp xml_parser.cpp initzer.cpp BigInteger.cpp ring_buf.cpp ctl_thread.cpp my_tcp_svr.cpp my_tcp_clt.cpp ebd_sender.h ebd_recv.h ebd_merge.h ebd_sort.h ebd_ctl.h ebd_thread.h recv_thread.h daq_thread.h xml_parser.h initzer.h BigInteger.h ring_buf.h ctl_thread.h my_tcp_svr.h my_tcp_clt.h 
	g++ ${CFLAGS} --std=c++11 -DMAKE_EVENT_BUILDER -o event_builder  event_builder.cpp ebd_sender.cpp ebd_recv.cpp ebd_merge.cpp ebd_sort.cpp ebd_ctl.cpp ebd_thread.cpp recv_thread.cpp daq_thread.cpp xml_parser.cpp initzer.cpp BigInteger.cpp ring_buf.cpp ctl_thread.cpp my_tcp_svr.cpp my_tcp_clt.cpp -lpthread

logger: logger.cpp log_save.cpp log_recv.cpp log_ctl.cpp log_thread.cpp recv_thread.cpp daq_thread.cpp xml_parser.cpp initzer.cpp BigInteger.cpp ring_buf.cpp ctl_thread.cpp my_tcp_svr.cpp my_tcp_clt.cpp log_save.h log_recv.h log_ctl.h log_thread.h recv_thread.h daq_thread.h xml_parser.h initzer.h BigInteger.h ring_buf.h ctl_thread.h my_tcp_svr.h my_tcp_clt.h
	g++ ${CFLAGS} --std=c++11 -DMAKE_LOGGER -o logger  logger.cpp log_save.cpp log_recv.cpp   log_ctl.cpp log_thread.cpp recv_thread.cpp daq_thread.cpp xml_parser.cpp initzer.cpp BigInteger.cpp ring_buf.cpp ctl_thread.cpp my_tcp_svr.cpp my_tcp_clt.cpp -lpthread

analyzer: analyzer.cpp ana_recv.cpp ana_main.cpp ana_ctl.cpp ana_roody_svr.cpp ana_usr.cpp ana_thread.cpp recv_thread.cpp daq_thread.cpp xml_parser.cpp initzer.cpp BigInteger.cpp ring_buf.cpp ctl_thread.cpp my_tcp_svr.cpp my_tcp_clt.cpp ana_evt_hd.cpp ana_frag_hd.h ana_v830.cpp ana_madc32.cpp ana_mqdc32.cpp ana_v775.cpp ana_v785.cpp ana_v1190.cpp ana_v1740.cpp ana_v1751.cpp ana_recv.h ana_main.h ana_ctl.h ana_roody_svr.h ana_thread.h recv_thread.h daq_thread.h xml_parser.h initzer.h BigInteger.h ring_buf.h ctl_thread.h my_tcp_svr.h my_tcp_clt.h ana_evt_hd.h ana_frag_hd.h ana_madc32.h ana_mqdc32.h ana_v830.h ana_v775.h ana_v785.h ana_v1190.h ana_v1740.h ana_v1751.h
	g++ ${CFLAGS} --std=c++11 -DMAKE_ANALYZER -o analyzer `root-config --cflags --libs` analyzer.cpp ana_recv.cpp ana_main.cpp ana_ctl.cpp ana_roody_svr.cpp ana_usr.cpp ana_thread.cpp recv_thread.cpp daq_thread.cpp xml_parser.cpp initzer.cpp BigInteger.cpp ring_buf.cpp ctl_thread.cpp my_tcp_svr.cpp my_tcp_clt.cpp ana_evt_hd.cpp ana_frag_hd.cpp ana_madc32.cpp ana_mqdc32.cpp ana_v830.cpp ana_v775.cpp ana_v785.cpp ana_v1190.cpp ana_v1740.cpp ana_v1751.cpp

raw2root: raw2root.cpp ana_evt_hd.cpp ana_frag_hd.cpp ana_v830.cpp ana_madc32.cpp ana_mqdc32.cpp ana_v775.cpp ana_v785.cpp ana_v1190.cpp ana_v1740.cpp ana_v1751.cpp xml_parser.cpp  ana_evt_hd.h ana_frag_hd.h ana_madc32.h ana_mqdc32.h ana_v830.h ana_v1190.h ana_v1740.h ana_v1751.h xml_parser.h ana_v775.h ana_v785.h
	g++ --std=c++11 `root-config --libs --cflags` -o raw2root raw2root.cpp ana_evt_hd.cpp ana_frag_hd.cpp ana_v830.cpp ana_madc32.cpp ana_mqdc32.cpp ana_v1190.cpp ana_v1740.cpp ana_v1751.cpp xml_parser.cpp ana_v775.cpp ana_v785.cpp

scaler2root: scaler2root.cpp
	g++ --std=c++11 `root-config --libs --cflags` -o scaler2root scaler2root.cpp

.PHONY: clean
clean:
	rm -f logger analyzer frontend event_builder
