#!/usr/bin/python3
# -*- coding: UTF-8 -*-
__metaclass__ = type

# This script provides a GUI controler of the whole DAQ.  The GUI is based on
# Tkinter package which is a wrapper of Tcl/Tk.
# Because the script need to communicate with many clients besides the
# 'mainloop', multithread should be needed. However, since multithreading is
# not trivail in python, we use the following simpler solution: 
#    We setup a timer using tk.self.listen_svr = listen_svr()
#    (in non blocking mode) in the timer events. See
#    https://stackoverflow.com/questions/2400262/how-to-create-a-timer-using-tkinter
#    for more details about timer in tk.
#
# To make the code easier, four classes are created, one for each server
# (frontend, event builder, logger, analyzer). The four classes derive from one
# common base class. 
#######
# By B.Gao Feb. 2019


import tkinter as tk
from tkinter import messagebox
from tkinter import ttk
from xml_parser import xml_parser
from listen_svr import listen_svr
from stat_tab import stat_tab
from rb_tab import rb_tab
from log_tab import log_tab
from svr import svr
import sys


class control:
    def __init__(self, f_name):
        # first, we get all config parameters
        self._get_pars(f_name)

        # root window
        self.root_win = tk.Tk()
        self.root_win.geometry('800x650')

        # start, stop and quit buttons
        self._create_buttons()

        # status box
        self._create_status_lable()

        # run number and if save button
        self._create_logger_info()

        # create the notebook
        self._create_notebook()

        # The liestening sockets
        self.listen_svr = listen_svr(self.port)

        # The connection sockets
        self.svr_fe = []
        for i in range(self.fe_num):
            self.svr_fe.append(svr())
        # debug ...
        print('num of fe: %d' % self.fe_num)
        ############
        self.svr_ebd = svr()
        self.svr_ana = svr()
        self.svr_log = svr()

        # The number of connected clients
        self.n_client = 0
        self.n_conn_fe = 0

        # The number of vme modules
        self.n_mod = -1

        # frontend(s) status
        self.fe_stat_lst = []
        for i in range(self.fe_num):
            self.fe_stat_lst.append('unknown')

        self.n_intv = 0
        self.ctrl_stat = 'stop'
        self.cur_time = [0 for i in range(self.fe_num)]
        self.cur_size = [0 for i in range(self.fe_num)]

    def _create_logger_info(self):
        # the run number
        tk.Label(self.root_win, text='run number: ', bg='#00ffff')\
                .place(x=0, y=600, width=80, height=25)
        self.run_num_entry = tk.Entry(self.root_win)
        self.run_num_entry.place(x=81, y=600, width=49, height=25)
        self.run_num_entry.insert(tk.END, 1)
        # the if save flag
        self.save_flag_var = tk.IntVar()
        tk.Checkbutton(self.root_win, text='Save', bg='#f0f0f0', 
                variable=self.save_flag_var).place(x=135, y=600, 
                        width=60, height=25)
        self.save_flag_var.set(1)
        # the title of this run
        self.run_title_entry = tk.Entry(self.root_win)
        self.run_title_entry.place(x=200, y=600, width=550, height=25)
        self.run_title_entry.insert(tk.END, 
                'set run title here (less then 100 letters)')


    def _create_status_lable(self):
        # frontend
        tk.Label(self.root_win, text='fe: ', bg='#00ffff')\
                .place(x=200, y=0, width=30, height=25)
        self.fe_stat_var = tk.StringVar()
        self.fe_stat = tk.Label(self.root_win,  bg='#00ff00',
                textvariable=self.fe_stat_var)
        self.fe_stat.place(x=231, y=0, width=79, height=25)
        # event builder
        tk.Label(self.root_win, text='ebd: ', bg='#00ffff')\
                .place(x=320, y=0, width=30, height=25)
        self.ebd_stat_var = tk.StringVar()
        self.ebd_stat = tk.Label(self.root_win,  bg='#00ff00',
                textvariable=self.ebd_stat_var)
        self.ebd_stat.place(x=351, y=0, width=79, height=25)
        # logger
        tk.Label(self.root_win, text='log: ', bg='#00ffff')\
                .place(x=440, y=0, width=30, height=25)
        self.log_stat_var = tk.StringVar()
        self.log_stat = tk.Label(self.root_win,  bg='#00ff00',
                textvariable=self.log_stat_var)
        self.log_stat.place(x=471, y=0, width=79, height=25)
        # analyzer
        tk.Label(self.root_win, text='ana: ', bg='#00ffff')\
                .place(x=560, y=0, width=30, height=25)
        self.ana_stat_var = tk.StringVar()
        self.ana_stat = tk.Label(self.root_win,  bg='#00ff00',
                textvariable=self.ana_stat_var)
        self.ana_stat.place(x=591, y=0, width=79, height=25)


    def _get_pars(self, f_name):
        # get the config parameters from the config file
        parser = xml_parser(f_name)
        if not parser.OK:
            print('cannot parse the config file!')
            sys.exit(-1)
        # The listening port
        tmp = parser.get_par('port')
        if not tmp:
            tmp = 9090
        self.port = tmp
        # the update interval
        tmp = parser.get_par('t_ms')
        if not tmp:
            tmp = 1000
        self.t_ms = tmp
        # the number of fe clients
        tmp = parser.get_par('fe_num')
        if not tmp:
            tmp = 1
        self.fe_num = tmp

    def _check_listen_sock(self):
        # debug:
        #print('checking...')
        if self.n_client < (3 + self.fe_num):
            conn,name = self.listen_svr.try_accept()
        else:
            return
        if not conn:
            return
        if not name:
            print('unknown client!')
            sys.exit(-1)
        if name == 'frontend':
            self.svr_fe[self.n_conn_fe].set_sock(conn)
            self.n_conn_fe += 1
            # debug ...
            print('fe +1')
            ##########
        elif name == 'event builder':
            self.svr_ebd.set_sock(conn)
        elif name == 'analyzer':
            self.svr_ana.set_sock(conn)
            # if we are already in 'run' status, send a 'start' command to the
            # analyzer
            if self._check_all_stat() == 'run':
                msg_type = (0).to_bytes(length=4, byteorder='little')
                run_stat = (1).to_bytes(length=4, byteorder='little')
                msg_tail = bytes([0 for i in range(120)])
                msg = msg_type + run_stat + msg_tail
                self.svr_ana.send_all(msg)
        elif name == 'logger':
            self.svr_log.set_sock(conn)
        else:
            print('unknown client!')
            sys.exit(-1)
        self.n_client += 1

    def _check_conn_sock(self):
        # This is the main part
        # Try to receive pending message from the clients (if any)
        # frontend
        msg = 1
        for i in range(self.fe_num):
            while msg:
                msg = self.svr_fe[i].try_recv()
                self._handle_fe_msg(msg, i)
        self._update_fe_stat()
        # event builder
        msg = 1
        while msg:
            msg = self.svr_ebd.try_recv()
            self._handle_ebd_msg(msg)
        # logger
        msg = 1
        while msg:
            msg = self.svr_log.try_recv()
            self._handle_log_msg(msg)
        # analyzer
        msg = 1
        while msg:
            msg = self.svr_ana.try_recv()
            self._handle_ana_msg(msg)
            if not self.svr_ana.sock:
                # broken connection:
                self.n_client -= 1
                self.ana_stat_var.set('unkown')


    def _update_fe_stat(self):
        tmp = self.fe_stat_lst[0]
        # debug ...
        #print('fe status updated: %s' % tmp)
        ###############
        for i in range(self.fe_num):
            if self.fe_stat_lst[i] != tmp:
                tmp = 'unknown'
                break
        self.fe_stat_var.set(tmp)


    def _check_all_stat(self):
        fe_stat = self.fe_stat_var.get()
        ebd_stat= self.ebd_stat_var.get()
        log_stat= self.log_stat_var.get()
        ana_stat= self.ana_stat_var.get()
