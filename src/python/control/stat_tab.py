#!/bin/python
# -*- coding: UTF-8 -*-
__metaclass__ = type

# this is the status tab of the GUI controller 
##########
# By B.Gao Feb. 2019

import tkinter as tk
from tkinter import ttk
from tkinter import messagebox


class stat_tab:
    def __init__(self, parent):
       # main frame (to be added as a tab in a notebook widget)
        self.frm = tk.Frame(parent)
        # create all sub windows inside the main frame
        self._create_all()
        
        self.cur_t = 0
        self.cur_t1 = 0
        self.cur_cnt = [0 for i in range(100)]
        self.cur_cnt1 = 0

    def get_frm(self):
        return self.frm

    def _create_all(self):
        self.label_rate = tk.Label(self.frm, text='data rate:', height=1,
                width=100, anchor='w')
        self.label_rate.place(x=0, y=0)
        self.label_data_sz = tk.Label(self.frm, text='data size:', height=1,
                width=100, anchor='w')
        self.label_data_sz.place(x=0, y=25)
        tmp_txt = '-' * 100 
        tmp_txt += '\n%5s%15s%15s%25s%20s'%('id', 'crate', 'slot', 
                'total trig', 'rate(/s)')
        tk.Label(self.frm, text=tmp_txt, height=2, width=300,
                anchor='nw').place(x=0, y=50)
        self.trig_cnt_label = tk.Label(self.frm, text='',height=100, width=300,
            anchor='nw')
        self.trig_cnt_label.place(x=0, y=90)
        self.rate_canv = tk.Canvas(self.frm, height=250, width=380, bg='white')
        self.rate_canv.place(x=410, y=10)
        tk.Label(self.rate_canv, text='rate ID:', height=1,
                width=7).place(x=200, y=3)
        self.sel_rate_entry = tk.Entry(self.rate_canv)
        self.sel_rate_entry.place(x=270,y=3,width=50,height=20)
        self.sel_rate_entry.insert(tk.END, 0)
        self._reset_rate_curve()
        self.sel_rate_butt = tk.Button(self.rate_canv, text='set',
                command=self._reset_rate_curve)
        self.sel_rate_butt.place(x=325,y=3,width=30,height=20)
        self.scal_canv = tk.Canvas(self.frm, height=250, width=380, bg='white')
        self.scal_canv.place(x=410, y=275)
        self.comb_cnters = ttk.Combobox(self.scal_canv, values=['cnter%02d' % i
            for i in range(30)], state='readonly')
        self.comb_cnters.set('cnter00')
        self.comb_cnters.bind("<<ComboboxSelected>>", self._choose_cnter)
        self.comb_cnters.place(x=5, y=5, width=80)
        self.scal_font_inc_butt = tk.Button(self.scal_canv, text='size +',
                command=self._scal_font_inc)
        self.scal_font_dec_butt = tk.Button(self.scal_canv, text='size -',
                command=self._scal_font_dec)
        self.scal_font_inc_butt.place(x=90, y=5, height=20)
        self.scal_font_dec_butt.place(x=160, y=5, height=20)
        self.scal_font_sz = 30
        self.select_cnt = 0


    def _reset_rate_curve(self):
        self.rate_curv = []
        try:
            self.rate_id = int(self.sel_rate_entry.get())
        except:
            self.rate_id = 0
        self.update_timer = 0
        self.rate_min = 1000000
        self.rate_max = 0

    def _scal_font_dec(self):
        self.scal_font_sz /= 1.1
#        self.update_scal_cnter([1000, 1000, 1000])
    def _scal_font_inc(self):
        self.scal_font_sz *= 1.1
