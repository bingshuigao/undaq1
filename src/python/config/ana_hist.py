#!/bin/python
# -*- coding: UTF-8 -*-
__metaclass__ = type

# this class defines the histograms to be shown in analyzer.
# By B.Gao Jan. 2019


import tkinter as tk
from tkinter import messagebox
from tkinter import ttk


class ana_hist:
    def __init__(self):
        
        # hists is a list of hist. Each hist is a list with the following
        # definations (each element in the tuple is a varaible):
        # widgets is a list of widget. Like the hists, but each element is a
        # widget associated with the varabile of the hist):
        # hist[0] = id
        # hist[1] = type (TH1D, TH2D)
        # hist[2] = name (str)
        # hist[3] = folder (str)
        # hist[4] = nbinsX
        # hist[5] = X_min
        # hist[6] = X_max
        # hist[7] = nbinsY
        # hist[8] = Y_min
        # hist[9] = Y_max
        # hist[10] = 'if delete (bool)'
        self.hists = []
        self.widgets = []
        self.h_id = 'id'
        self.h_type = 'type'
        self.h_name = 'name'
        self.h_folder = 'folder'
        self.h_nbinsX = 'nbinsX'
        self.h_nbinsY = 'nbinsY'
        self.h_X_min = 'X_min'
        self.h_X_max = 'X_max'
        self.h_Y_min = 'Y_min'
        self.h_Y_max = 'Y_max'


    # get a list of histos
    def get_all_hists(self):
        return self.hists

    def set_hist(self, h):
        self.hists = h

    # set the histos
    def add_hist(self, h):
        self.hists.append(h)

    # create widgets based on the parameters of a hist
    def _create_wid(self, h):
        wid = {}
        # the id
        wid['id'] = tk.Entry(self.frm_set, width=5)
        wid['id'].insert(0, h[0])
        # the type
        wid['type'] = ttk.Combobox(self.frm_set, state='readonly', 
                values=['TH1D', 'TH2D'], width=8)
        wid['type'].set(h[1])
        # the name
        wid['name'] = tk.Entry(self.frm_set, width=10)
        wid['name'].insert(0, h[2])
        # the folder
        wid['folder'] = tk.Entry(self.frm_set, width=10)
        wid['folder'].insert(0, h[3])
        # the nbinsX
        wid['nbinsX'] = tk.Entry(self.frm_set, width=8)
        wid['nbinsX'].insert(0, h[4])
        # the X_min
        wid['X_min'] = tk.Entry(self.frm_set, width=8)
        wid['X_min'].insert(0, h[5])
        # the X_max
        wid['X_max'] = tk.Entry(self.frm_set, width=8)
        wid['X_max'].insert(0, h[6])
        # the nbinsY
        wid['nbinsY'] = tk.Entry(self.frm_set, width=8)
        wid['nbinsY'].insert(0, h[7])
        # the Y_min
        wid['Y_min'] = tk.Entry(self.frm_set, width=8)
        wid['Y_min'].insert(0, h[8])
        # the Y_max
        wid['Y_max'] = tk.Entry(self.frm_set, width=8)
        wid['Y_max'].insert(0, h[9])
        # delete checkbox
        wid['var_del'] = tk.IntVar()
        wid['del'] = tk.Checkbutton(self.frm_set, text='delete',
                variable=wid['var_del'])
        return wid



    # this method should be called when the user wants to edit the hists
    def show_win(self):
        self._init_layouts()

    def get_win(self):
        return self.win

    def _init_layouts(self):
        self.win = tk.Toplevel()
        self.frm = tk.Frame(self.win, width=1000, height=600)
        self.frm.pack()
        
        # defination of histos, because we need to implement a scrollbar, the
        # codes are much more complecated. The implementation of scrollbar is
        # reffered to
        # https://stackoverflow.com/questions/3085696/adding-a-scrollbar-to-a-group-of-widgets-in-tkinter
        self.canv_set = tk.Canvas(self.frm, bg='gray')
        self.frm_set = tk.Frame(self.canv_set, bg='gray') 
        self.vsb_set = tk.Scrollbar(self.frm, orient='vertical',
                command=self.canv_set.yview)
        self.canv_set.configure(yscrollcommand=self.vsb_set.set)
        self.vsb_set.place(x=1000-20, y=50, width=20, height=550)
        self.canv_set.place(x=0, y=50, width=1000, height=550)
        self.canv_set.create_window((0,0), window=self.frm_set, anchor='nw')
        self.frm_set.bind('<Configure>', self._frm_set_conf)

        # labels
        x_width = 50
        gap = 2
        x_pos = 0
        tmp = tk.Label(self.frm, bg='green', text='id')
        tmp.place(x=x_pos, y=0, width=x_width, height=50)
        x_pos += x_width + gap
        x_width = 60
        tmp = tk.Label(self.frm, bg='green', text='type')
        tmp.place(x=x_pos, y=0, width=x_width, height=50)
        x_pos += x_width + gap
        x_width = 100
        tmp = tk.Label(self.frm, bg='green', text='name')
        tmp.place(x=x_pos, y=0, width=x_width, height=50)
        x_pos += x_width + gap
        x_width = 85
        tmp = tk.Label(self.frm, bg='green', text='folder')
        tmp.place(x=x_pos, y=0, width=x_width, height=50)
        x_pos += x_width + gap
        x_width = 67
        tmp = tk.Label(self.frm, bg='green', text='nbinsX')
        tmp.place(x=x_pos, y=0, width=x_width, height=50)
        x_pos += x_width + gap
        x_width = 67
        tmp = tk.Label(self.frm, bg='green', text='X_min')
        tmp.place(x=x_pos, y=0, width=x_width, height=50)
        x_pos += x_width + gap
        x_width = 67
        tmp = tk.Label(self.frm, bg='green', text='X_max')
        tmp.place(x=x_pos, y=0, width=x_width, height=50)
        x_pos += x_width + gap
        x_width = 67
        tmp = tk.Label(self.frm, bg='green', text='nbinsY')
        tmp.place(x=x_pos, y=0, width=x_width, height=50)
        x_pos += x_width + gap
        x_width = 67
        tmp = tk.Label(self.frm, bg='green', text='Y_min')
        tmp.place(x=x_pos, y=0, width=x_width, height=50)
        x_pos += x_width + gap
        x_width = 67
        tmp = tk.Label(self.frm, bg='green', text='Y_max')
        tmp.place(x=x_pos, y=0, width=x_width, height=50)

        # the delete and add and OK buttons
        self.butt_add = tk.Button(self.frm, text='Add', command=self._add)
        self.butt_del = tk.Button(self.frm, text='Del', command=self._del)
        self.butt_ok = tk.Button(self.frm, text='OK', command=self._OK)
        x_pos += x_width + gap
        x_width = 50
        self.butt_del.place(x=x_pos, y=0, width=x_width, height=50)
        x_pos += x_width + gap
        x_width = 50
        self.butt_add.place(x=x_pos, y=0, width=x_width, height=50)
        x_pos += x_width + gap
        x_width = 50
        self.butt_ok.place(x=x_pos, y=0, width=x_width, height=50)

        # show the widgets associated with the hists:
        self.widgets = []
        self._update_wid()

    # update the internal hists by the widgets
    def _update_hists(self):
        self.hists = []
        for wid in self.widgets:
            if wid['var_del'].get():
                continue
            tmp = []
            self.h_id = int(wid['id'].get())
            self.h_type = wid['type'].get()
            self.h_name = wid['name'].get()
            self.h_folder = wid['folder'].get()
            self.h_nbinsX = int(wid['nbinsX'].get())
            self.h_X_min = float(wid['X_min'].get())
            self.h_X_max = float(wid['X_max'].get())
            if wid['type'].get() == 'TH1D':
                self.h_nbinsY = 0
                self.h_Y_min = 0
                self.h_Y_max = 0
            elif wid['type'].get() == 'TH2D':
                self.h_nbinsY = wid['nbinsY'].get()
                self.h_Y_min = wid['Y_min'].get() 
                self.h_Y_max = wid['Y_max'].get() 
            tmp.append(self.h_id)
            tmp.append(self.h_type)
            tmp.append(self.h_name)
            tmp.append(self.h_folder)
            tmp.append(self.h_nbinsX)
            tmp.append(self.h_X_min)
            tmp.append(self.h_X_max)
            tmp.append(self.h_nbinsY)
            tmp.append(self.h_Y_min)
            tmp.append(self.h_Y_max)
            tmp.append(False)
            self.hists.append(tmp)



    def _OK(self):
        self._update_hists()
        self.win.destroy()

    def _increment_id(self):
        try:
            self.h_id += 1
            name_id = 0
            base = 1
            for ch in reversed(self.h_name):
                if ch in [str(i) for i in range(10)]:
                    name_id += base * int(ch)
                    base *= 10
                else:
                    break
            name_hd = self.h_name[0:len(self.h_name)-len(str(base//10))]
            fmat = '%%s%%0%dd' % len(str(base//10))
#            print(base)
            self.h_name = fmat % (name_hd,name_id+1)
        except:
            pass


    # add a hist
    def _add(self):
        self._update_hists()
        self._increment_id()
        h = [self.h_id, self.h_type, self.h_name, self.h_folder, self.h_nbinsX,
                self.h_X_min, self.h_X_max, self.h_nbinsY, self.h_Y_min,
                self.h_Y_max, False]
        self.add_hist(h)
        self._update_wid()

    # update the widgets based on the internal self.hist
    def _update_wid(self):
        self._rm_wid()
        self.widgets = []
        for h in self.hists:
            if h[10]:
                continue
            self.widgets.append(self._create_wid(h))

        row = 1
        for wid in self.widgets:
            self._place_wid(row, wid)
            row += 1

    def _rm_wid(self):
        for wid in self.widgets:
            wid['id'].destroy()
            wid['type'].destroy()
            wid['name'].destroy()
            wid['folder'].destroy()
            wid['nbinsX'].destroy()
            wid['X_min'].destroy()
            wid['X_max'].destroy()
            wid['nbinsY'].destroy()
            wid['Y_min'].destroy()
            wid['Y_max'].destroy()
            wid['del'].destroy()
            

    # place the hist in row
    def _place_wid(self, row, h):
        h['id'].grid(    row=row, column=1)
        h['type'].grid(  row=row, column=2)
        h['name'].grid(  row=row, column=3)
        h['folder'].grid(row=row, column=4)
        h['nbinsX'].grid(row=row, column=5)
        h['X_min'].grid( row=row, column=6)
        h['X_max'].grid( row=row, column=7)
        h['nbinsY'].grid(row=row, column=8)
        h['Y_min'].grid( row=row, column=9)
        h['Y_max'].grid( row=row, column=10)
        h['del'].grid(   row=row, column=11)



    # delete the selected hist
    def _del(self):
        self._update_hists()
        self._update_wid()
    
    # Callback functions of the scrollbars
    def _frm_set_conf(self, evt):
        self.canv_set.configure(scrollregion=self.canv_set.bbox('all'))
