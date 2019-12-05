#!/usr/bin/python3
# -*- coding: UTF-8 -*-
__metaclass__ = type


# this class is the advanced configuration for the frontend

import tkinter as tk
from adv_conf import adv_conf 

class adv_conf_fe(adv_conf):
    def __init__(self):
        self.var_lst = []

        # the data ring buffer size (in bytes)
        self.var_lst.append({
                 'value' : 'default',
                 'name' : 'ring_buf_sz',
                 'wid_type' : 'entry',
                 'comment' : None
                 })
        # the message ring buffer size (in bytes)
        self.var_lst.append({
                 'value' : 'default',
                 'name' : 'ring_buf2_sz',
                 'wid_type' : 'entry',
                 'comment' : None
                 })
        # number of try when checking trigger
        self.var_lst.append({
                 'value' : 'default',
                 'name' : 'n_try',
                 'wid_type' : 'entry',
                 'comment' : None
                 })
        # size of event buffer for each blt readout (in bytes)
        self.var_lst.append({
                 'value' : 'default',
                 'name' : 'blt_buf_sz',
                 'wid_type' : 'entry',
                 'comment' : None
                 })
        # the listening port of the frontend data sender 
        self.var_lst.append({
                 'value' : 'default',
                 'name' : 'sender_port',
                 'wid_type' : 'entry',
                 'comment' : None
                 })
        # The interval which the frontend data sender spend on checking the
        # ring buffer.
        self.var_lst.append({
                 'value' : 'default',
                 'name' : 'sender_itv',
                 'wid_type' : 'entry',
                 'comment' : None
                 })
        # The frontend socket buffer size (bytes)
        self.var_lst.append({
                 'value' : 'default',
                 'name' : 'sender_buf_sz',
                 'wid_type' : 'entry',
                 'comment' : None
                 })
        # the connection port of the frontend controller thread
        self.var_lst.append({
                 'value' : 'default',
                 'name' : 'ctl_port',
                 'wid_type' : 'entry',
                 'comment' : None
                 })
        # the socket buffer size (bytes) of the frontend controller thread
        self.var_lst.append({
                 'value' : 'default',
                 'name' : 'ctl_buf_sz',
                 'wid_type' : 'entry',
                 'comment' : None
                 })
        
        # the max number of seconds allowed to spend on on_start() functions
        self.var_lst.append({
                 'value' : 'default',
                 'name' : 'rd_start_t',
                 'wid_type' : 'entry',
                 'comment' : None
                 })
        

        # the server address of the frontend controller thread. The address can
        # be the form of 'www.host.com' or '127.0.0.1'. Since all the config
        # parameters are saved in the form of integers, these strings are
        # converted to integer in the following way:
        #     each charactor is first converted to its ascii value. Then it is
        #     shifted to the left according to its position in the string.
        #     Fianlly all these integer values are or'ed (or added) to form the
        #     final value. For example, the string '127.0' is converted this way:
        #         (ascii('1')<<(0*8)) + 
        #         (ascii('2')<<(1*8)) + 
        #         (ascii('7')<<(2*8)) + 
        #         (ascii('.')<<(3*8)) + 
        #         (ascii('0')<<(4*8))  
        self.var_lst.append({
                 'value' : 'default',
                 'name' : 'ctl_svr_addr',
                 'str2int' : self._str2_big_int,
                 'int2str' : self._big_int2_str,
                 'wid_type' : 'entry',
                 'comment' : None
                 })
        # the timeout (us) of the select syscall in the
        # control thread of the frontend.
        self.var_lst.append({
                 'value' : 'default',
                 'name' : 'ctl_t_us',
                 'wid_type' : 'entry',
                 'comment' : None
                 })


    def _str2_big_int(self, ch):
        try:
            tmp = ch.strip().lower()
            if tmp == 'default':
                return 0, 'DEF'
            big_int = 0 
            for (i,x) in enumerate(tmp):
                big_int += ord(x) << (i*8)                                                                                                                   
        except:
            return 0, 'ERR'
        return big_int, 'OK'
    
    def _big_int2_str(self, big_int):
        hex_str = '%x' % big_int
        str_out = ''
        for i in range(len(hex_str)//2):
            sub_str = '0x' + hex_str[i*2:i*2+2]
            str_out = '%c' % int(sub_str, 16) + str_out
        return str_out


########## TEST ##########
#win = tk.Tk()
#adv_conf = adv_conf_fe()
#tk.Button(win, text='button', command=adv_conf.show_win).pack()
#win.mainloop()