#        if self.ctrl_stat == fe_stat and self.ctrl_stat == ebd_stat and self.ctrl_stat == log_stat and self.ctrl_stat == ana_stat:
        if self.ctrl_stat == fe_stat and self.ctrl_stat == ebd_stat and self.ctrl_stat == log_stat:
            return self.ctrl_stat
        else:
            return 'inconsist'


    def _handle_fe_msg(self, msg, i):
        if not msg:
            return
        msg_type = int.from_bytes(msg[:4], 'little') 
        if msg_type == 2: 
            # run status update
            run_stat = int.from_bytes(msg[4:8], 'little')
            if run_stat == 0:
                self.fe_stat_lst[i] = 'stop'
            elif run_stat == 1:
                self.fe_stat_lst[i] = 'run'
        elif msg_type == 3:
            # data rate
            n_byte_hi = int.from_bytes(msg[4:8],   'little')
            n_byte_lo = int.from_bytes(msg[8:12],  'little')
            ts_hi     = int.from_bytes(msg[12:16], 'little')
            ts_lo     = int.from_bytes(msg[16:20], 'little')
            n_byte = (n_byte_hi<<32) + n_byte_lo
            delta_n = n_byte - self.cur_size[i]
            self.cur_size[i] = n_byte
            ts = (ts_hi<<32) + ts_lo
            delta_t = ts - self.cur_time[i]
            self.cur_time[i] = ts
            self.stat_tab.set_rate(1.*delta_n/delta_t)
            self.stat_tab.set_data_sz(n_byte/1000000.)
            #print('fe_num = %d, n_byte = %d, ts = %d, rate = %f (kB/s)' % (i, n_byte, ts, 1.*delta_n/delta_t))
        elif msg_type == 4:
            tot_sz = int.from_bytes(msg[4:8], 'little')
            use_sz = int.from_bytes(msg[8:12],'little')
            #print('frontend buffer: %d/%d' % (use_sz, tot_sz))
            self.rb_tab.draw_rb_fe(i, tot_sz, use_sz)
        elif msg_type == 5:
            # text messages from clients 
            self._handle_text_msg(msg, 'frontend'+str(i))

    def _handle_text_msg(self, msg, client):
            level = int.from_bytes(msg[4:8], 'little')
            # debug
            # print(msg[8:])
            txt_len = 0
            for b in msg[8:]:
                if b:
                    txt_len += 1
                else:
                    break;
            msg_body = msg[8:8+txt_len].decode('utf-8')
            self.log_tab.insert_log('(%s) %s' % (client, msg_body), not level)




    def _handle_ebd_msg(self, msg):
        if not msg:
            return
        msg_type = int.from_bytes(msg[:4], 'little') 
        if msg_type == 2: 
            # run status update
            run_stat = int.from_bytes(msg[4:8], 'little')
            if run_stat == 0:
                self.ebd_stat_var.set('stop')
            elif run_stat == 1:
                self.ebd_stat_var.set('run')
            # Now check the overall status and enable/disable the buttons
            # the 'n_intv' is to let the check status slower
            self.n_intv += 1
            if self.n_intv == 2:
                self.n_intv = 0
                status = self._check_all_stat()
                if (status == 'stop'):
                    self.butt_start.config(state=tk.NORMAL)
                    self.butt_stop.config(state=tk.DISABLED)
                    self.butt_quit.config(state=tk.NORMAL)
                elif (status == 'run'):
                    self.butt_start.config(state=tk.DISABLED)
                    self.butt_stop.config(state=tk.NORMAL)
                    self.butt_quit.config(state=tk.DISABLED)
                elif (status == 'inconsist'):
                    self.butt_start.config(state=tk.DISABLED)
                    self.butt_stop.config(state=tk.DISABLED)
                    self.butt_quit.config(state=tk.DISABLED)
        elif msg_type == 3:
            self.n_mod = int.from_bytes(msg[4:8], 'little')
        elif msg_type == 4:
            # the receivers ring buffers (the number is fe_num)
            ptr = 4
            for i in range(self.fe_num):
                sz_tot = int.from_bytes(msg[ptr:ptr+4], 'little')
                ptr += 4
                sz_use = int.from_bytes(msg[ptr:ptr+4], 'little')
                ptr += 4
            #    print('ebd buffer: %d/%d', (sz_use, sz_tot))
                self.rb_tab.draw_rb_ebd_recv(self.fe_num, i, sz_tot, sz_use)
            # the individual vme module buffers (the number is n_mod)
            for i in range(self.n_mod):
                crate_slot = int.from_bytes(msg[ptr:ptr+4], 'little')
                ptr += 4
                sz_tot = int.from_bytes(msg[ptr:ptr+4], 'little')
                ptr += 4
                sz_use = int.from_bytes(msg[ptr:ptr+4], 'little')
                ptr += 4
                #print('ebd buffer (crate: %d, slot %d): %d/%d',
                        #(crate_slot&0xff, (crate_slot>>8)&0xff, sz_use, sz_tot))
                self.rb_tab.draw_rb_ebd_sort(i, crate_slot, sz_tot, sz_use)
            # the scaler buffer
            sz_tot = int.from_bytes(msg[ptr:ptr+4], 'little')
            ptr += 4
            sz_use = int.from_bytes(msg[ptr:ptr+4], 'little')
            ptr += 4