#        self.update_scal_cnter([1000, 1000, 1000])

    def update_scal_cnter(self, cnts):
        self.scal_canv.delete('all')
        self.scal_canv.create_text(190, 145, text=str(cnts[self.select_cnt]),
                font='Times %d' % self.scal_font_sz)


    def _choose_cnter(self, evt):
        print('cnters: %s chosen' % self.comb_cnters.get())
        self.select_cnt = int(self.comb_cnters.get()[5:])
        print(self.select_cnt)
        



    def set_rate(self, rate):
        self.label_rate.config(text='data rate: %.2f kB/s'%rate)

    def set_data_sz(self, sz):
        self.label_data_sz.config(text='data size: %.3f MB'%sz)

    # update_evt_cnt and update_evt_cnt1 are the responses for the query evt
    # cnts from ebd sort and ebd merger, respectively. We update the texts only
    # in response to the latter (update_evt_cnt1).
    def update_evt_cnt(self, msg):
        ts_hi     = int.from_bytes(msg[0:4], 'little')
        ts_lo     = int.from_bytes(msg[4:8], 'little')
        n_module  = int.from_bytes(msg[8:12],'little')
        ts = (ts_hi<<32) + ts_lo
        delta_t = ts - self.cur_t
        self.label_str = ''
        cnt = 0
        for i in range(n_module):
            int_entry = int.from_bytes(msg[12+(i//2)*4:16+(i//2)*4], 'little')
            slot = (int_entry >>((i%2)*16)) & 0xff
            crate = ((int_entry >> ((i%2)*16) + 8)) & 0xff
            cnt = int.from_bytes(msg[53*4+i*4:54*4+i*4], 'little')
            delta_cnt = cnt - self.cur_cnt[i]
            rate = delta_cnt*1000./delta_t
            rate_str = '%02d%14d%20d%20d%20.3f' % (i+1, crate, slot, cnt, rate)
            print(rate_str)
            self.label_str += rate_str + '\n'
            self.cur_cnt[i] = cnt
            if self.rate_id == (i+1):
                self._update_rate_curve(rate)
#        self.trig_cnt_label.config(text=label_str)
        self.cur_t = ts
    
    def update_evt_cnt1(self, msg):
        ts_hi     = int.from_bytes(msg[0:4], 'little')
        ts_lo     = int.from_bytes(msg[4:8], 'little')
        cnt  = int.from_bytes(msg[8:12],'little')
        ts = (ts_hi<<32) + ts_lo
        delta_t = ts - self.cur_t1
        delta_cnt = cnt - self.cur_cnt1
        rate = delta_cnt*1000./delta_t
        rate_str = '%02d%14s%15s%20d%20.3f' % (0, 'merged rate', '', cnt, rate)
        self.label_str += '-'*100 + '\n' + rate_str + '\n'
        self.cur_cnt1 = cnt
        self.trig_cnt_label.config(text=self.label_str)
        self.cur_t1 = ts
        if self.rate_id == 0:
            self._update_rate_curve(rate)


    def _update_rate_curve(self, rate):
        if rate < 0:
            return
        self.rate_curv.append(rate)
        self.update_timer += 1
        if self.update_timer % 3:
            return
        if len(self.rate_curv) > 1000:
            self.rate_curv = self.rate_curv[-1000:]
        self.rate_max = max(self.rate_curv)
        self.rate_min = min(self.rate_curv)
        if self.rate_max == self.rate_min:
            self.rate_max += 1
        self._draw_curve()


    def _draw_curve(self):
        #           (10,30) -------------- (370,30)
        #              |                      |
        #              |                      |
        #              |                      |
        #           (10,240) ------------- (370,240)   
        self.rate_canv.delete('all')
        self.rate_canv.create_line(10, 30, 370, 30, width=2)
        self.rate_canv.create_line(10, 30, 10, 240, width=2)
        self.rate_canv.create_line(10, 240, 370, 240, width=2)
        self.rate_canv.create_line(370, 30, 370, 240, width=2)
        self.rate_canv.create_line(10, 30+52*1, 370, 30+52*1, dash=(4,2))
        self.rate_canv.create_line(10, 30+52*2, 370, 30+52*2, dash=(4,2))
        self.rate_canv.create_line(10, 30+52*3, 370, 30+52*3, dash=(4,2))
        self.rate_canv.create_line(10+90*1, 30, 10+90*1, 240, dash=(4,2))
        self.rate_canv.create_line(10+90*2, 30, 10+90*2, 240, dash=(4,2))
        self.rate_canv.create_line(10+90*3, 30, 10+90*3, 240, dash=(4,2))
        if len(self.rate_curv) < 2:
            return
        for i in range(len(self.rate_curv)-1):
            x0,y0 = self._convert_coor(i, self.rate_curv[i])
            x1,y1 = self._convert_coor(i+1, self.rate_curv[i+1])
            self.rate_canv.create_line(x0, y0, x1, y1, fill="red")
        for i in range(5):
            self.rate_canv.create_text((20, 10+52*i), anchor="nw",
                    text = '%.1f' % (self.rate_max -
                        (self.rate_max-self.rate_min)*(30+52*i-35)/200.))

    def _convert_coor(self, x, y):
        yy = 35 + (self.rate_max - y)/(self.rate_max - self.rate_min) * 200
        xx = 15 + x/1000.*350
        return (xx, yy)
        





