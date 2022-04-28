#!/bin/python
# -*- coding: UTF-8 -*-
__metaclass__ = type

# this is the frontend tab of the notebook widget in the main window. It
# manages everything (settings) of the frontend.
# How to add another supported module ?
#     1, Derive a subclass from the base class vme_mod (see the comments in the
#        file vme_mod.py to know how)
#     2, import the new class for the new module
#     3, Add a name for the new supported module into the list:
#        self.sup_mods_lst (in the __init__ function).
#     4, In the function self._new_mod, add another elif clause (similar
#        sentenses as the existing supported modules)
##########
# By B.Gao Sep. 2018

import tkinter as tk
from tkinter import ttk
from tkinter import messagebox
from madc32 import madc32
from mqdc32 import mqdc32
from v1190 import v1190
from v830 import v830
from v977 import v977
from v2718 import v2718
from test_ctl import test_ctl
from adv_conf_fe import adv_conf_fe
from v1740 import v1740
from v1751 import v1751
from v775 import v775
from v785 import v785
from v792 import v792
from v775n import v775n
from v785n import v785n
from fake_module import fake_module
from pixie16 import pixie16
from pixie16_ctl import pixie16_ctl


class frontend:
    def __init__(self, parent):
        # number of crates
        self.n_crate = 1 
        # list of supported modules
        self.sup_mods_lst = ['V2718', 'TEST_CTL', 'MADC32', 'MQDC32', 'V1190A', 'V830', 'V977',
                'V1740', 'V1751', 'V775', 'V775N', 'V785', 'V785N', 'V792', 'FAKE_MODULE', 'PIXIE16', 'PIXIE16_CTL']
        # list of selected modules (note: the element is an object of the
        # vme_mod class, not a string as in the sup_mods_list)
        self.sel_mods_lst = []
        # the advanced settings
        self.adv_conf = adv_conf_fe()
       
       # main frame (to be added as a tab in a notebook widget)
        self.frm = tk.Frame(parent)
        # create all sub windows inside the main frame
        self._create_all()

    def get_frm(self):
        return self.frm
    def get_crate_n(self):
        return self.n_crate
    def set_crate_n(self,n):
        self.n_crate = n

    def get_adv_conf(self):
        return self.adv_conf

    def _create_all(self):
        # create the listbox for the supported modules
        tk.Label(self.frm, text='supported modules', 
                bg = 'yellow').place(x=0, y=10, width=150, height=20)
        self.sup_mods_w = tk.Listbox(self.frm)
        self.sup_mods_w.place(x=0, y=32, width=150, height=500)
        for mod in self.sup_mods_lst:
            self.sup_mods_w.insert(tk.END, mod)
        self.sup_mods_w.selection_set(0)

        #create the listbox for the selected modules
        tk.Label(self.frm, text='selected modules', 
                bg = 'green').place(x=300, y=10, width=150, height=20)
        self.sel_mods_w = tk.Listbox(self.frm)
        self.sel_mods_w.place(x=300, y=32, width=250, height=500)
        self._update_sel_mod_w()


        # create the buttons
        self.butt_new_crate = tk.Button(self.frm, text='New Crate',
                command=self._new_crate)
        self.butt_rm_crate = tk.Button(self.frm, text='rm last Crate',
                command=self._rm_crate)
        self.butt_add = tk.Button(self.frm, text='Add ==>',
                command=self._add_module)
        self.butt_up =  tk.Button(self.frm, text='Move up',
                command=self._move_up_slot)
        self.butt_down =tk.Button(self.frm, text='Move down',
                command=self._move_down_slot)
        self.butt_del = tk.Button(self.frm, text='Delete',
                command=self._del_module)
        self.butt_edit= tk.Button(self.frm, text='Edit', 
                command=self._edit_module)
        self.butt_dup= tk.Button(self.frm, text='duplicate', 
                command=self._dup_module)
        self.butt_adv = tk.Button(self.frm, text='advanced...',
                command=self._adv_conf)
        self.butt_new_crate.place(x=180, y=250, width=100, height=25)
        self.butt_rm_crate.place(x=180, y=280, width=100, height=25)
        self.butt_add.place(x=180, y=310, width=100, height=25)
        self.butt_up.place(x=560, y=250, width=100, height=25)
        self.butt_down.place(x=560, y=280, width=100, height=25)
        self.butt_del.place(x =560, y=310, width=100, height=25)
        self.butt_edit.place(x =560, y=340, width=100, height=25)
        self.butt_dup.place(x =560, y=370, width=100, height=25)
        self.butt_adv.place(x =560, y=400, width=100, height=25)

    def _adv_conf(self):
        self.adv_conf.show_win()
        self.adv_conf.get_win().grab_set()
        self.frm.wait_window(self.adv_conf.get_win())

    def _new_crate(self):
        self.n_crate += 1
        self._update_sel_mod_w()
    def _rm_crate(self):
        if self.n_crate > 1:
            self.n_crate -= 1
        self._update_sel_mod_w()

    def _do_add_module(self, name, mod):
        tmp = self._new_mod(name, mod)
        if not tmp:
            self._show_msg('cannot add module!', 'err')
            return
        if self._conf_mod(tmp) == 'OK':
            self.sel_mods_lst.append(tmp)
            self._update_sel_mod_w()

    def _new_mod(self, name, mod):
        # Here is where we create different kinds of modules. Add more
        # supported modules here if needed.
        if name == 'MADC32':
            tmp = madc32(self._get_uniq_name(name), mod)
        elif name == 'MQDC32':
            tmp = mqdc32(self._get_uniq_name(name), mod)
        elif name == 'V1190A':
            tmp = v1190(self._get_uniq_name(name), mod)
        elif name == 'V830':
            tmp = v830(self._get_uniq_name(name), mod)
        elif name == 'V2718':
            tmp = v2718(self._get_uniq_name(name), mod)
        elif name == 'TEST_CTL':
            tmp = test_ctl(self._get_uniq_name(name), mod)
        elif name == 'V977':
            tmp = v977(self._get_uniq_name(name), mod)
        elif name == 'V1740':
            tmp = v1740(self._get_uniq_name(name), mod)
        elif name == 'V1751':
            tmp = v1751(self._get_uniq_name(name), mod)
        elif name == 'V775':
            tmp = v775(self._get_uniq_name(name), mod)
        elif name == 'V792':
            tmp = v792(self._get_uniq_name(name), mod)
        elif name == 'V785':
            tmp = v785(self._get_uniq_name(name), mod)
        elif name == 'V775N':
            tmp = v775n(self._get_uniq_name(name), mod)
        elif name == 'V785N':
            tmp = v785n(self._get_uniq_name(name), mod)
        elif name == 'FAKE_MODULE':
            tmp = fake_module(self._get_uniq_name(name), mod)
        elif name == 'PIXIE16_CTL':
            tmp = pixie16_ctl(self._get_uniq_name(name), mod)
        elif name == 'PIXIE16':
            tmp = pixie16(self._get_uniq_name(name), mod)
        else:
            return None
        tmp.set_max_crate(self.n_crate)
        return tmp

    def _add_module(self):
        try:
            name = self.sup_mods_w.get(self.sup_mods_w.curselection())
        except:
            self._show_msg('cannot add module!', 'err')
            return
        self._do_add_module(name, None)

    # a local loop in which the module is configured 
    def _conf_mod(self, mod):
        while True:
            mod.show_win()
            mod.get_win().grab_set()
            self.frm.wait_window(mod.get_win())
            stat = mod.get_stat()
            if stat != 'OK':
                break

            # check if all the settings are good
            if mod.check() != 'OK':
                self._show_msg('some settings are incorrect!', 'err')
                continue

            # check if the base address and slot numbers conflict with
            # existing modules
            bad_base = False
            bad_slot = False
            bad_geo = False
            base = mod.get_base()
            crate = mod.get_crate()
            slot = mod.get_slot()
            mod_geo = mod.get_geo()
            mod_name = mod.get_name()[0:-3]
            #print(mod_name)
            for tmp in self.sel_mods_lst:
                if tmp is mod:
                    continue
                if crate == tmp.get_crate():
                    if base == tmp.get_base():
                        bad_base = True
                        break
                    if slot == tmp.get_slot():
                        bad_slot = True
                        break
                    if mod_geo == tmp.get_geo():
                        if mod_name == tmp.get_name()[0:-3]:
                            bad_geo = True
            if bad_base:
                self._show_msg('base address conflicts', 'err')
                continue
            if bad_slot:
                self._show_msg('slot number conflicts', 'err')
                continue
            if bad_geo:
                self._show_msg('geo conflicts', 'err')
                continue
            # if we reached this point, all the checks are passed, we need
            # to break the loop
            break
        return stat
    def _move_up_slot(self):
        self._move_slot('up')

    def _move_down_slot(self):
        self._move_slot('down')

    def _move_slot(self, direction):
        i = self._get_cur_mod()
        if i < 0:
            self._show_msg('cannot move module!', 'err')
            return
        slot = self.sel_mods_lst[i].get_slot()
        crate = self.sel_mods_lst[i].get_crate()
        # we first need to move the module down(up) if there is one
        # above(below)
        if direction == 'up':
            slot -= 1
            for mod in self.sel_mods_lst:
                if mod.get_slot() == slot and mod.get_crate() == crate:
                    mod.move_down()
            self.sel_mods_lst[i].move_up()
        else:
            slot += 1
            for mod in self.sel_mods_lst:
                if mod.get_slot() == slot and mod.get_crate() == crate:
                    mod.move_up()
            self.sel_mods_lst[i].move_down()
        self._update_sel_mod_w()

    def _del_module(self):
        i = self._get_cur_mod()
        if i < 0:
            self._show_msg('cannot delete module', 'err')
            return
        del self.sel_mods_lst[i]
        self._update_sel_mod_w()

    # get the currently selected module in the sel_mods_w widget
    def _get_cur_mod(self):
        try:
            name = self.sel_mods_w.get(self.sel_mods_w.curselection())
        except:
            return -1
        for i,mod in enumerate(self.sel_mods_lst):
            if name.find(mod.get_name()) >= 0:
                return i
        return -1

    def _edit_module(self):
        i = self._get_cur_mod()
        if i < 0:
            self._show_msg('cannot edit module!', 'err')
            return
        cur_mod = self.sel_mods_lst[i]
        # make a copy of the current module, just in case the user pressed
        # 'cancel'
        name = cur_mod.get_name()[:-3]
        tmp = self._new_mod(name, cur_mod)
        tmp.set_name(cur_mod.get_name())
        if self._conf_mod(cur_mod) == 'OK':
            self._update_sel_mod_w()
        else:
            self.sel_mods_lst[i] = tmp

    # duplicate a module
    def _dup_module(self):
        i = self._get_cur_mod()
        if i < 0:
            self._show_msg('cannot duplicate module', 'err')
            return
        cur_mod = self.sel_mods_lst[i]
        name = cur_mod.get_name()[:-3]
        self._do_add_module(name, cur_mod)

    def _show_msg(self, msg, flag='info'):
        if flag == 'err':
            messagebox.showerror('error', msg, parent=self.frm)
        else:
            messagebox.showinfo('info', msg, parent=self.frm)

    def _update_sel_mod_w(self):
        self.sel_mods_w.delete(0, tk.END)
        # debug ...
