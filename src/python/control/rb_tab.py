#!/bin/python
# -*- coding: UTF-8 -*-
__metaclass__ = type

# this is the ring buffers tab of the GUI controller 
##########
# By B.Gao Feb. 2019

import tkinter as tk
from tkinter import ttk
from tkinter import messagebox


class rb_tab:
    def __init__(self, parent):
       # main frame (to be added as a tab in a notebook widget)
        self.frm = tk.Frame(parent)
        # create all sub windows inside the main frame
        self._create_all()

        self.if_show = False
        self.rb_height = 70
        self.rb_width = 200

    def get_frm(self):
        return self.frm

    def _create_all(self):
        # we need to create scroll bar, similar as config.py
        self.canv = tk.Canvas(self.frm, bg='gray')
        self.rb_frm = tk.Frame(self.canv, bg='gray') 
        self.vsb = tk.Scrollbar(self.frm, orient='vertical',
                        command=self.canv.yview)
        self.canv.configure(yscrollcommand=self.vsb.set)
        self.vsb.place(x=770, y=30, width=20, height=500)
        self.canv.place(x=0, y=30, width=790, height=500)
        self.canv.create_window((0,0), window=self.rb_frm, anchor='nw')
        self.rb_frm.bind('<Configure>', self._frm_conf)
        self.hsb = tk.Scrollbar(self.frm, orient='horizontal',
                        command=self.canv.xview)
        self.canv.configure(xscrollcommand=self.hsb.set)
        self.hsb.place(x=0, y=530, width=790, height=20)
        self.rb_frm.bind('<Configure>', self._frm_conf)

        # create a button to show/hide the ring buffer map
        self.butt_show_rb = tk.Button(self.frm, text='show/hide ring buffer map', command=self._show_hide)
        self.butt_show_rb.place(x=0,y=0)

    def _frm_conf(self, evt):
        self.canv.configure(scrollregion=self.canv.bbox('all'))

    def _show_hide(self):
        self.if_show = not self.if_show

    def draw_rb_fe(self, fe_id, tot_sz, use_sz):
        w = self.rb_width
        h = self.rb_height
        self.canv.create_rectangle(fe_id*w, 0, (fe_id+1)*(w), 2*h, fill='white')
        self.canv.create_text(fe_id*w+w/2, h/4, text='RD_TRIG(FE%d)' % fe_id,
                font='Times 20')
        self.canv.create_line(fe_id*w, h/2, (fe_id+1)*w, h/2, dash=(4,4))
        self.canv.create_line(fe_id*w, h+h/2, (fe_id+1)*w, h+h/2, dash=(4,4))
        self.canv.create_text(fe_id*w+w/2, h/4+h/2+h, text='SENDER',
                font='Times 20')
        self._draw_rb(fe_id*w, h/2, 'rb_fe', tot_sz, use_sz)

    def draw_rb_ana_scal(self, tot_sz, use_sz):
        w = self.rb_width
        h = self.rb_height
        self.canv.create_rectangle(0, h*7, (2)*(w), 9*h, fill='#ffaa00')
        self.canv.create_text(w/2, h*7+h/4, text='RECV', font='Times 20')
        self.canv.create_line(0, h*7+h/2, (2)*w, h*7+h/2, dash=(4,4))
        self.canv.create_line(0, h*8+h/2, (2)*w, h*8+h/2, dash=(4,4))
        self.canv.create_text(w/2, h*8+h/2+h/4, text='ANA_MAIN', font='Times 20')
        self._draw_rb(0, h*7+h/2, 'rb_scal', tot_sz, use_sz)

    def draw_rb_ana_trig(self, tot_sz, use_sz):
        w = self.rb_width
        h = self.rb_height
        self._draw_rb(w, h*7+h/2, 'rb_trig', tot_sz, use_sz)

    def draw_rb_log_scal(self, tot_sz, use_sz):
        w = self.rb_width
        h = self.rb_height
        self.canv.create_rectangle(w*2, h*7, (4)*(w), 9*h, fill='yellow')
        self.canv.create_text(w*2+w/2, h*7+h/4, text='RECV', font='Times 20')
        self.canv.create_line(w*2, h*7+h/2, (4)*w, h*7+h/2, dash=(4,4))
        self.canv.create_line(w*2, h*8+h/2, (4)*w, h*8+h/2, dash=(4,4))
        self.canv.create_text(w*2+w/2, h*8+h/2+h/4, text='SAVE', font='Times 20')
        self._draw_rb(w*2, h*7+h/2, 'rb_scal', tot_sz, use_sz)

    def draw_rb_log_trig(self, tot_sz, use_sz):
        w = self.rb_width
        h = self.rb_height
        self._draw_rb(w*3, h*7+h/2, 'rb_trig', tot_sz, use_sz)



    def draw_rb_ebd_recv(self, fe_tot, fe_id, tot_sz, use_sz):
        w = self.rb_width
        h = self.rb_height
        self.canv.create_rectangle(0, h*2, (fe_tot+1)*(w), 7*h, fill='green')
        self.canv.create_text(w/2, h*2+h/4, text='RECV', font='Times 20')
        self.canv.create_line(0, h*2+h/2, (1+fe_tot)*w, h*2+h/2, dash=(4,4))
        self.canv.create_line(0, h*3+h/2, (1+fe_tot)*w, h*3+h/2, dash=(4,4))
        self.canv.create_text(w/2, h*3+h/2+h/4, text='SORT', font='Times 20')
        self.canv.create_line(0, h*4, (1+fe_tot)*w, h*4, dash=(4,4))
        self.canv.create_line(0, h*5, (fe_tot)*w, h*5, dash=(4,4))
        self.canv.create_text(w/2, h*5+h/4, text='MERGER', font='Times 20')
        self.canv.create_line(0, h*5+h/2, (fe_tot)*w, h*5+h/2, dash=(4,4))
        self.canv.create_line(0, h*6+h/2, (1+fe_tot)*w, h*6+h/2, dash=(4,4))
        self.canv.create_text(w/2, h*6+h/2+h/4, text='SENDER', font='Times 20')
        self._draw_rb(fe_id*w, h*2+h/2, 'rb_fe(%d)'%fe_id, tot_sz, use_sz)
    
    def draw_rb_ebd_sort(self, mod_id, crate_slot, sz_tot, sz_use):
        w = self.rb_width
        h = self.rb_height
        crate = crate_slot&0xff
        slot = (crate_slot>>8)&0xff
        self._draw_rb(mod_id*w, h*4, 'crate%02d:slot%02d'%(crate,slot), sz_tot, sz_use)

    def draw_rb_ebd_merger(self, sz_tot, sz_use):
        w = self.rb_width
        h = self.rb_height
        self._draw_rb(0, h*5+h/2, 'rb_evt', sz_tot, sz_use)

    def draw_rb_ebd_scal(self, n_mod, sz_tot, sz_use):
        w = self.rb_width
        h = self.rb_height
        self._draw_rb(n_mod*w, h*4, 'rb_scal', sz_tot, sz_use)


    def _draw_rb(self, x, y, name, tot_sz, use_sz):
        h = self.rb_height
        #tot_sz=100000000
        #use_sz=30000000
        self.canv.create_rectangle(x+20, y, x+30, y+h, outline='blue', width=2)
        self.canv.create_rectangle(x+20+2, y+h*(1-1.*use_sz/tot_sz), x+30-2,
                y+h, width=2, fill='red', outline='red')
        self.canv.create_text(x+20+30-5, y+5, text=name, anchor='nw',
                font='Times 15')
        self.canv.create_text(x+20+30-5, y+5+20, text='total: %.1f MB' %
                (tot_sz/1024./1024), anchor='nw', font='Times 15')
        self.canv.create_text(x+20+30-5, y+5+20*2, 
                text='used: %.1f MB (%.1f%%)' % (use_sz/1024./1024, 100.*use_sz/tot_sz), 
                anchor='nw', font='Times 15')