#            print('scaler buffer: %d/%d', (sz_use, sz_tot))
            self.rb_tab.draw_rb_ebd_scal(self.n_mod, sz_tot, sz_use)
            # the built-event buffer
            sz_tot = int.from_bytes(msg[ptr:ptr+4], 'little')
            ptr += 4
            sz_use = int.from_bytes(msg[ptr:ptr+4], 'little')
            ptr += 4
#            print('built event buffer: %d/%d', (sz_use, sz_tot))
            self.rb_tab.draw_rb_ebd_merger(sz_tot, sz_use)
        elif msg_type == 5:
            # text messages from clients 
            self._handle_text_msg(msg, 'event builder')
        elif msg_type == 6:
            # broken pipe
#            print('broken pipe ...')
            msg_type = (5).to_bytes(length=4, byteorder='little')
            msg_tail = bytes([0 for i in range(124)])
            msg = msg_type + msg_tail
            # send the message to all
            for i in range(self.fe_num):
                self.svr_fe[i].send_all(msg)
        elif msg_type == 7:
            # the evt cnts
            self.stat_tab.update_evt_cnt(msg[4:])
        elif msg_type == 8:
            # the evt cnts
            self.stat_tab.update_evt_cnt1(msg[4:])


    def _handle_ana_msg(self, msg):
        if not msg:
            return
        msg_type = int.from_bytes(msg[:4], 'little') 
        if msg_type == 2: 
            # run status update
            run_stat = int.from_bytes(msg[4:8], 'little')
            if run_stat == 0:
                self.ana_stat_var.set('stop')
            elif run_stat == 1:
                self.ana_stat_var.set('run')
        elif msg_type == 4:
            sz_tot = int.from_bytes(msg[4:8], 'little')
            sz_use = int.from_bytes(msg[8:12],'little')
