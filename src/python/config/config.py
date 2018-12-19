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
from ctl import ctl
import xml.etree.ElementTree as et
from xml.dom import minidom


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
        self.butt_frm.place(x=0, y=0, width=250, height=25)
        self.butt_open = tk.Button(self.butt_frm, text='open...',
                command=self._open)
        self.butt_open.pack(side=tk.LEFT)
        self.butt_saveas = tk.Button(self.butt_frm, text='save as...',
                command=self._saveas)
        self.butt_saveas.pack(side=tk.LEFT)
        self.butt_save = tk.Button(self.butt_frm, text='save',
                command=self._save)
        self.butt_save.pack(side=tk.LEFT)

    # create the notebook widget
    def _create_notebook(self):
        self.notebook_frm = tk.Frame(self.root_win)
        self.notebook_frm.place(x=0, y=30, width=800, height=600)
        self.tab_ctrl = ttk.Notebook(self.notebook_frm)

        # create the frontend tab
        self._create_frontend_tab(self.tab_ctrl)

        # create the event builder tab
        self._create_evt_builder(self.tab_ctrl)
        
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
    
    # parse the frontend part of the cofnig file
    def _parse_fe(self, fe):
        # the module settings
        for mod in fe.findall('vme_module'):
            glo = mod.findall('global_var')
            reg = mod.findall('register')
            adv = mod.findall('advance_var')
            name = self._find_name(glo)
            tmp = self.frontend.create_mod(name)
            conf = self._get_mod_conf(glo, reg, adv)
            msg = tmp.set_conf(conf)
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
                

    def run(self):
        self.root_win.mainloop()

app = config()
app.run()
