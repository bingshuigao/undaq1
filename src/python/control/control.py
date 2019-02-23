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
        self.svr_fe = svr()
        self.svr_ebd = svr()
        self.svr_ana = svr()
        self.svr_log = svr()

        # The number of connected clients
        self.n_client = 0

    def _create_logger_info(self):
        # the run number
        tk.Label(self.root_win, text='run number: ', bg='#00ffff')\
                .place(x=0, y=600, width=80, height=25)
        self.run_num_entry = tk.Entry(self.root_win)
        self.run_num_entry.place(x=81, y=600, width=49, height=25)
        # the if save flag
        self.save_flag_var = tk.IntVar()
        tk.Checkbutton(self.root_win, text='Save', bg='#f0f0f0', 
                variable=self.save_flag_var).place(x=135, y=600, 
                        width=60, height=25)


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
            tmp = 9999
        self.port = tmp
        # the update interval
        tmp = parser.get_par('t_ms')
        if not tmp:
            tmp = 1000
        self.t_ms = tmp

    def _check_listen_sock(self):
        # debug:
        #print('checking...')
        if self.n_client < 4:
            conn,name = self.listen_svr.try_accept()
        else:
            return
        if not conn:
            return
        if not name:
            print('unknown client!')
            sys.exit(-1)
        if name == 'frontend':
            self.svr_fe.set_sock(conn)
        elif name == 'event builder':
            self.svr_ebd.set_sock(conn)
        elif name == 'analyzer':
            self.svr_ana.set_sock(conn)
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
        msg = self.svr_fe.try_recv()
        self._handle_fe_msg(msg)
        # event builder
        msg = self.svr_ebd.try_recv()
        self._handle_ebd_msg(msg)
        # logger
        msg = self.svr_log.try_recv()
        self._handle_log_msg(msg)
        # analyzer
        msg = self.svr_ana.try_recv()
        self._handle_ana_msg(msg)

    def _handle_fe_msg(self, msg):
        if not msg:
            return
        msg_type = int.from_bytes(msg[:4], 'little') 
        if msg_type == 2: 
            # run status update
            run_stat = int.from_bytes(msg[4:8], 'little')
            if run_stat == 0:
                self.fe_stat_var.set('stop')
            elif run_stat == 1:
                self.fe_stat_var.set('run')


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

    def _check_stat(self):
        # send a query status message to the clients
        msg_type = (2).to_bytes(length=4, byteorder='little')
        msg_tail = bytes([0 for i in range(124)])
        msg = msg_type + msg_tail
        # send the message to all
        self.svr_fe.send_all(msg)
        self.svr_ebd.send_all(msg)
        self.svr_ana.send_all(msg)
        self.svr_log.send_all(msg)

    def _clock(self):
        # we check the listening socket periodically
        self._check_listen_sock()
        # we check the connection sockets too
        self._check_conn_sock()
        # check the run status
        self._check_stat()
        # setup the clock again
        self.root_win.after(self.t_ms, self._clock)

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
        self.svr_fe.send_all(msg)
        self.svr_ebd.send_all(msg)
        self.svr_ana.send_all(msg)
        # modify the message and send it to the logger
        run_num = int(self.run_num_entry.get()).to_bytes(length=4,
                byteorder='little')
        if_save = self.save_flag_var.get().to_bytes(length=4,
                byteorder='little')
        msg_tail = bytes([0 for i in range(112)])
        msg = msg_type + run_stat + run_num + if_save
        self.svr_log.send_all(msg)
        


    def _stop(self):
        # To stop the DAQ, we need only to send message to frontend
        msg_type = (0).to_bytes(length=4, byteorder='little')
        run_stat = (0).to_bytes(length=4, byteorder='little')
        msg_tail = bytes([0 for i in range(120)])
        msg = msg_type + run_stat + msg_tail
        # send the start message to frontend, event builder and analyzer
        self.svr_fe.send_all(msg)

    def _quit(self):
        msg_type = (0).to_bytes(length=4, byteorder='little')
        run_stat = (2).to_bytes(length=4, byteorder='little')
        msg_tail = bytes([0 for i in range(120)])
        msg = msg_type + run_stat + msg_tail
        # send the message to all
        self.svr_fe.send_all(msg)
        self.svr_ebd.send_all(msg)
        self.svr_ana.send_all(msg)
        self.svr_log.send_all(msg)


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

app = control('c.xml')
app.run()