#            print('ana scaler buffer: %d/%d', (sz_use, sz_tot))
            self.rb_tab.draw_rb_ana_scal(sz_tot, sz_use)
            sz_tot = int.from_bytes(msg[12:16], 'little')
            sz_use = int.from_bytes(msg[16:20],'little')
#            print('ana trig buffer: %d/%d', (sz_use, sz_tot))
            self.rb_tab.draw_rb_ana_trig(sz_tot, sz_use)
        elif msg_type == 5:
            # text messages from clients 
            self._handle_text_msg(msg, 'analyzer')
        elif msg_type == 6:
            # scaler counters
            cnts = []
            for i in range(30):
                cnts.append(msg[4+i*4:8+i*4])
            self.stat_tab.update_scal_cnter(cnts)

    def _handle_log_msg(self, msg):
        if not msg:
            return
        msg_type = int.from_bytes(msg[:4], 'little') 
        if msg_type == 2: 
            # run status update
            run_stat = int.from_bytes(msg[4:8], 'little')
            if run_stat == 0:
                self.log_stat_var.set('stop')
            elif run_stat == 1:
                self.log_stat_var.set('run')
        elif msg_type == 4:
            sz_tot = int.from_bytes(msg[4:8], 'little')
            sz_use = int.from_bytes(msg[8:12],'little')
