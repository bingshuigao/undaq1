#!/usr/bin/python3
# -*- coding: UTF-8 -*-
__metaclass__ = type

# This script provides a GUI to create configuration file to be used by all
# parts of the DAQ. The configuration file is an xml file and manipulated by
# the xml.etree.ElementTree module. The GUI is based on Tkinter package which
# is a wrapper of Tcl/Tk.
#######
# By B.Gao Sep. 2018


import tkinter as tk
from tkinter import messagebox
from tkinter import ttk
from tkinter import filedialog
from frontend import frontend
from ebd import ebd
from log import log
from ana import ana
from ctl import ctl
import xml.etree.ElementTree as et
from xml.dom import minidom
import os


class config:
    def __init__(self):
        # root window
        self.root_win = tk.Tk()
        self.root_win.geometry('800x600')

        # save and open buttons
        self._create_buttons()

        # create the notebook
        self._create_notebook()

        # The file name associated with the configurations
        self.f_name = None



    # create the open and save buttons:
    def _create_buttons(self):
        self.butt_frm = tk.Frame(self.root_win)
        self.butt_frm.place(x=0, y=0, width=350, height=25)
        self.butt_open = tk.Button(self.butt_frm, text='open...',
                command=self._open)
        self.butt_open.pack(side=tk.LEFT)
        self.butt_saveas = tk.Button(self.butt_frm, text='save as...',
                command=self._saveas)
        self.butt_saveas.pack(side=tk.LEFT)
        self.butt_save = tk.Button(self.butt_frm, text='save',
                command=self._save)
        self.butt_save.pack(side=tk.LEFT)
        self.butt_link = tk.Button(self.butt_frm, text='create link',
                command=self._link)
        self.butt_link.pack(side=tk.LEFT)

    # create the notebook widget
    def _create_notebook(self):
        self.notebook_frm = tk.Frame(self.root_win)
        self.notebook_frm.place(x=0, y=30, width=800, height=600)
        self.tab_ctrl = ttk.Notebook(self.notebook_frm)

        # create the frontend tab
        self._create_frontend_tab(self.tab_ctrl)

        # create the event builder tab
        self._create_evt_builder(self.tab_ctrl)
        
        # create the logger tab
        self._create_logger(self.tab_ctrl)

        # create the analyzer tab
        self._create_analyzer(self.tab_ctrl)
        
        # create the GUI controler tab
        self._create_ctl(self.tab_ctrl)

        # create more tabs here:
        # .............

    # create the frontend tab
    def _create_frontend_tab(self, tab_ctrl):
        self.frontend = frontend(tab_ctrl)
        tab_ctrl.add(self.frontend.get_frm(), text='frontend')
        tab_ctrl.pack(fill=tk.BOTH, expand=1)

    # create the event builder tab
    def _create_evt_builder(self, tab_ctrl):
        self.ebd = ebd(tab_ctrl)
        tab_ctrl.add(self.ebd.get_frm(), text='event builder')

    # create the logger tab
    def _create_logger(self, tab_ctrl):
        self.log = log(tab_ctrl)
        tab_ctrl.add(self.log.get_frm(), text='logger')

    # create the analyzer tab
    def _create_analyzer(self, tab_ctrl):
        self.ana = ana(tab_ctrl)
        tab_ctrl.add(self.ana.get_frm(), text='analyzer')

    # create the GUI controller tab
    def _create_ctl(self, tab_ctrl):
        self.ctl = ctl(tab_ctrl)
        tab_ctrl.add(self.ctl.get_frm(), text='GUI controler')

    
    # save the config file
    def _do_save(self):
        # The element tree representing an empty configuration file. We always
        # start with an empty tree even when we open an existing configuration
        # file.
        root = et.Element('config')

        # first, we create the elements of the frontend
        ele_frontend = et.SubElement(root, 'frontend')
        for mod in self.frontend.get_sel_mod():
            ele_mod = et.SubElement(ele_frontend, 'vme_module')
            for name,val,com in mod.get_conf():
                if name.startswith('reg'):
                    # this element is a register
                    register = et.SubElement(ele_mod, 'register')
                    et.SubElement(register, 'offset').text = name[4:]
                    et.SubElement(register, 'value').text = val
                    et.SubElement(register, 'comment').text = com
                elif name.startswith('glo'):
                    # this is a global variable
                    global_var= et.SubElement(ele_mod, 'global_var')
                    et.SubElement(global_var, 'name').text = name[4:]
                    et.SubElement(global_var, 'value').text = val
                    et.SubElement(global_var, 'comment').text = com
                elif name.startswith('adv'):
                    # this is an advanced variable
                    advance_var= et.SubElement(ele_mod, 'advance_var')
                    et.SubElement(advance_var, 'name').text = name[4:]
                    et.SubElement(advance_var, 'value').text = val
                    et.SubElement(advance_var, 'comment').text = com
        # the advanced settings for frontend
        for name,val,com in self.frontend.get_adv_conf().get_conf():
            ele_adv = et.SubElement(ele_frontend, 'advanced_fe')
            et.SubElement(ele_adv, 'name').text = name
            et.SubElement(ele_adv, 'value').text = val
            et.SubElement(ele_adv, 'comment').text = com

        # create the elements of event builder 
        ele_ebd = et.SubElement(root, 'event_builder')
        # the advanced settings for event builder
        for name,val,com in self.ebd.get_adv_conf().get_conf():
            ele_adv = et.SubElement(ele_ebd, 'advanced_ebd')
            et.SubElement(ele_adv, 'name').text = name
            et.SubElement(ele_adv, 'value').text = val
            et.SubElement(ele_adv, 'comment').text = com

        # create the elements of GUI controller
        ele_ctl = et.SubElement(root, 'GUI_ctl')
        # the advanced settings for GUI controler
        for name,val,com in self.ctl.get_adv_conf().get_conf():
            ele_adv = et.SubElement(ele_ctl, 'advanced_ctl')
            et.SubElement(ele_adv, 'name').text = name
            et.SubElement(ele_adv, 'value').text = val
            et.SubElement(ele_adv, 'comment').text = com

        # create the elements of logger
        ele_log = et.SubElement(root, 'logger')
        # the advanced settings for logger
        for name,val,com in self.log.get_adv_conf().get_conf():
            ele_adv = et.SubElement(ele_log, 'advanced_log')
            et.SubElement(ele_adv, 'name').text = name
            et.SubElement(ele_adv, 'value').text = val
            et.SubElement(ele_adv, 'comment').text = com

        # create the elements of analyzer
        ele_ana = et.SubElement(root, 'analyzer')
        # the advanced settings for analyzer
        for name,val,com in self.ana.get_adv_conf().get_conf():
            ele_adv = et.SubElement(ele_ana, 'advanced_ana')
            et.SubElement(ele_adv, 'name').text = name
            et.SubElement(ele_adv, 'value').text = val
            et.SubElement(ele_adv, 'comment').text = com
        # the histograms of the analyzer
        for h in self.ana.get_ana_hist().get_all_hists():
            xid = h[0]
            xtype = h[1]
            name = h[2]
            folder = h[3]
            nbinsX = h[4]
            X_min = h[5]
            X_max = h[6]
            nbinsY = h[7]
            Y_min = h[8]
            Y_max = h[9]
            xdel = h[10]
            if xdel:
                continue
            ele_hist = et.SubElement(ele_ana, 'hist_ana')
            if xtype == 'TH1D':
                nbinsY = 0
                Y_min = 0
                Y_max = 0
            et.SubElement(ele_hist, 'id').text = str(xid)
            et.SubElement(ele_hist, 'type').text = xtype
            et.SubElement(ele_hist, 'name').text = name
            et.SubElement(ele_hist, 'folder').text = folder
            et.SubElement(ele_hist, 'nbinsX').text = str(nbinsX)
            et.SubElement(ele_hist, 'nbinsY').text = str(nbinsY)
            et.SubElement(ele_hist, 'X_min').text = str(X_min)
            et.SubElement(ele_hist, 'Y_min').text = str(Y_min)
            et.SubElement(ele_hist, 'X_max').text = str(X_max)
            et.SubElement(ele_hist, 'Y_max').text = str(Y_max)



        ####################################

        # save the config
        try:
            reparsed = minidom.parseString(et.tostring(root, 'utf-8'))
            tmp = reparsed.toprettyxml(indent="    ")
            with open(self.f_name, 'w') as f:
                f.write(tmp)
            messagebox.showinfo('info', 'saved in file: ' + self.f_name, 
                    parent=self.root_win)
        except:
            messagebox.showerror('error', 'cannot save to file: ' +
                    self.f_name, parent=self.root_win)


    # create link
    def _link(self):
        if self.f_name:
            os.system('ln -sf ' + self.f_name +' ../control/conf.xml')
            os.system('ln -sf ' + self.f_name +' ../../config.xml')
            messagebox.showinfo('info', 'link created!', parent=self.root_win)
        else:
            messagebox.showerror('error', 'please save file first',
                    parent=self.root_win)


    # save the config file
    def _save(self):
        if self.f_name:
            self._do_save()
        else:
            self._saveas()
    def _saveas(self):
        tmp = filedialog.asksaveasfilename(title = "Save As",
                filetypes = (('xml files','*.xml'),("all files","*.*")))
        if tmp:
            self.f_name = tmp
            self._do_save()

    # open a config file
    def _open(self):
        tmp = filedialog.askopenfilename(title='open', 
                filetypes = (('xml files','*.xml'),("all files","*.*")))
        if not tmp:
            return
        
        self.f_name = tmp
        try:
            self._parse_file()
        except:
            messagebox.showerror('error', 'cannot parse file!',
                    parent=self.root_win)
        # allow only open file once, or else you will confuse me!
        self.butt_open.config(state=tk.DISABLED)

    # parse a config file and sets the variables accrodingly
    def _parse_file(self):
        root = et.parse(self.f_name).getroot()
        # parse the frontend
        fe = root.find('frontend')
        if fe:
            self._parse_fe(fe)
        # parse event builder 
        ebd = root.find('event_builder')
        if ebd:
            self._parse_ebd(ebd)
        # parse GUI controler
        ctl = root.find('GUI_ctl')
        if ctl:
            self._parse_ctl(ctl)
        # parse logger
        log = root.find('logger')
        if log:
            self._parse_log(log)

        # parse analyzer
        ana = root.find('analyzer')
        if ana:
            self._parse_ana(ana)

        ####################

    # parse the GUI controller part of the cofnig file
    def _parse_ctl(self, ctl):
        # the advance settings (if any)
        conf = []
        for adv_conf in ctl.findall('advanced_ctl'):
            name = adv_conf.find('name').text
            val  = adv_conf.find('value').text
            conf.append((name, val, ''))
        self.ctl.get_adv_conf().set_conf(conf)

    # parse the event builder part of the cofnig file
    def _parse_ebd(self, ebd):
        # the advance settings (if any)
        conf = []
        for adv_conf in ebd.findall('advanced_ebd'):
            name = adv_conf.find('name').text
            val  = adv_conf.find('value').text
            conf.append((name, val, ''))
        self.ebd.get_adv_conf().set_conf(conf)

    # parse the analyzer part of the cofnig file
    def _parse_ana(self, ana):
        # the advance settings (if any)
        conf = []
        for adv_conf in ana.findall('advanced_ana'):
            name = adv_conf.find('name').text
            val  = adv_conf.find('value').text
            conf.append((name, val, ''))
        self.ana.get_adv_conf().set_conf(conf)
        # the histograms if any
        hist = []
        for h in ana.findall('hist_ana'):
            xid = int(h.find('id').text)
            xtype = h.find('type').text
            name = h.find('name').text
            folder = h.find('folder').text
            nbinsX = int(h.find('nbinsX').text)
            nbinsY = int(h.find('nbinsY').text)
            X_min = float(h.find('X_min').text)
            Y_min = float(h.find('Y_min').text)
            X_max = float(h.find('X_max').text)
            Y_max = float(h.find('Y_max').text)
            hist.append((xid, xtype, name, folder, nbinsX, X_min, X_max,
                nbinsY, Y_min, Y_max, False))
        self.ana.get_ana_hist().set_hist(hist)


    # parse the logger part of the cofnig file
    def _parse_log(self, log):
        # the advance settings (if any)
        conf = []
        for adv_conf in log.findall('advanced_log'):
            name = adv_conf.find('name').text
            val  = adv_conf.find('value').text
            conf.append((name, val, ''))
        self.log.get_adv_conf().set_conf(conf)
    # parse the frontend part of the cofnig file
    def _parse_fe(self, fe):
        # the module settings
        for mod in fe.findall('vme_module'):
            glo = mod.findall('global_var')
            reg = mod.findall('register')
            adv = mod.findall('advance_var')
            name = self._find_name(glo)
            crate_n = int(self._find_crate_n(glo)) + 1
            # debug ...
            #print('n_crate: %d' % crate_n)
            #############
            if self.frontend.get_crate_n() < crate_n:
                self.frontend.set_crate_n(crate_n)
            tmp = self.frontend.create_mod(name)
            conf = self._get_mod_conf(glo, reg, adv)
            msg = tmp.set_conf(conf, True)
            if msg:
                messagebox.showerror('error', msg, parent=self.root_win)
            self.frontend.update_win()
        # the advance settings (if any)
        conf = []
        for adv_conf in fe.findall('advanced_fe'):
            name = adv_conf.find('name').text
            val  = adv_conf.find('value').text
            conf.append((name, val, ''))
        self.frontend.get_adv_conf().set_conf(conf)


    # compose the config package:
    def _get_mod_conf(self, glo, reg, adv):
        conf = []
        for var in glo:
            name = var.find('name').text
            val = var.find('value').text
            conf.append(('glo_'+name, val, ''))
        for r in reg:
            off = r.find('offset').text
            val = r.find('value').text
            conf.append(('reg_'+off, val, ''))
        for a in adv:
            name = a.find('name').text
            val = a.find('value').text
            conf.append(('adv_'+name, val, ''))
        return conf



    # find the name from the (many) global parameters
    def _find_name(self, glo):
        for var in glo:
            name = var.find('name').text
            if name == 'name':
                return var.find('value').text
    # find the crate n from the (many) global parameters
    def _find_crate_n(self, glo):
        for var in glo:
            name = var.find('name').text
            if name == 'crate_n':
                return var.find('value').text
                

    def run(self):
        self.root_win.mainloop()

app = config()
app.run()