#        print(self.n_crate)
        #########
        for i in range(self.n_crate):
            self.sel_mods_w.insert(tk.END, 'Crate%02d' % (i))
            for j in range(21):
                mod = self._find_mod(i, j)
                if mod:
                    txt = '%10s@0x%08x' % (mod.get_name(), mod.get_base())
                else:
                    txt = ''
                self.sel_mods_w.insert(tk.END, 'SLOT%02d---->%s' % (j, txt))
        # highlight scaler type modules:
        for i in range(self.n_crate):
            for j in range(21):
                mod = self._find_mod(i, j)
                if mod:
                    if mod.get_type().lower() == 's':
                        self.sel_mods_w.itemconfig(i*22+j+1, {'bg' : 'gray'})


    def _find_mod(self, crate, slot):
        for mod in self.sel_mods_lst:
            if mod.get_crate() == crate and mod.get_slot() == slot:
                return mod
        return None

    def _get_uniq_name(self, name):
        num = 0
        for mod in self.sel_mods_lst:
            if mod.get_name().find(name) >= 0:
                num += 1
        return '%s%03d' % (name, num)

    # get a list of selected modules:
    def get_sel_mod(self):
        return self.sel_mods_lst

    # create a new module and return it:
    def create_mod(self, name):
        tmp = self._new_mod(name[0:-3], None)
        self.sel_mods_lst.append(tmp)
        return tmp

    # update the selected modules window:
    def update_win(self):
        self._update_sel_mod_w()


            

# tests ##########################
win = tk.Tk()
win.geometry('800x650')
frm = tk.Frame(win)
frm.place(x=0, y=30, width=800, height=900) 
tab_ctrl = ttk.Notebook(frm)
tab1 = frontend(tab_ctrl)
tab_ctrl.add(tab1.get_frm(), text='frontend')
tab_ctrl.pack(fill=tk.BOTH, expand=1) 
win.mainloop()