#            print('ana scaler buffer: %d/%d', (sz_use, sz_tot))
            self.rb_tab.draw_rb_log_scal(sz_tot, sz_use)
            sz_tot = int.from_bytes(msg[12:16], 'little')
            sz_use = int.from_bytes(msg[16:20],'little')
#            print('ana trig buffer: %d/%d', (sz_use, sz_tot))
            self.rb_tab.draw_rb_log_trig(sz_tot, sz_use)
        elif msg_type == 5:
            # text messages from clients 
            self._handle_text_msg(msg, 'logger')


    def _check_stat(self):
        # send a query status message to the clients
        msg_type = (2).to_bytes(length=4, byteorder='little')
        msg_tail = bytes([0 for i in range(124)])
        msg = msg_type + msg_tail
        # send the message to all
        for i in range(self.fe_num):
            self.svr_fe[i].send_all(msg)
        self.svr_ebd.send_all(msg)
        self.svr_ana.send_all(msg)
        self.svr_log.send_all(msg)

    def _check_rate(self):
        # send a query statistics message to the clients
        msg_type = (3).to_bytes(length=4, byteorder='little')
        msg_tail = bytes([0 for i in range(124)])
        msg = msg_type + msg_tail
        # send the message to all
        for i in range(self.fe_num):
            self.svr_fe[i].send_all(msg)
            print('checking rate......')
        # we also send a query to ebd
        msg_type = (5).to_bytes(length=4, byteorder='little')
        msg_tail = bytes([0 for i in range(124)])
        msg = msg_type + msg_tail
        self.svr_ebd.send_all(msg)


    def _clock(self):
        # we check the listening socket periodically
        self._check_listen_sock()
        # we check the connection sockets too
        self._check_conn_sock()
        # check the run status
        self._check_stat()
        if self.tab_ctrl.tab(self.tab_ctrl.select(), 'text').strip() == 'status':
            self._check_rate()
        # check the ring buffers
        self._check_rb()
        # setup the clock again
        self.root_win.after(self.t_ms, self._clock)

    def _check_rb(self):
        # we do the checking only if the daq is in run status
        if self._check_all_stat() != 'run':
            return
        # we do the checking only if the ring buffers tab is selected
        if self.tab_ctrl.tab(self.tab_ctrl.select(), 'text').strip() != 'ring buffers':
            return
        if self.n_mod == -1:
            # send a query n_mod message to the ebd client 
            msg_type = (3).to_bytes(length=4, byteorder='little')
            msg_tail = bytes([0 for i in range(124)])
            msg = msg_type + msg_tail
            # send the message to ebd
            self.svr_ebd.send_all(msg)
        else:
            # send a query rb status message to all the clients
            # the fe
            msg_type = (4).to_bytes(length=4, byteorder='little')
            msg_tail = bytes([0 for i in range(124)])
            msg = msg_type + msg_tail
            for i in range(self.fe_num):
                self.svr_fe[i].send_all(msg)
            # the ebd
            self.svr_ebd.send_all(msg)
            # the log
            self.svr_log.send_all(msg)
            # the ana
            self.svr_ana.send_all(msg)



    # create the open and save buttons:
    def _create_buttons(self):
        self.butt_start = tk.Button(self.root_win,
                text='start', command=self._start)
        self.butt_start.place(x=0, y=0, width=50, height=25)
        self.butt_stop = tk.Button(self.root_win,
                text='stop', command=self._stop)
        self.butt_stop.place(x=60, y=0, width=50, height=25)
        self.butt_quit = tk.Button(self.root_win,
                text='quit', command=self._quit)
        self.butt_quit.place(x=120, y=0, width=50, height=25)

    def _start(self):

        msg_type = (0).to_bytes(length=4, byteorder='little')
        run_stat = (1).to_bytes(length=4, byteorder='little')
        msg_tail = bytes([0 for i in range(120)])
        msg = msg_type + run_stat + msg_tail
        # send the start message to frontend, event builder and analyzer
        for i in range(self.fe_num):
            self.svr_fe[i].send_all(msg)
        self.svr_ebd.send_all(msg)
        self.svr_ana.send_all(msg)
        # modify the message and send it to the logger
        run_num = int(self.run_num_entry.get()).to_bytes(length=4,
                byteorder='little')
        if_save = self.save_flag_var.get().to_bytes(length=4,
                byteorder='little')
        run_title = self.run_title_entry.get()[:100].encode('utf-8') + b'\0\n'
        msg_tail = bytes([0 for i in range(112-len(run_title))])
        msg = msg_type + run_stat + run_num + if_save + run_title + msg_tail
        self.svr_log.send_all(msg)
        # disable the 'star', 'stop' and 'quit' buttons
        self.butt_start.config(state=tk.DISABLED)
        self.butt_stop.config(state=tk.DISABLED)
        self.butt_quit.config(state=tk.DISABLED)
        self.n_intv = 0
        self.ctrl_stat = 'run'
        


    def _stop(self):
        # To stop the DAQ, we need only to send message to frontend
        msg_type = (0).to_bytes(length=4, byteorder='little')
        run_stat = (0).to_bytes(length=4, byteorder='little')
        msg_tail = bytes([0 for i in range(120)])
        msg = msg_type + run_stat + msg_tail
        for i in range(self.fe_num):
            self.svr_fe[i].send_all(msg)
        # auto increment of the run number if 'if_save' is set:
        if self.save_flag_var.get() == 1:
            run_num = int(self.run_num_entry.get())
            run_num += 1
            self.run_num_entry.delete(0, tk.END)
            self.run_num_entry.insert(tk.END, str(run_num))
        # disable the 'star', 'stop' and 'quit' buttons
        self.butt_start.config(state=tk.DISABLED)
        self.butt_stop.config(state=tk.DISABLED)
        self.butt_quit.config(state=tk.DISABLED)
        self.n_intv = 0
        self.ctrl_stat = 'stop'



            

    def _quit(self):
        msg_type = (0).to_bytes(length=4, byteorder='little')
        run_stat = (2).to_bytes(length=4, byteorder='little')
        msg_tail = bytes([0 for i in range(120)])
        msg = msg_type + run_stat + msg_tail
        # send the message to all
        for i in range(self.fe_num):
            self.svr_fe[i].send_all(msg)
        self.svr_ebd.send_all(msg)
        self.svr_ana.send_all(msg)
        self.svr_log.send_all(msg)
        # quit the mainloop 
        self.root_win.quit()


    # create the notebook widget
    def _create_notebook(self):
        self.notebook_frm = tk.Frame(self.root_win)
        self.notebook_frm.place(x=0, y=30, width=800, height=570)
        self.tab_ctrl = ttk.Notebook(self.notebook_frm)

        # create the log tab
        self._create_log_tab(self.tab_ctrl)

        # create the status tab
        self._create_status_tab(self.tab_ctrl)
        
        # create the ring buffers tab
        self._create_rb_tab(self.tab_ctrl)

        # create more tabs here:
        # .............

    # create the log tab
    def _create_log_tab(self, tab_ctrl):
        self.log_tab = log_tab(tab_ctrl)
        tab_ctrl.add(self.log_tab.get_frm(), text='log       ')
        tab_ctrl.pack(fill=tk.BOTH, expand=1)

    # create the status tab
    def _create_status_tab(self, tab_ctrl):
        self.stat_tab = stat_tab(tab_ctrl)
        tab_ctrl.add(self.stat_tab.get_frm(), text='status   ')

    # create the ring buffers tab
    def _create_rb_tab(self, tab_ctrl):
        self.rb_tab = rb_tab(tab_ctrl)
        tab_ctrl.add(self.rb_tab.get_frm(), text='ring buffers')

    def run(self):
        self._clock()
        self.root_win.mainloop()

app = control('conf.xml')
app.run()
