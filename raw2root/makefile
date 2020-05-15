all: raw2root scaler

raw2root:raw2root.cpp
	g++ `root-config --libs --cflags` -o raw2root raw2root.cpp ana_evt_hd.cpp ana_frag_hd.cpp ana_madc32.cpp
scaler:scaler.cpp
	gcc -o scaler scaler.cpp
