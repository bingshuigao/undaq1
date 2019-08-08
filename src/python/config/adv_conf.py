#!/usr/bin/python3
# -*- coding: UTF-8 -*-
__metaclass__ = type


# this is the base class of the advanced configuration. To use this base class,
# the user need to derive from it and initialize the self.var_lst variable.
# The variable list: This is the centrol data structure of the class. It is
# a list, each element is a dict which has the following structure.
#   {'value':value, ---> value of the variable, a special value 'default' is
#                        allowed.
#   'name':name, --->name of the variable (will be used as the text of the
#                    label) 
#   'wid_type':type, ---> The type of the widget.  For example, it can be
#                         'entry' or 'comb' which means tk.Entry and
#                         ttk.Combobox, respectively.
#   'wid':window, ---> The widget associated with the variable
#   'wid_values':values ---> The possible values of the widget if it is a
#                            combobox. It is a list .
#   'comment':'comment' ---> Comments (explaination) about this variable.
#  }
# By B.Gao Oct. 2018

import tkinter as tk
from tkinter import messagebox
from tkinter import ttk

class adv_conf:
    def __init__(self):
        pass

    # converts a string (content in a widget) to integer. A status code also
    # returned. The the status codes are 'OK', 'ERR', 'DEF'. The 'ERR' means
    # error and 'DEF' means the string is 'default'
    def _str2int(self, ch):
        try:
            tmp = ch.strip().lower()
            if tmp == 'default':
                return 0, 'DEF'
            base = 10
            if tmp.startswith('0x'):
                base = 16
            return int(tmp, base), 'OK'
        except:
            return 0, 'ERR'

    # update the internale configuration data accoring to the widgets, return 1
    # if error occurs, otherwise return 0
    def _update_data(self):
        ret = 0
        for var in self.var_lst:
            if var['wid_type'] == 'entry':
                val,flag = self._str2int(var['wid'].get())
                if flag == 'DEF':
                    var['value'] = 'default'
                elif flag == 'ERR':
                    self._show_msg('invilid value for %s' % var['name'], 'err')
                    ret = 1
                else:
                    var['value'] = val
            elif var['wid_type'] == 'comb':
                values = var['wid_values']
                selected = var['wid'].get()
                if selected == 'default':
                    var['value'] = 'default'
                else:
                    var['value'] = values.index(selected)
        return ret
        
    def get_win(self):
        return self.win

    def _OK(self):
        if not self._update_data():
            self.stat = 'OK'
            self.win.destroy()
    def _cancel(self):
        self.stat = 'cancel'
        self.win.destroy()
    
    def _init_layouts(self):
        self.win = tk.Toplevel()
        self.frm1 = tk.Frame(self.win, width=800, height=600)
        self.frm1.pack()
        self.frm = tk.Frame(self.frm1)
        self.frm.place(x=0,y=0)

        # The 'OK' and 'Cancel' buttons
        self.stat = 'cancel'
        self.butt_ok = tk.Button(self.win, text='OK', command=self._OK)
        self.butt_cl = tk.Button(self.win, text='Cancel', command=self._cancel)
        self.butt_ok.place(x=0, y=530, width=60, height=25)
        self.butt_cl.place(x=80, y=530, width=60, height=25)
        
        # this controls the layout of the widgets: The layout is col columns,
        # row number is increased as needed.
        self.tot_wid = 0
        self.col_wid = 6

        # init the widgets for the variables
        self.labels = []
        for var in self.var_lst: 
            tmp_wid = tk.Label(self.frm, bg='green', text=var['name'])
            self.labels.append(tmp_wid)
            self._place_win(tmp_wid)
            if var['wid_type'] == 'entry':
                win = tk.Entry(self.frm)
            elif var['wid_type'] == 'comb':
                win = ttk.Combobox(self.frm, state='readonly',
                        values=var['wid_values'])
            self._place_win(win)
            var['wid'] = win

        # update the values of widgets 
        self._update_wid()


    # get a list of xml elements. The elements are represented by tuples:
    # xml_ele = ('name', 'value', 'comment').  Each element with non-default
    # values should generate an xml element.  The comment is the 'comment' in
    # var_lst element if available, otherwise use 'name'.
    def get_conf(self):
        tmp = []
        for var in self.var_lst:
            if var['value'] == 'default':
                continue
            value = '0x%x' % var['value']
            com = var['name']
            name = var['name']
            if var['comment']:
                com = var['comment']
            tmp.append((name,value,com))

        return tmp

    # the opposite of get_conf:
    def set_conf(self, conf):
        try:
            for name,val,com in conf:
                self.set_the_conf(name, val)
        except:
            self._show_msg('error setting the advanced conf: %s' % name, 'err')

    def set_the_conf(self, name, val):
        for var in self.var_lst:
            if var['name'] == name:
                var['value'] = int(val, 16)

    
    # place the window in propriate posistion
    def _place_win(self, win):
        win.grid(row=self.tot_wid//self.col_wid, 
                column=(self.tot_wid%self.col_wid))
        self.tot_wid += 1

    # update the widgets accoring to its values.
    def _update_wid(self):
        for var in self.var_lst:
            tmp = var['value']
            if tmp != 'default':
                if var['wid_type'] == 'entry':
                    value = '%d' % tmp 
                elif var['wid_type'] == 'comb':
                    value = var['wid_values'][tmp]
            else:
                value = 'default'
            if var['wid_type'] == 'entry':
                var['wid'].delete(0, tk.END)
                var['wid'].insert(0, value)
            elif var['wid_type'] == 'comb':
                var['wid'].set(value)
            # if the variable has non-default value, change the color of the
            # label:
            tmp_wid = self._get_label_wid(var['name'])
            if value != 'default':
                tmp_wid.config(bg='yellow')
            else:
                tmp_wid.config(bg='green')
    # get the label widget of the variable with given name
    def _get_label_wid(self, var_name):
        for label_wid in self.labels:
            if label_wid.config()['text'][-1] == var_name:
                return label_wid
        # to avoid exception
        return self.labels[0]


# this method should be called when the user want to edit the advanced
    # configurations. 
    def show_win(self):
        self._init_layouts()

    
    # pops up a messagebox to show the message:
    def _show_msg(self, msg, flag='info'):
        if flag == 'err':
            messagebox.showerror('error', msg, parent=self.frm)
        else:
            messagebox.showinfo('info', msg, parent=self.frm)




