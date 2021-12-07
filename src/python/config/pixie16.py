#!/bin/python
# -*- coding: UTF-8 -*-
__metaclass__ = type


 # This is the pixie16 module class (modified based on the vme_mod.py). The
 # pixie16 module object maintains all configuration data for the module. The
 # pixie16 module object also manages a top-level window (by calling the
 # show_win method to shown the window) allowing the interaction with user to
 # modify/view its configurations.
 # Similarly as the vme_mod, I also use the variable 'reg_map' for the centrol
 # data structure of the class. It is a list, each element is a dict which has
 # the following structure. (note, the elements in this dict doesn't have to be
 # a physical register of the module, it can be any setting parameter with uniq
 # offset. In the case of pixie16 module, most, if not all, of the setting
 # parameters are not physical registers.):
 #  {'off':offset, ---> offset of the register
 #   'value':value, ---> value of the registter, which is a 16-length array
 #   'name':name, --->name of the register (will be used as the text of the
 #                    label in the settings area if this register has one
 #                    corresponding widget in the settings area)
 #   'comment':'comment' ---> Comments (explaination) about this register. 
 #  }
 # 
 # The GUI interface is made as similar as possible to that of Wang Jianguo's
 # DAQ, so that user can use my DAQ without much training.
 # By B.Gao (Nov. 2021)

import tkinter as tk
from tkinter import messagebox
from tkinter import ttk
from adv_conf_mod import adv_conf_mod
import copy
import math

# the config_* parameters are for 100 MHz, 14bit, Rev F modules. For other
# ones, one should check and modify those values accordingly
config_adc_bits                           = 14
config_adc_msps                           = 100
config_adc_clk_div                        = 1
config_fpga_clk_mhz                       = 100
config_fifo_length                        = 16380

hw_limit_CFDDELAY_MAX                     = 63
hw_limit_CFDDELAY_MIN                     = 1
hw_limit_CFDSCALE_MAX                     = 7
hw_limit_CFDTHRESH_MAX                    = 65535
hw_limit_CFDTHRESH_MIN                    = 1
hw_limit_CHANTRIGSTRETCH_MAX              = 4095
hw_limit_CHANTRIGSTRETCH_MIN              = 1
hw_limit_DAC_VOLTAGE_RANGE                = 3
hw_limit_DSP_CLOCK_MHZ                    = 100
hw_limit_EXTDELAYLEN_MAX_REVBCD           = 255
hw_limit_EXTDELAYLEN_MAX_REVF             = 511
hw_limit_EXTDELAYLEN_MIN                  = 0
hw_limit_EXTTRIGSTRETCH_MAX               = 4095
hw_limit_EXTTRIGSTRETCH_MIN               = 1
hw_limit_FASTFILTERRANGE_MAX              = 0
hw_limit_FASTFILTERRANGE_MIN              = 0
hw_limit_FASTFILTER_MAX_LEN               = 127
hw_limit_FASTLENGTH_MIN_LEN               = 2
hw_limit_FASTTRIGBACKLEN_MAX              = 4095
hw_limit_FASTTRIGBACKDELAY_MIN            = 0
hw_limit_FASTTRIGBACKLEN_MIN_100MHZFIPCLK = 1
hw_limit_FASTTRIGBACKLEN_MIN_125MHZFIPCLK = 2
hw_limit_FAST_THRESHOLD_MAX               = 65535
hw_limit_QDCLEN_MAX                       = 32767
hw_limit_QDCLEN_MIN                       = 1
hw_limit_SLOWFILTERRANGE_MAX              = 6
hw_limit_SLOWFILTERRANGE_MIN              = 1
hw_limit_SLOWFILTER_MAX_LEN               = 127
hw_limit_SLOWGAP_MIN_LEN                  = 3
hw_limit_SLOWLENGTH_MIN_LEN               = 2
hw_limit_TRACEDELAY_MAX                   = 1023
hw_limit_VETOSTRETCH_MAX                  = 4095
hw_limit_VETOSTRETCH_MIN                  = 1




class pixie16:
    def __init__(self, name, mod=None):
        self._init_reg_map()
        self.adv_conf = adv_conf_mod()

        if mod:
            # init the common attributes. For the meanings of these attributes,
            # see module.h
            self.crate = mod.get_crate()
            self.slot =  mod.get_slot()
            self.base =  mod.get_base()
            self.am_reg = mod.get_am()
            self.type = mod.get_type() # type is 'T' (trigger) or 'S' (scaler)
            self.period = mod.get_period() 
            self.max_crate = mod.get_max_crate() # max number of crate
            # name of the module
            self.name = name
            # set values of registers
            regs = mod.get_reg_map()
            for off,val in regs.items():
                self.set_reg(off, val)
            # set non-default values of advanced variables
            self.adv_conf.set_conf(mod.get_adv_conf().get_conf())
        else:
            # default values
            self.crate = 0
            self.slot = 0
            self.base = 0x0
            self.am_reg = 9
            self.type = 'T' 
            self.period = 1000
            self.max_crate = 1 
            self.name = name
        # check if all the keys of each of the elements (dict) in the reg_map
        # list are initialized, initialize to None if not.
        list_of_key = ['off', 'value', 'name', 'comment']
        for reg in self.reg_map:
            for key in list_of_key:
                if not key in reg:
                    reg[key] = None
        # check if all the elements in the reg_map have unique addresses
        # (sometimes if one uses copy-paste to create many elements, one may
        # forget to change some of the fields after pasting)
        for i,reg in enumerate(self.reg_map):
            off = reg['off']
            for j in range(len(self.reg_map)):
                if j <= i:
                    continue
                off1 = self.reg_map[j]['off']
                if off1 == off:
                    self._show_msg('conflicting offsets: 0x%04x in module: %s' % 
                            (off, name), 'error')
        # let's now sort the elements based on their offsets:
        self.reg_map.sort(key=lambda x : x['off'])
        
        self.cur_ch_num = 0

    
    # a list of set and get methods (note: most of those methds are copy-past
    # from vme_mod.py, therefore, are meaningless and not used for the pixie16
    # module here )
    def set_crate(self, n):
        self.crate = n
    def set_slot(self, n):
        self.slot = n
    def set_base(self, base):
        self.base = base
    def set_am(self, am_reg):
        self.am_reg = am_reg
    def set_type(self, t):
        self.type = t
    def set_period(self, t):
        self.period = t
    def set_reg(self, off, val):
        for reg in self.reg_map:
            if reg['off'] == off:
                reg['value'] = val
                break
    def set_max_crate(self, n):
        self.max_crate = n
    def set_name(self, name):
        self.name = name
    def set_mod_num(self, n):
        self.reg_map[0]['value'] = [n for i in range(16)]
        self.set_base(n)
    def get_crate(self):
        return self.crate
    def get_slot(self):
        return self.slot
    def get_base(self):
        return self.base
    def get_am(self):
        return self.am_reg
    def get_type(self):
        return self.type
    def get_period(self):
        return self.period
    def get_reg_map(self):
        tmp = {}
        for reg in self.reg_map:
            tmp[reg['off']] = reg['value']
        return tmp
    def get_adv_conf(self):
        return self.adv_conf
    def get_name(self):
        return self.name
    def get_win(self):
        return self.win
    def get_max_crate(self):
        return self.max_crate
    def get_stat(self):
        return self.stat

    def move_up(self):
        if self.slot > 0:
            self.slot -= 1

    def move_down(self):
        if self.slot < 20:
            self.slot += 1

    # get a list of xml elements. The elements are represented by tuples:
    # xml_ele = ('offset/global_var_name', 'value', 'comment'). if xml_ele[0]
    # starts with 'reg', it represents a register, if it startswith 'glo', it
    # represents a global variable. Each element in reg_map with non-default
    # values should generate an xml element.  The comment is the 'comment' in
    # reg_map element if available, otherwise use 'name'.
    def get_conf(self):
        tmp = []
        # registers 
        for reg in self.reg_map:
            if reg['value'] == 'default':
                continue
            off = 'reg_0x%x' % reg['off']
            value = ['0x%x' % int(reg['value'][i]) for i in range(16)]
            com = reg['name']
            if reg['comment']:
                com = reg['comment']
            tmp.append((off,value,com))
        # global vars
        tmp.append(('glo_crate_n', str(self.crate), 'crate number'))
        tmp.append(('glo_slot_n',  str(self.slot), 'slot number'))
        tmp.append(('glo_base', '0x%08x' % self.base, 'base address'))
        tmp.append(('glo_am_reg', '0x%x' % self.am_reg, 
            'am used to access most of the registers'))
        tmp.append(('glo_type', self.type, 'type of module (Trigg/Scal)'))
        tmp.append(('glo_period',  str(self.period), 
            'period of reading for scaler type modules'))
        tmp.append(('glo_max_crate', str(self.max_crate), 'max crate'))
        tmp.append(('glo_name', self.name, 'name of the module'))

        #advanced conf
        conf = self.adv_conf.get_conf()
        for name,val,com in conf:
            tmp.append(('adv_'+name, val, com))

        return tmp

    # reverse of get_conf
    def set_conf(self, conf, clr_all_reg=False):
        if clr_all_reg:
            # clear the intrinsic non-default values 
            for reg in self.reg_map:
                reg['value'] = 'default'
        for name, val, com in conf:
            try:
                if name.startswith('glo'):
                    name = name[4:]
                    if name == 'crate_n':
                        # crate number is slot number for pixie16
                        self.slot = int(val)
                    elif name == 'slot_n':
                        pass
                        #self.slot = int(val)
                    elif name == 'base':
                        self.base = int(val, 16)
                    elif name == 'am_reg':
                        self.am_reg = int(val, 16)
                    elif name == 'type':
                        self.type = val
                    elif name == 'period':
                        self.period = int(val)
                    elif name == 'max_crate':
                        self.max_crate = int(val)
                    elif name == 'name':
                        self.name = val
                elif name.startswith('reg'):
                    name = name[4:]
                    off = int(name, 16)
                    val1 = [int(val[i], 16) for i in range(16)]
                    self.set_reg(off, val1)
                elif name.startswith('adv'):
                    name = name[4:]
                    self.adv_conf.set_the_conf(name, val)
                else:
                    return 'unknown tag name: ' + name
            except:
                return 'cannot set conf of the module: ' + self.name + \
                        'with tag name: ' + name
        return None

    def get_geo(self):
        return self.slot
        if self.name.find('V2718') >= 0:
            return int(self.name[5:])
        else:
            for reg in self.reg_map:
                if reg['off'] == self.geo_addr:
                    return reg['value']


   
   # This method should be called when the user want to edit/view the
    # configurations of the module
    def show_win(self):
        self._init_layouts()
        #self._init_com()
        #self._init_set()
        #self._init_reg()

    # place the window in propriate posistion
    def _place_win_set(self, win):
        win.grid(row=self.tot_set//self.col_set, 
                column=(self.tot_set%self.col_set))
        self.tot_set += 1
    def _place_win_reg(self, win):
        win.grid(row=self.tot_reg//self.col_reg, 
                column=(self.tot_reg%self.col_reg))
        self.tot_reg += 1

    


    def _create_ModParSet(self, tab_ctrl):
        self.ModParSet = tk.Frame(tab_ctrl)
        tab_ctrl.add(self.ModParSet, text='ModParSet')
        tab_ctrl.pack(fill=tk.BOTH, expand=1)
        
        win = self.ModParSet
        # create the widgets for each register
        # ModNum
        off = 0
        txt = self.reg_map[off]['name']
        val = self.reg_map[off]['value'][0]
        tk.Label(win, text=txt).place(x=5, y=10, width=60, height=25)
        self.ModNum_wid = tk.Entry(win)
        self.ModNum_wid.place(x=70, y=10, width=20, height=25)
        self.ModNum_wid.insert(0, str(val))
        self.ModNum_wid.config(state=tk.DISABLED)
        #self.ModNum_wid.bind('<Return>',self._ModNum_evt)

        # slot number
        txt = 'Slot Num'
        val = self.slot
        tk.Label(win, text=txt).place(x=95, y=10, width=60, height=25)
        slots = [str(i) for i in range(16)]
        self.slot_wid = ttk.Combobox(win, state='readonly', values=slots)
        self.slot_wid.set(slots[val])
        self.slot_wid.place(x=155, y=10, width=35, height=25)
        self.slot_wid.bind("<<ComboboxSelected>>", self._slot_evt) 

        # module revision
        off = 88
        txt = self.reg_map[off]['name']
        val = self.reg_map[off]['value'][0]
        tk.Label(win, text=txt, anchor=tk.W).place(x=200, y=10, width=130,
                height=25)
        t_filters = ['rev_B', 'rev_C', 'rev_D', 'rev_F', 'rev_H']
        self.rev_wid = ttk.Combobox(win, state='readonly', values=t_filters)
        self.rev_wid.set(t_filters[val])
        self.rev_wid.place(x=260, y=10, width=80, height=25)
        self.rev_wid.bind("<<ComboboxSelected>>", self._rev_evt)

        # module parameter sets
        tk.Label(win, bg='yellow', text='ModPar set').place(x=10, y=60,
                width=80, height=25)
        self.ModParSet_mod = tk.Frame(win, height=500, width=160, relief =
                tk.GROOVE, borderwidth = 2)
        self.ModParSet_mod.place(x=10, y=80)
        win = self.ModParSet_mod
        
        # Energy fileter range 
        off = 1
        txt = self.reg_map[off]['name']
        val = self.reg_map[off]['value'][0]
        tk.Label(win, text=txt, anchor=tk.W).place(x=10, y=5, width=130,
                height=25)
        e_filters = [str(i+1) for i in range(6)]
        self.E_Filter_Range_wid = ttk.Combobox(win, state='readonly',
                values=e_filters)
        self.E_Filter_Range_wid.set(e_filters[val])
        self.E_Filter_Range_wid.place(x=10, y=30, width=130, height=25)
        self.E_Filter_Range_wid.bind("<<ComboboxSelected>>",
                self._e_filter_range_evt)
        
        # Fast fileter range 
        off = 2
        txt = self.reg_map[off]['name']
        val = self.reg_map[off]['value'][0]
        tk.Label(win, text=txt, anchor=tk.W).place(x=10, y=60, width=130,
                height=25)
        t_filters = [str(i+1) for i in range(1)]
        self.T_Filter_Range_wid = ttk.Combobox(win, state='readonly',
                values=t_filters)
        self.T_Filter_Range_wid.set(t_filters[val])
        self.T_Filter_Range_wid.place(x=10, y=85, width=130, height=25)
        self.T_Filter_Range_wid.bind("<<ComboboxSelected>>",
                self._t_filter_range_evt)

        #CPLD
        off = 3
        txt = self.reg_map[off]['name']
        val = self.reg_map[off]['value'][0]
        self.CPLD_var = tk.IntVar()
        self.CPLD_var.set(val)
        tk.Checkbutton(win, text=txt, variable=self.CPLD_var, anchor=tk.W,
                command=self._CPLD_evt).place(x=10, y=115, width=130,
                        height=25)
        self.CPLD_var.set(val)

        #SortEvent
        off = 4
        txt = self.reg_map[off]['name']
        val = self.reg_map[off]['value'][0]
        self.SortEvent_var = tk.IntVar()
        self.SortEvent_var.set(val)
        tk.Checkbutton(win, text=txt, variable=self.SortEvent_var, anchor=tk.W,
                command=self._SortEvent_evt).place(x=10, y=140, width=130,
                        height=25)
        self.SortEvent_var.set(val)

        #InhibitEna
        off = 5
        txt = self.reg_map[off]['name']
        val = self.reg_map[off]['value'][0]
        self.InhibitEna_var = tk.IntVar()
        self.InhibitEna_var.set(val)
        tk.Checkbutton(win, text=txt, variable=self.InhibitEna_var,
                anchor=tk.W, command=self._InhibitEna_evt).place(x=10, y=165,
                        width=130, height=25)
        self.InhibitEna_var.set(val)

        #MulCrate Enable
        off = 6
        txt = self.reg_map[off]['name']
        val = self.reg_map[off]['value'][0]
        self.MulCrate_var = tk.IntVar()
        self.MulCrate_var.set(val)
        tk.Checkbutton(win, text=txt, variable=self.MulCrate_var, anchor=tk.W,
                command=self._MulCrate_evt).place(x=10, y=190, width=130,
                        height=25)
        self.MulCrate_var.set(val)

        #XDT (us)
        off = 7
        txt = self.reg_map[off]['name']
        val = self.reg_map[off]['value'][self.cur_ch_num]
        tk.Label(win, text=txt).place(x=10, y=220, width=60, height=25)
        self.XDT_wid = tk.Entry(win)
        self.XDT_wid.place(x=70, y=220, width=80, height=25)
        self.XDT_wid.insert(0, '%.3f'%(val/1000.))
        self.XDT_wid.bind('<Return>',self._XDT_evt)

        # channel parameter sets
        win = self.ModParSet
        tk.Label(win, bg='green', text='ChPar set').place(x=180, y=60,
                width=80, height=25)
        self.ModParSet_ch = tk.Frame(win, height=500, width=600, relief =
                tk.GROOVE, borderwidth = 2)
        self.ModParSet_ch.place(x=180, y=80)
        win = self.ModParSet_ch

        # channel number 
        off = 39
        txt = self.reg_map[off]['name']
        val = self.reg_map[off]['value'][self.cur_ch_num]
        tk.Label(win, text=txt).place(x=10, y=5, width=60, height=25)
        self.ChNum_wid = tk.Entry(win)
        self.ChNum_wid.place(x=70, y=5, width=25, height=25)
        self.ChNum_wid.insert(0, str(val))
        self.ChNum_wid.config(state=tk.DISABLED)
        #self.ChNum_wid.bind('<Return>',self._ChNum_evt)
        # decrease/increase current ch num buttons
        tk.Button(win, text='+', command=self._inc_ch_num,
                bg='gray').place(x=95, y=5, width=20,height=13)
        tk.Button(win, text='-', command=self._dec_ch_num,
                bg='gray').place(x=95, y=18, width=20,height=13)

        # Good Ch
        off = 8
        txt = self.reg_map[off]['name']
        val = self.reg_map[off]['value'][self.cur_ch_num]
        good_ch_val = ['no', 'yes']
        tk.Label(win, text=txt).place(x=120, y=5, width=60, height=25)
        self.GoodCh_wid = ttk.Combobox(win, state='readonly',
                values=good_ch_val)
        self.GoodCh_wid.set(good_ch_val[val])
        self.GoodCh_wid.place(x=180, y=5, width=40, height=25)
        self.GoodCh_wid.bind("<<ComboboxSelected>>", self._good_ch_evt)

        # Copy Ch pars to/from
        tk.Label(win, text='Copy ChPars to').place(x=230, y=5, width=100,
                height=25)
        cp_ch_val = ['ch'+str(i) for i in range(16)]
        cp_ch_val.append('all ch')
        self.cp_par_to_wid = ttk.Combobox(win, state='readonly',
                values=cp_ch_val)
        self.cp_par_to_wid.set(cp_ch_val[0])
        self.cp_par_to_wid.place(x=330, y=5, width=50, height=25)
        self.cp_par_to_wid.bind("<<ComboboxSelected>>", self._cp_par_to_evt)
        tk.Label(win, text='Copy ChPars from').place(x=400, y=5, width=120,
                height=25)
        cp_ch_val = ['ch'+str(i) for i in range(16)]
        self.cp_par_from_wid = ttk.Combobox(win, state='readonly',
                values=cp_ch_val)
        self.cp_par_from_wid.set(cp_ch_val[0])
        self.cp_par_from_wid.place(x=520, y=5, width=50, height=25)
        self.cp_par_from_wid.bind("<<ComboboxSelected>>", self._cp_par_from_evt)

        # POL
        off = 9
        txt = self.reg_map[off]['name']
        val = self.reg_map[off]['value'][self.cur_ch_num]
        pol_val = ['Neg', 'Pos']
        tk.Label(win, anchor=tk.W, text=txt).place(x=10, y=45, width=100,
                height=25)
        #tk.Label(win, text='E_Risetime(us)').place(x=10, y=35, width=100, height=25)
        self.pol_wid = ttk.Combobox(win, state='readonly',
                values=pol_val)
        self.pol_wid.set(pol_val[val])
        #self.pol_wid.set('Reject pileup')
        self.pol_wid.place(x=110, y=45, width=100, height=25)
        self.pol_wid.bind("<<ComboboxSelected>>", self._pol_evt)

        # Gain
        off = 10
        txt = self.reg_map[off]['name']
        val = self.reg_map[off]['value'][self.cur_ch_num]
        gain_val = ['0.25', '1']
        tk.Label(win, anchor=tk.W, text=txt).place(x=10, y=75, width=100,
                height=25)
        self.Gain_wid = ttk.Combobox(win, state='readonly',
                values=gain_val)
        self.Gain_wid.set(gain_val[val])
        self.Gain_wid.place(x=110, y=75, width=100, height=25)
        self.Gain_wid.bind("<<ComboboxSelected>>", self._gain_evt)

        # Offset (%)
        off = 11
        txt = self.reg_map[off]['name']
        val = self.reg_map[off]['value'][self.cur_ch_num]
        tk.Label(win, text=txt, anchor=tk.W).place(x=10, y=105, width=100,
                height=25)
        self.offset_wid = tk.Entry(win)
        self.offset_wid.place(x=110, y=105, width=100, height=25)
        self.offset_wid.insert(0, str(val))
        self.offset_wid.bind('<Return>',self._offset_evt)

        # T_Risetime(us)
        off = 12
        txt = self.reg_map[off]['name']
        val = self.reg_map[off]['value'][self.cur_ch_num]
        tk.Label(win, text=txt, anchor=tk.W).place(x=10, y=135, width=100,
                height=25)
        self.T_Risetime_wid = tk.Entry(win)
        self.T_Risetime_wid.place(x=110, y=135, width=100, height=25)
        self.T_Risetime_wid.insert(0, '%.3f'%(val/1000.))
        self.T_Risetime_wid.bind('<Return>',self._t_risetime_evt)

        # T_FlatTop(us)
        off = 13
        txt = self.reg_map[off]['name']
        val = self.reg_map[off]['value'][self.cur_ch_num]
        tk.Label(win, text=txt, anchor=tk.W).place(x=10, y=165, width=100,
                height=25)
        self.T_FlatTop_wid = tk.Entry(win)
        self.T_FlatTop_wid.place(x=110, y=165, width=100, height=25)
        self.T_FlatTop_wid.insert(0, '%.3f'%(val/1000.))
        self.T_FlatTop_wid.bind('<Return>',self._t_flattop_evt)

        # T_Thresh
        off = 14
        txt = self.reg_map[off]['name']
        val = self.reg_map[off]['value'][self.cur_ch_num]
        tk.Label(win, text=txt, anchor=tk.W).place(x=10, y=195, width=100,
                height=25)
        self.T_Thresh_wid = tk.Entry(win)
        self.T_Thresh_wid.place(x=110, y=195, width=100, height=25)
        self.T_Thresh_wid.insert(0, str(val))
        self.T_Thresh_wid.bind('<Return>',self._t_thresh_evt)

        # E_Risetime(us)
        off = 15
        txt = self.reg_map[off]['name']
        val = self.reg_map[off]['value'][self.cur_ch_num]
        tk.Label(win, text=txt, anchor=tk.W).place(x=10, y=225, width=100,
                height=25)
        self.E_Risetime_wid = tk.Entry(win)
        self.E_Risetime_wid.place(x=110, y=225, width=100, height=25)
        self.E_Risetime_wid.insert(0, '%.3f'%(val/1000.))
        self.E_Risetime_wid.bind('<Return>',self._e_risetime_evt)

        # E_FlatTop(us)
        off = 16
        txt = self.reg_map[off]['name']
        val = self.reg_map[off]['value'][self.cur_ch_num]
        tk.Label(win, text=txt, anchor=tk.W).place(x=10, y=255, width=100,
                height=25)
        self.E_FlatTop_wid = tk.Entry(win)
        self.E_FlatTop_wid.place(x=110, y=255, width=100, height=25)
        self.E_FlatTop_wid.insert(0, '%.3f'%(val/1000.))
        self.E_FlatTop_wid.bind('<Return>',self._e_flattop_evt)

        # Tau(us)
        off = 17
        txt = self.reg_map[off]['name']
        val = self.reg_map[off]['value'][self.cur_ch_num]
        tk.Label(win, text=txt, anchor=tk.W).place(x=10, y=285, width=100,
                height=25)
        self.Tau_wid = tk.Entry(win)
        self.Tau_wid.place(x=110, y=285, width=100, height=25)
        self.Tau_wid.insert(0, '%.3f'%(val/1000.))
        self.Tau_wid.bind('<Return>',self._Tau_evt)

        # PileUp
        off = 18
        txt = self.reg_map[off]['name']
        val = self.reg_map[off]['value'][self.cur_ch_num]
        pileup_val = ['AcceptAll', 'RejectPileUp', 'TracePileUp', 'RejectSingle']
        tk.Label(win, anchor=tk.W, text=txt).place(x=10, y=315, width=100,
                height=25)
        self.PileUp_wid = ttk.Combobox(win, state='readonly',
                values=pileup_val)
        self.PileUp_wid.set(pileup_val[val])
        self.PileUp_wid.place(x=110, y=315, width=100, height=25)
        self.PileUp_wid.bind("<<ComboboxSelected>>", self._pileup_evt)

        # SyncData
        off = 19
        txt = self.reg_map[off]['name']
        val = self.reg_map[off]['value'][self.cur_ch_num]
        good_ch_val = ['no', 'yes']
        #txt='Trace_PreTrig(us)'
        tk.Label(win, text=txt, anchor=tk.W).place(x=220, y=45, width=110,
                height=25)
        self.SyncData_wid = ttk.Combobox(win, state='readonly',
                values=good_ch_val)
        self.SyncData_wid.set(good_ch_val[val])
        self.SyncData_wid.place(x=330, y=45, width=60, height=25)
        self.SyncData_wid.bind("<<ComboboxSelected>>", self._syncdata_evt)

        # EnExtTS
        off = 20
        txt = self.reg_map[off]['name']
        val = self.reg_map[off]['value'][self.cur_ch_num]
        good_ch_val = ['no', 'yes']
        #txt='Trace_PreTrig(us)'
        tk.Label(win, text=txt, anchor=tk.W).place(x=220, y=75, width=110,
                height=25)
        self.EnExtTS_wid = ttk.Combobox(win, state='readonly',
                values=good_ch_val)
        self.EnExtTS_wid.set(good_ch_val[val])
        self.EnExtTS_wid.place(x=330, y=75, width=60, height=25)
        self.EnExtTS_wid.bind("<<ComboboxSelected>>", self._enextts_evt)

        # ESumEna
        off = 21
        txt = self.reg_map[off]['name']
        val = self.reg_map[off]['value'][self.cur_ch_num]
        good_ch_val = ['no', 'yes']
        #txt='Trace_PreTrig(us)'
        tk.Label(win, text=txt, anchor=tk.W).place(x=220, y=105, width=110,
                height=25)
        self.ESumEna_wid = ttk.Combobox(win, state='readonly',
                values=good_ch_val)
        self.ESumEna_wid.set(good_ch_val[val])
        self.ESumEna_wid.place(x=330, y=105, width=60, height=25)
        self.ESumEna_wid.bind("<<ComboboxSelected>>", self._esumena_evt)

        # CFDEna
        off = 22
        txt = self.reg_map[off]['name']
        val = self.reg_map[off]['value'][self.cur_ch_num]
        good_ch_val = ['no', 'yes']
        #txt='Trace_PreTrig(us)'
        tk.Label(win, text=txt, anchor=tk.W).place(x=220, y=135, width=110,
                height=25)
        self.CFDEna_wid = ttk.Combobox(win, state='readonly',
                values=good_ch_val)
        self.CFDEna_wid.set(good_ch_val[val])
        self.CFDEna_wid.place(x=330, y=135, width=60, height=25)
        self.CFDEna_wid.bind("<<ComboboxSelected>>", self._cfdena_evt)

        # CFD_Scaler
        off = 23
        txt = self.reg_map[off]['name']
        val = self.reg_map[off]['value'][self.cur_ch_num]
        good_ch_val = ['%d/8'%(8-i) for i in range(8)]
        #txt='Trace_PreTrig(us)'
        tk.Label(win, text=txt, anchor=tk.W).place(x=220, y=165, width=110,
                height=25)
        self.CFD_Scaler_wid = ttk.Combobox(win, state='readonly',
                values=good_ch_val)
        self.CFD_Scaler_wid.set(good_ch_val[val])
        self.CFD_Scaler_wid.place(x=330, y=165, width=60, height=25)
        self.CFD_Scaler_wid.bind("<<ComboboxSelected>>", self._cfdscaler_evt)

        # CFD_Dealy(us)
        off = 24
        txt = self.reg_map[off]['name']
        val = self.reg_map[off]['value'][self.cur_ch_num]
        tk.Label(win, text=txt, anchor=tk.W).place(x=220, y=195, width=110,
                height=25)
        self.CFD_Dealy_wid = tk.Entry(win)
        self.CFD_Dealy_wid.place(x=330, y=195, width=60, height=25)
        self.CFD_Dealy_wid.insert(0, '%.3f'%(val/1000.))
        self.CFD_Dealy_wid.bind('<Return>',self._cfd_delay_evt)

        # CFD_Threshol
        off = 25
        txt = self.reg_map[off]['name']
        val = self.reg_map[off]['value'][self.cur_ch_num]
        tk.Label(win, text=txt, anchor=tk.W).place(x=220, y=225, width=110,
                height=25)
        self.CFD_Threshol_wid = tk.Entry(win)
        self.CFD_Threshol_wid.place(x=330, y=225, width=60, height=25)
        self.CFD_Threshol_wid.insert(0, str(val))
        self.CFD_Threshol_wid.bind('<Return>',self._cfd_threshol_evt)

        # RecordTrace
        off = 26
        txt = self.reg_map[off]['name']
        val = self.reg_map[off]['value'][self.cur_ch_num]
        good_ch_val = ['no', 'yes']
        #txt='Trace_PreTrig(us)'
        tk.Label(win, text=txt, anchor=tk.W).place(x=220, y=255, width=110,
                height=25)
        self.RecordTrace_wid = ttk.Combobox(win, state='readonly',
                values=good_ch_val)
        self.RecordTrace_wid.set(good_ch_val[val])
        self.RecordTrace_wid.place(x=330, y=255, width=60, height=25)
        self.RecordTrace_wid.bind("<<ComboboxSelected>>", self._RecordTrace_evt)

        # Trace_PreTrig(us)
        off = 27
        txt = self.reg_map[off]['name']
        val = self.reg_map[off]['value'][self.cur_ch_num]
        tk.Label(win, text=txt, anchor=tk.W).place(x=220, y=285, width=110,
                height=25)
        self.Trace_PreTrig_wid = tk.Entry(win)
        self.Trace_PreTrig_wid.place(x=330, y=285, width=60, height=25)
        self.Trace_PreTrig_wid.insert(0, '%.3f'%(val/1000.))
        self.Trace_PreTrig_wid.bind('<Return>',self._trace_pretrig_evt)

        # Trace_Length(us)
        off = 28
        txt = self.reg_map[off]['name']
        val = self.reg_map[off]['value'][self.cur_ch_num]
        tk.Label(win, text=txt, anchor=tk.W).place(x=220, y=315, width=110,
                height=25)
        self.Trace_Length_wid = tk.Entry(win)
        self.Trace_Length_wid.place(x=330, y=315, width=60, height=25)
        self.Trace_Length_wid.insert(0, '%.3f'%(val/1000.))
        self.Trace_Length_wid.bind('<Return>',self._trace_length_evt)

        # RecordQDC
        off = 29
        txt = self.reg_map[off]['name']
        val = self.reg_map[off]['value'][self.cur_ch_num]
        good_ch_val = ['no', 'yes']
        #txt='Trace_PreTrig(us)'
        tk.Label(win, text=txt, anchor=tk.W).place(x=410, y=45, width=100,
                height=25)
        self.RecordQDC_wid = ttk.Combobox(win, state='readonly',
                values=good_ch_val)
        self.RecordQDC_wid.set(good_ch_val[val])
        self.RecordQDC_wid.place(x=500, y=45, width=60, height=25)
        self.RecordQDC_wid.bind("<<ComboboxSelected>>", self._recordqdc_evt)

        # QDC Len0(us)
        off = 30
        txt = self.reg_map[off]['name']
        val = self.reg_map[off]['value'][self.cur_ch_num]
        tk.Label(win, text=txt, anchor=tk.W).place(x=410, y=75, width=100,
                height=25)
        self.QDC_Len0_wid = tk.Entry(win)
        self.QDC_Len0_wid.place(x=500, y=75, width=60, height=25)
        self.QDC_Len0_wid.insert(0, '%.3f'%(val/1000.))
        self.QDC_Len0_wid.bind('<Return>',self._qdclen0_length_evt)

        # QDC Len1(us)
        off = 31
        txt = self.reg_map[off]['name']
        val = self.reg_map[off]['value'][self.cur_ch_num]
        tk.Label(win, text=txt, anchor=tk.W).place(x=410, y=105, width=100,
                height=25)
        self.QDC_Len1_wid = tk.Entry(win)
        self.QDC_Len1_wid.place(x=500, y=105, width=60, height=25)
        self.QDC_Len1_wid.insert(0, '%.3f'%(val/1000.))
        self.QDC_Len1_wid.bind('<Return>',self._qdclen1_length_evt)

        # QDC Len2(us)
        off = 32
        txt = self.reg_map[off]['name']
        val = self.reg_map[off]['value'][self.cur_ch_num]
        tk.Label(win, text=txt, anchor=tk.W).place(x=410, y=135, width=100,
                height=25)
        self.QDC_Len2_wid = tk.Entry(win)
        self.QDC_Len2_wid.place(x=500, y=135, width=60, height=25)
        self.QDC_Len2_wid.insert(0, '%.3f'%(val/1000.))
        self.QDC_Len2_wid.bind('<Return>',self._qdclen2_length_evt)

        # QDC Len3(us)
        off = 33
        txt = self.reg_map[off]['name']
        val = self.reg_map[off]['value'][self.cur_ch_num]
        tk.Label(win, text=txt, anchor=tk.W).place(x=410, y=165, width=100,
                height=25)
        self.QDC_Len3_wid = tk.Entry(win)
        self.QDC_Len3_wid.place(x=500, y=165, width=60, height=25)
        self.QDC_Len3_wid.insert(0, '%.3f'%(val/1000.))
        self.QDC_Len3_wid.bind('<Return>',self._qdclen3_length_evt)

        # QDC Len4(us)
        off = 34
        txt = self.reg_map[off]['name']
        val = self.reg_map[off]['value'][self.cur_ch_num]
        tk.Label(win, text=txt, anchor=tk.W).place(x=410, y=195, width=100,
                height=25)
        self.QDC_Len4_wid = tk.Entry(win)
        self.QDC_Len4_wid.place(x=500, y=195, width=60, height=25)
        self.QDC_Len4_wid.insert(0, '%.3f'%(val/1000.))
        self.QDC_Len4_wid.bind('<Return>',self._qdclen4_length_evt)

        # QDC Len5(us)
        off = 35
        txt = self.reg_map[off]['name']
        val = self.reg_map[off]['value'][self.cur_ch_num]
        tk.Label(win, text=txt, anchor=tk.W).place(x=410, y=225, width=100,
                height=25)
        self.QDC_Len5_wid = tk.Entry(win)
        self.QDC_Len5_wid.place(x=500, y=225, width=60, height=25)
        self.QDC_Len5_wid.insert(0, '%.3f'%(val/1000.))
        self.QDC_Len5_wid.bind('<Return>',self._qdclen5_length_evt)

        # QDC Len6(us)
        off = 36
        txt = self.reg_map[off]['name']
        val = self.reg_map[off]['value'][self.cur_ch_num]
        tk.Label(win, text=txt, anchor=tk.W).place(x=410, y=255, width=100,
                height=25)
        self.QDC_Len6_wid = tk.Entry(win)
        self.QDC_Len6_wid.place(x=500, y=255, width=60, height=25)
        self.QDC_Len6_wid.insert(0, '%.3f'%(val/1000.))
        self.QDC_Len6_wid.bind('<Return>',self._qdclen6_length_evt)

        # QDC Len7(us)
        off = 37
        txt = self.reg_map[off]['name']
        val = self.reg_map[off]['value'][self.cur_ch_num]
        tk.Label(win, text=txt, anchor=tk.W).place(x=410, y=285, width=100,
                height=25)
        self.QDC_Len7_wid = tk.Entry(win)
        self.QDC_Len7_wid.place(x=500, y=285, width=60, height=25)
        self.QDC_Len7_wid.insert(0, '%.3f'%(val/1000.))
        self.QDC_Len7_wid.bind('<Return>',self._qdclen7_length_evt)

        # BLCUT
        off = 38
        txt = self.reg_map[off]['name']
        val = self.reg_map[off]['value'][self.cur_ch_num]
        tk.Label(win, text=txt, anchor=tk.W).place(x=410, y=315, width=100,
                height=25)
        self.BLCUT_wid = tk.Entry(win)
        self.BLCUT_wid.place(x=500, y=315, width=60, height=25)
        self.BLCUT_wid.insert(0, str(val))
        self.BLCUT_wid.bind('<Return>',self._blcut_evt)

        # AutoBLCUT 
        off = 87
        txt = self.reg_map[off]['name']
        val = self.reg_map[off]['value'][self.cur_ch_num]
        self.auto_blcut_var = tk.IntVar()
        self.auto_blcut_var.set(val)
        tk.Checkbutton(win, text=txt, variable=self.auto_blcut_var,
                anchor=tk.W, command=self._auto_blcut_evt).place(x=400, y=315,
                        width=100, height=25)

    def _auto_blcut_evt(self):
        off = 87
        self.reg_map[off]['value'][self.cur_ch_num] = self.auto_blcut_var.get()
        print('new %s selected: ' % self.reg_map[off]['name'])
        print(self.reg_map[off]['value'])




    def _blcut_evt(self, e):
        off = 38
        val = self._get_rounded_val(self.reg_map[off]['name'],
                int(self.BLCUT_wid.get()))
        self.reg_map[off]['value'][self.cur_ch_num] = val
        self.BLCUT_wid.delete(0, tk.END)
        self.BLCUT_wid.insert(0, str(val))
        print('new %s selected: ' % self.reg_map[off]['name'])
        print(self.reg_map[off]['value'])



    def _qdclen1_length_evt(self, e):
        off = 31
        val = self._get_rounded_val(self.reg_map[off]['name'],
                float(self.QDC_Len1_wid.get()))
        val = int(val*1000)
        self.reg_map[off]['value'][self.cur_ch_num] = val
        self.QDC_Len1_wid.delete(0, tk.END)
        self.QDC_Len1_wid.insert(0, '%.3f'%(val/1000.))
        print('new %s selected: ' % self.reg_map[off]['name'])
        print(self.reg_map[off]['value'])


    def _qdclen2_length_evt(self, e):
        off = 32
        val = self._get_rounded_val(self.reg_map[off]['name'],
                float(self.QDC_Len2_wid.get()))
        val = int(val*1000)
        self.reg_map[off]['value'][self.cur_ch_num] = val
        self.QDC_Len2_wid.delete(0, tk.END)
        self.QDC_Len2_wid.insert(0, '%.3f'%(val/1000.))
        print('new %s selected: ' % self.reg_map[off]['name'])
        print(self.reg_map[off]['value'])


    def _qdclen3_length_evt(self, e):
        off = 33
        val = self._get_rounded_val(self.reg_map[off]['name'],
                float(self.QDC_Len3_wid.get()))
        val = int(val*1000)
        self.reg_map[off]['value'][self.cur_ch_num] = val
        self.QDC_Len3_wid.delete(0, tk.END)
        self.QDC_Len3_wid.insert(0, '%.3f'%(val/1000.))
        print('new %s selected: ' % self.reg_map[off]['name'])
        print(self.reg_map[off]['value'])


    def _qdclen4_length_evt(self, e):
        off = 34
        val = self._get_rounded_val(self.reg_map[off]['name'],
                float(self.QDC_Len4_wid.get()))
        val = int(val*1000)
        self.reg_map[off]['value'][self.cur_ch_num] = val
        self.QDC_Len4_wid.delete(0, tk.END)
        self.QDC_Len4_wid.insert(0, '%.3f'%(val/1000.))
        print('new %s selected: ' % self.reg_map[off]['name'])
        print(self.reg_map[off]['value'])


    def _qdclen5_length_evt(self, e):
        off = 35
        val = self._get_rounded_val(self.reg_map[off]['name'],
                float(self.QDC_Len5_wid.get()))
        val = int(val*1000)
        self.reg_map[off]['value'][self.cur_ch_num] = val
        self.QDC_Len5_wid.delete(0, tk.END)
        self.QDC_Len5_wid.insert(0, '%.3f'%(val/1000.))
        print('new %s selected: ' % self.reg_map[off]['name'])
        print(self.reg_map[off]['value'])


    def _qdclen6_length_evt(self, e):
        off = 36
        val = self._get_rounded_val(self.reg_map[off]['name'],
                float(self.QDC_Len6_wid.get()))
        val = int(val*1000)
        self.reg_map[off]['value'][self.cur_ch_num] = val
        self.QDC_Len6_wid.delete(0, tk.END)
        self.QDC_Len6_wid.insert(0, '%.3f'%(val/1000.))
        print('new %s selected: ' % self.reg_map[off]['name'])
        print(self.reg_map[off]['value'])


    def _qdclen7_length_evt(self, e):
        off = 37
        val = self._get_rounded_val(self.reg_map[off]['name'],
                float(self.QDC_Len7_wid.get()))
        val = int(val*1000)
        self.reg_map[off]['value'][self.cur_ch_num] = val
        self.QDC_Len7_wid.delete(0, tk.END)
        self.QDC_Len7_wid.insert(0, '%.3f'%(val/1000.))
        print('new %s selected: ' % self.reg_map[off]['name'])
        print(self.reg_map[off]['value'])


    def _qdclen0_length_evt(self, e):
        off = 30
        val = self._get_rounded_val(self.reg_map[off]['name'],
                float(self.QDC_Len0_wid.get()))
        val = int(val*1000)
        self.reg_map[off]['value'][self.cur_ch_num] = val
        self.QDC_Len0_wid.delete(0, tk.END)
        self.QDC_Len0_wid.insert(0, '%.3f'%(val/1000.))
        print('new %s selected: ' % self.reg_map[off]['name'])
        print(self.reg_map[off]['value'])





    def _recordqdc_evt(self, e):
        off = 29
        if self.RecordQDC_wid.get() == 'yes':
            val = 1
        else:
            val = 0
        self.reg_map[off]['value'][self.cur_ch_num] = val
        print('new %s selected: ' % self.reg_map[off]['name'])
        print(self.reg_map[off]['value'])





    def _trace_length_evt(self, e):
        off = 28
        val = self._get_rounded_val(self.reg_map[off]['name'],
                float(self.Trace_Length_wid.get()))
        val = int(val*1000)
        self.reg_map[off]['value'][self.cur_ch_num] = val
        self.Trace_Length_wid.delete(0, tk.END)
        self.Trace_Length_wid.insert(0, '%.3f'%(val/1000.))
        print('new %s selected: ' % self.reg_map[off]['name'])
        print(self.reg_map[off]['value'])





    def _trace_pretrig_evt(self, e):
        off = 27
        val = self._get_rounded_val(self.reg_map[off]['name'],
                float(self.Trace_PreTrig_wid.get()))
        val = int(val*1000)
        self.reg_map[off]['value'][self.cur_ch_num] = val
        self.Trace_PreTrig_wid.delete(0, tk.END)
        self.Trace_PreTrig_wid.insert(0, '%.3f'%(val/1000.))
        print('new %s selected: ' % self.reg_map[off]['name'])
        print(self.reg_map[off]['value'])


    def _RecordTrace_evt(self, e):
        off = 26
        if self.RecordTrace_wid.get() == 'yes':
            val = 1
        else:
            val = 0
        self.reg_map[off]['value'][self.cur_ch_num] = val
        print('new %s selected: ' % self.reg_map[off]['name'])
        print(self.reg_map[off]['value'])




    def _cfd_threshol_evt(self, e):
        off = 25
        val = self._get_rounded_val(self.reg_map[off]['name'],
                int(self.CFD_Threshol_wid.get()))
        self.reg_map[off]['value'][self.cur_ch_num] = val
        self.CFD_Threshol_wid.delete(0, tk.END)
        self.CFD_Threshol_wid.insert(0, str(val))
        print('new %s selected: ' % self.reg_map[off]['name'])
        print(self.reg_map[off]['value'])



    def _cfd_delay_evt(self, e):
        off = 24
        val = self._get_rounded_val(self.reg_map[off]['name'],
                float(self.CFD_Dealy_wid.get()))
        val = int(val*1000)
        self.reg_map[off]['value'][self.cur_ch_num] = val
        self.CFD_Dealy_wid.delete(0, tk.END)
        self.CFD_Dealy_wid.insert(0, '%.3f'%(val/1000.))
        print('new %s selected: ' % self.reg_map[off]['name'])
        print(self.reg_map[off]['value'])


    def _cfdscaler_evt(self, e):
        off = 23
        if self.CFD_Scaler_wid.get() == '8/8':
            val = 0
        elif self.CFD_Scaler_wid.get() == '7/8':
            val = 1
        elif self.CFD_Scaler_wid.get() == '6/8':
            val = 2
        elif self.CFD_Scaler_wid.get() == '5/8':
            val = 3
        elif self.CFD_Scaler_wid.get() == '4/8':
            val = 4
        elif self.CFD_Scaler_wid.get() == '3/8':
            val = 5
        elif self.CFD_Scaler_wid.get() == '2/8':
            val = 6
        elif self.CFD_Scaler_wid.get() == '1/8':
            val = 7
        self.reg_map[off]['value'][self.cur_ch_num] = val
        print('new %s selected: ' % self.reg_map[off]['name'])
        print(self.reg_map[off]['value'])



    def _cfdena_evt(self, e):
        off = 22
        if self.CFDEna_wid.get() == 'yes':
            val = 1
        else:
            val = 0
        self.reg_map[off]['value'][self.cur_ch_num] = val
        print('new %s selected: ' % self.reg_map[off]['name'])
        print(self.reg_map[off]['value'])




    def _esumena_evt(self, e):
        off = 21
        if self.ESumEna_wid.get() == 'yes':
            val = 1
        else:
            val = 0
        self.reg_map[off]['value'][self.cur_ch_num] = val
        print('new %s selected: ' % self.reg_map[off]['name'])
        print(self.reg_map[off]['value'])



    def _enextts_evt(self, e):
        off = 20
        if self.EnExtTS_wid.get() == 'yes':
            val = 1
        else:
            val = 0
        self.reg_map[off]['value'][self.cur_ch_num] = val
        print('new %s selected: ' % self.reg_map[off]['name'])
        print(self.reg_map[off]['value'])

    
    def _syncdata_evt(self, e):
        off = 19
        if self.SyncData_wid.get() == 'yes':
            val = 1
        else:
            val = 0
        self.reg_map[off]['value'][self.cur_ch_num] = val
        print('new %s selected: ' % self.reg_map[off]['name'])
        print(self.reg_map[off]['value'])



    def _pileup_evt(self, e):
        off = 18
        if self.PileUp_wid.get() == 'AcceptAll':
            val = 0
        elif self.PileUp_wid.get() == 'RejectPileUp':
            val = 1
        elif self.PileUp_wid.get() == 'TracePileUp':
            val = 2
        elif self.PileUp_wid.get() == 'RejectSingle':
            val = 3
        self.reg_map[off]['value'][self.cur_ch_num] = val
        print('new %s selected: ' % self.reg_map[off]['name'])
        print(self.reg_map[off]['value'])



    def _Tau_evt(self, e):
        off = 17
        val = self._get_rounded_val(self.reg_map[off]['name'],
                float(self.Tau_wid.get()))
        val = int(val*1000)
        self.reg_map[off]['value'][self.cur_ch_num] = val
        self.Tau_wid.delete(0, tk.END)
        self.Tau_wid.insert(0, '%.3f'%(val/1000.))
        print('new %s selected: ' % self.reg_map[off]['name'])
        print(self.reg_map[off]['value'])



    def _e_flattop_evt(self, e):
        off = 16
        val = self._get_rounded_val(self.reg_map[off]['name'],
                float(self.E_FlatTop_wid.get()))
        val = int(val*1000)
        self.reg_map[off]['value'][self.cur_ch_num] = val
        self.E_FlatTop_wid.delete(0, tk.END)
        self.E_FlatTop_wid.insert(0, '%.3f'%(val/1000.))
        print('new %s selected: ' % self.reg_map[off]['name'])
        print(self.reg_map[off]['value'])



    def _e_risetime_evt(self, e):
        off = 15
        val = self._get_rounded_val(self.reg_map[off]['name'],
                float(self.E_Risetime_wid.get()))
        val = int(val*1000)
        self.reg_map[off]['value'][self.cur_ch_num] = val
        self.E_Risetime_wid.delete(0, tk.END)
        self.E_Risetime_wid.insert(0, '%.3f'%(val/1000.))
        print('new %s selected: ' % self.reg_map[off]['name'])
        print(self.reg_map[off]['value'])


    def _t_thresh_evt(self, e):
        off = 14
        val = self._get_rounded_val(self.reg_map[off]['name'],
                int(self.T_Thresh_wid.get()))
        self.reg_map[off]['value'][self.cur_ch_num] = val
        self.T_Thresh_wid.delete(0, tk.END)
        self.T_Thresh_wid.insert(0, str(val))
        print('new %s selected: ' % self.reg_map[off]['name'])
        print(self.reg_map[off]['value'])


    def _t_flattop_evt(self, e):
        off = 13
        val = self._get_rounded_val(self.reg_map[off]['name'],
                float(self.T_FlatTop_wid.get()))
        val = int(val*1000)
        self.reg_map[off]['value'][self.cur_ch_num] = val
        self.T_FlatTop_wid.delete(0, tk.END)
        self.T_FlatTop_wid.insert(0, '%.3f'%(val/1000.))
        print('new %s selected: ' % self.reg_map[off]['name'])
        print(self.reg_map[off]['value'])



    def _t_risetime_evt(self, e):
        off = 12
        val = self._get_rounded_val(self.reg_map[off]['name'],
                float(self.T_Risetime_wid.get()))
        val = int(val*1000)
        self.reg_map[off]['value'][self.cur_ch_num] = val
        self.T_Risetime_wid.delete(0, tk.END)
        self.T_Risetime_wid.insert(0, '%.3f'%(val/1000.))
        print('new %s selected: ' % self.reg_map[off]['name'])
        print(self.reg_map[off]['value'])



    def _offset_evt(self, e):
        off = 11
        val = self._get_rounded_val(self.reg_map[off]['name'],
                int(self.offset_wid.get()))
        self.reg_map[off]['value'][self.cur_ch_num] = val
        self.offset_wid.delete(0, tk.END)
        self.offset_wid.insert(0, str(val))
        print('new %s selected: ' % self.reg_map[off]['name'])
        print(self.reg_map[off]['value'])


    def _gain_evt(self, e):
        off = 10
        if self.Gain_wid.get() == '0.25':
            val = 0
        else:
            val = 1
        self.reg_map[off]['value'][self.cur_ch_num] = val
        print('new %s selected: ' % self.reg_map[off]['name'])
        print(self.reg_map[off]['value'])


    def _pol_evt(self, e):
        off = 9
        if self.pol_wid.get() == 'Pos':
            val = 1
        else:
            val = 0
        self.reg_map[off]['value'][self.cur_ch_num] = val
        print('new %s selected: ' % self.reg_map[off]['name'])
        print(self.reg_map[off]['value'])


    def _cp_par_from_evt(self, e):
        from_ch = int(self.cp_par_from_wid.get()[2:])
        for off in [8 , 9 , 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22,
                23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 
                42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57,
                58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71]:
            val = self.reg_map[off]['value'][from_ch]
            self.reg_map[off]['value'][self.cur_ch_num] = val
        self._update_ch_pars()
    
    def _cp_par_to_evt(self, e):
        to_ch = self.cp_par_to_wid.get()
        if to_ch != 'all ch':
            to_ch = int(to_ch[2:])
        #print(to_ch)
        for off in [8 , 9 , 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22,
                23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 
                42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57,
                58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71]:
            val = self.reg_map[off]['value'][self.cur_ch_num]
            if to_ch == 'all ch':
                self.reg_map[off]['value'] = [val for i in range(16)]
            else:
                self.reg_map[off]['value'][to_ch] = val
                #print(to_ch)
        self._update_ch_pars()


    def _good_ch_evt(self, e):
        off = 8
        if self.GoodCh_wid.get() == 'yes':
            val = 1
        else:
            val = 0
        self.reg_map[off]['value'][self.cur_ch_num] = val
        print('new %s selected: ' % self.reg_map[off]['name'])
        print(self.reg_map[off]['value'])


    def _inc_ch_num(self):
        self.cur_ch_num += 1
        if self.cur_ch_num >= 15:
            self.cur_ch_num = 15
        self._update_ch_pars()
    def _dec_ch_num(self):
        self.cur_ch_num -= 1
        if self.cur_ch_num <= 0:
            self.cur_ch_num = 0
        self._update_ch_pars()


    def _update_ch_pars(self):
        self.ChNum_wid.config(state=tk.NORMAL)
        self.ChNum_wid.delete(0, tk.END)
        self.ChNum_wid.insert(0, str(self.cur_ch_num))
        self.ChNum_wid.config(state=tk.DISABLED)
        
        off = 7
        val = self.reg_map[off]['value'][self.cur_ch_num]
        self.XDT_wid.delete(0, tk.END)
        self.XDT_wid.insert(0, '%.3f'%(val/1000.))

        off = 8
        val = self.reg_map[off]['value'][self.cur_ch_num]
        good_ch_val = ['no', 'yes']
        self.GoodCh_wid.set(good_ch_val[val])

        off = 9
        val = self.reg_map[off]['value'][self.cur_ch_num]
        pol_val = ['Neg', 'Pos']
        self.pol_wid.set(pol_val[val])

        off = 10
        val = self.reg_map[off]['value'][self.cur_ch_num]
        gain_val = ['0.25', '1']
        self.Gain_wid.set(gain_val[val])

        off = 11
        val = self.reg_map[off]['value'][self.cur_ch_num]
        self.offset_wid.delete(0, tk.END)
        self.offset_wid.insert(0, str(val))

        off = 12
        val = self.reg_map[off]['value'][self.cur_ch_num]
        self.T_Risetime_wid.delete(0, tk.END)
        self.T_Risetime_wid.insert(0, '%.3f'%(val/1000.))

        off = 13
        val = self.reg_map[off]['value'][self.cur_ch_num]
        self.T_FlatTop_wid.delete(0, tk.END)
        self.T_FlatTop_wid.insert(0, '%.3f'%(val/1000.))

        off = 14
        val = self.reg_map[off]['value'][self.cur_ch_num]
        self.T_Thresh_wid.delete(0, tk.END)
        self.T_Thresh_wid.insert(0, str(val))

        off = 15
        val = self.reg_map[off]['value'][self.cur_ch_num]
        self.E_Risetime_wid.delete(0, tk.END)
        self.E_Risetime_wid.insert(0, '%.3f'%(val/1000.))

        off = 16
        val = self.reg_map[off]['value'][self.cur_ch_num]
        self.E_FlatTop_wid.delete(0, tk.END)
        self.E_FlatTop_wid.insert(0, '%.3f'%(val/1000.))

        off = 17
        val = self.reg_map[off]['value'][self.cur_ch_num]
        self.Tau_wid.delete(0, tk.END)
        self.Tau_wid.insert(0, '%.3f'%(val/1000.))

        off = 18
        val = self.reg_map[off]['value'][self.cur_ch_num]
        pileup_val = ['AcceptAll', 'RejectPileUp', 'TracePileUp', 'RejectSingle']
        self.PileUp_wid.set(pileup_val[val])

        off = 19
        val = self.reg_map[off]['value'][self.cur_ch_num]
        good_ch_val = ['no', 'yes']
        self.SyncData_wid.set(good_ch_val[val])

        off = 20
        val = self.reg_map[off]['value'][self.cur_ch_num]
        good_ch_val = ['no', 'yes']
        self.EnExtTS_wid.set(good_ch_val[val])

        off = 21
        val = self.reg_map[off]['value'][self.cur_ch_num]
        good_ch_val = ['no', 'yes']
        self.ESumEna_wid.set(good_ch_val[val])

        off = 22
        val = self.reg_map[off]['value'][self.cur_ch_num]
        good_ch_val = ['no', 'yes']
        self.CFDEna_wid.set(good_ch_val[val])

        off = 23
        val = self.reg_map[off]['value'][self.cur_ch_num]
        good_ch_val = ['%d/8'%(8-i) for i in range(8)]
        self.CFD_Scaler_wid.set(good_ch_val[val])

        off = 24
        val = self.reg_map[off]['value'][self.cur_ch_num]
        self.CFD_Dealy_wid.delete(0, tk.END)
        self.CFD_Dealy_wid.insert(0, '%.3f'%(val/1000.))

        off = 25
        val = self.reg_map[off]['value'][self.cur_ch_num]
        self.CFD_Threshol_wid.delete(0, tk.END)
        self.CFD_Threshol_wid.insert(0, str(val))

        off = 26
        val = self.reg_map[off]['value'][self.cur_ch_num]
        good_ch_val = ['no', 'yes']
        self.RecordTrace_wid.set(good_ch_val[val])

        off = 27
        val = self.reg_map[off]['value'][self.cur_ch_num]
        self.Trace_PreTrig_wid.delete(0, tk.END)
        self.Trace_PreTrig_wid.insert(0, '%.3f'%(val/1000.))

        off = 28
        val = self.reg_map[off]['value'][self.cur_ch_num]
        self.Trace_Length_wid.delete(0, tk.END)
        self.Trace_Length_wid.insert(0, '%.3f'%(val/1000.))

        off = 29
        val = self.reg_map[off]['value'][self.cur_ch_num]
        good_ch_val = ['no', 'yes']
        self.RecordQDC_wid.set(good_ch_val[val])

        off = 30
        val = self.reg_map[off]['value'][self.cur_ch_num]
        self.QDC_Len0_wid.delete(0, tk.END)
        self.QDC_Len0_wid.insert(0, '%.3f'%(val/1000.))

        off = 31
        val = self.reg_map[off]['value'][self.cur_ch_num]
        self.QDC_Len1_wid.delete(0, tk.END)
        self.QDC_Len1_wid.insert(0, '%.3f'%(val/1000.))

        off = 32
        val = self.reg_map[off]['value'][self.cur_ch_num]
        self.QDC_Len2_wid.delete(0, tk.END)
        self.QDC_Len2_wid.insert(0, '%.3f'%(val/1000.))

        off = 33
        val = self.reg_map[off]['value'][self.cur_ch_num]
        self.QDC_Len3_wid.delete(0, tk.END)
        self.QDC_Len3_wid.insert(0, '%.3f'%(val/1000.))

        off = 34
        val = self.reg_map[off]['value'][self.cur_ch_num]
        self.QDC_Len4_wid.delete(0, tk.END)
        self.QDC_Len4_wid.insert(0, '%.3f'%(val/1000.))

        off = 35
        val = self.reg_map[off]['value'][self.cur_ch_num]
        self.QDC_Len5_wid.delete(0, tk.END)
        self.QDC_Len5_wid.insert(0, '%.3f'%(val/1000.))

        off = 36
        val = self.reg_map[off]['value'][self.cur_ch_num]
        self.QDC_Len6_wid.delete(0, tk.END)
        self.QDC_Len6_wid.insert(0, '%.3f'%(val/1000.))

        off = 37
        val = self.reg_map[off]['value'][self.cur_ch_num]
        self.QDC_Len7_wid.delete(0, tk.END)
        self.QDC_Len7_wid.insert(0, '%.3f'%(val/1000.))

        off = 38
        val = self.reg_map[off]['value'][self.cur_ch_num]
        self.BLCUT_wid.delete(0, tk.END)
        self.BLCUT_wid.insert(0, str(val))

        off = 42
        val = self.reg_map[off]['value'][self.cur_ch_num]
        gain_val = ['Local FT', 'Mod FT', 'Ch VT']
        self.ChFT_wid.set(gain_val[val])

        off = 43
        val = self.reg_map[off]['value'][self.cur_ch_num]
        self.ChVT_var.set(val)

        off = 44
        val = self.reg_map[off]['value'][self.cur_ch_num]
        gain_val = ['ChanTrig', 'ChInFP']
        self.ChVT_sel_wid.set(gain_val[val])

        off = 45
        val = self.reg_map[off]['value'][self.cur_ch_num]
        self.ModVT_var.set(val)

        off = 46
        val = self.reg_map[off]['value'][self.cur_ch_num]
        gain_val = ['Mod_VT', 'ModInFP']
        self.ModVT_sel_wid.set(gain_val[val])

        off = 47
        val = self.reg_map[off]['value'][self.cur_ch_num]
        self.Veto_var.set(val)

        off = 48
        val = self.reg_map[off]['value'][self.cur_ch_num]
        gain_val = ['ChInFP', 'Ch_VT']
        self.ChVeto_sel_wid.set(gain_val[val])

        off = 49
        val = self.reg_map[off]['value'][self.cur_ch_num]
        gain_val = ['ModIn_FP', 'Mod_VT']
        self.ModVeto_wid.set(gain_val[val])

        off = 50
        val = self.reg_map[off]['value'][self.cur_ch_num]
        self.ChanTrig_ChVT_val.set(val)

        off = 51
        val = self.reg_map[off]['value'][self.cur_ch_num]
        self.MaskL_wid.delete(0, tk.END)
        self.MaskL_wid.insert(0, '%04x'%val)

        off = 52
        val = self.reg_map[off]['value'][self.cur_ch_num]
        self.MaskM_wid.delete(0, tk.END)
        self.MaskM_wid.insert(0, '%04x'%val)

        off = 53
        val = self.reg_map[off]['value'][self.cur_ch_num]
        self.MaskR_wid.delete(0, tk.END)
        self.MaskR_wid.insert(0, '%04x'%val)

        off = 54
        val = self.reg_map[off]['value'][self.cur_ch_num]
        self.MulTh_wid.delete(0, tk.END)
        self.MulTh_wid.insert(0, str(val))

        off = 55
        txt = self.reg_map[off]['name']
        self.CoinThL_wid.delete(0, tk.END)
        self.CoinThL_wid.insert(0, str(val))

        off = 56
        val = self.reg_map[off]['value'][self.cur_ch_num]
        self.CoinThM_wid.delete(0, tk.END)
        self.CoinThM_wid.insert(0, str(val))

        off = 57
        val = self.reg_map[off]['value'][self.cur_ch_num]
        self.CoinThR_wid.delete(0, tk.END)
        self.CoinThR_wid.insert(0, str(val))

        off = 58
        val = self.reg_map[off]['value'][self.cur_ch_num]
        gain_val = ['ChM', 'ChR', 'ChL']
        self.Group0_wid.set(gain_val[val])

        off = 59
        val = self.reg_map[off]['value'][self.cur_ch_num]
        gain_val = ['ChM', 'ChR', 'ChL']
        self.Group1_wid.set(gain_val[val])
        
        off = 60
        val = self.reg_map[off]['value'][self.cur_ch_num]
        gain_val = ['ChM', 'ChR', 'ChL']
        self.Group2_wid.set(gain_val[val])

        off = 61
        val = self.reg_map[off]['value'][self.cur_ch_num]
        gain_val = ['ChM', 'ChR', 'ChL']
        self.Group3_wid.set(gain_val[val])

        off = 62
        val = self.reg_map[off]['value'][self.cur_ch_num]
        self.Group0ch_wid.delete(0, tk.END)
        self.Group0ch_wid.insert(0, str(val))

        off = 63
        val = self.reg_map[off]['value'][self.cur_ch_num]
        self.Group1ch_wid.delete(0, tk.END)
        self.Group1ch_wid.insert(0, str(val))

        off = 64
        val = self.reg_map[off]['value'][self.cur_ch_num]
        self.Group2ch_wid.delete(0, tk.END)
        self.Group2ch_wid.insert(0, str(val))

        off = 65
        val = self.reg_map[off]['value'][self.cur_ch_num]
        self.Group3ch_wid.delete(0, tk.END)
        self.Group3ch_wid.insert(0, str(val))

        off = 66
        val = self.reg_map[off]['value'][self.cur_ch_num]
        self.LCFT_Delay_wid.delete(0, tk.END)
        self.LCFT_Delay_wid.insert(0, '%.3f'%(val/1000.))

        off = 67
        val = self.reg_map[off]['value'][self.cur_ch_num]
        self.LCFT_Width_wid.delete(0, tk.END)
        self.LCFT_Width_wid.insert(0, '%.3f'%(val/1000.))

        off = 68
        val = self.reg_map[off]['value'][self.cur_ch_num]
        self.FIFO_Delay_wid.delete(0, tk.END)
        self.FIFO_Delay_wid.insert(0, '%.3f'%(val/1000.))

        off = 69
        val = self.reg_map[off]['value'][self.cur_ch_num]
        self.ChVT_Width_wid.delete(0, tk.END)
        self.ChVT_Width_wid.insert(0, '%.3f'%(val/1000.))

        off = 70
        val = self.reg_map[off]['value'][self.cur_ch_num]
        self.ModVT_Width_wid.delete(0, tk.END)
        self.ModVT_Width_wid.insert(0, '%.3f'%(val/1000.))

        off = 71
        val = self.reg_map[off]['value'][self.cur_ch_num]
        self.Veto_Width_wid.delete(0, tk.END)
        self.Veto_Width_wid.insert(0, '%.3f'%(val/1000.))
        
        self.ChNum_wid1.config(state=tk.NORMAL)
        self.ChNum_wid1.delete(0, tk.END)
        self.ChNum_wid1.insert(0, str(self.cur_ch_num))
        self.ChNum_wid1.config(state=tk.DISABLED)
        
        # AutoBLCUT
        off = 87
        val = self.reg_map[off]['value'][self.cur_ch_num]
        self.auto_blcut_var.set(val)



    def _XDT_evt(self, e):
        off = 7
        val = self._get_rounded_val(self.reg_map[off]['name'],
                float(self.XDT_wid.get()))
        val = int(val*1000)
        self.reg_map[off]['value'][self.cur_ch_num] = val
        self.XDT_wid.delete(0, tk.END)
        self.XDT_wid.insert(0, '%.3f'%(val/1000.))
        print('new %s selected: ' % self.reg_map[off]['name'])
        print(self.reg_map[off]['value'])


    def _MulCrate_evt(self):
        off = 6
        self.reg_map[off]['value'] = [self.MulCrate_var.get() for i in range(16)]
        print('new %s selected: ' % self.reg_map[off]['name'])
        print(self.reg_map[off]['value'])


    def _InhibitEna_evt(self):
        off = 5
        self.reg_map[off]['value'] = [self.InhibitEna_var.get() for i in range(16)]
        print('new %s selected: ' % self.reg_map[off]['name'])
        print(self.reg_map[off]['value'])



    def _SortEvent_evt(self):
        off = 4
        self.reg_map[off]['value'] = [self.SortEvent_var.get() for i in range(16)]
        print('new %s selected: ' % self.reg_map[off]['name'])
        print(self.reg_map[off]['value'])



    def _CPLD_evt(self):
        off = 3
        self.reg_map[off]['value'] = [self.CPLD_var.get() for i in range(16)]
        print('new %s selected: ' % self.reg_map[off]['name'])
        print(self.reg_map[off]['value'])


    def _rev_evt(self, e):
        off = 88
        rev = self.rev_wid.get()
        if rev == 'rev_B':
            val = 0
        elif rev == 'rev_C':
            val = 1
        elif rev == 'rev_D':
            val = 2
        elif rev == 'rev_F':
            val = 3
        elif rev == 'rev_H':
            val = 4
        self.reg_map[off]['value'] = [val for i in range(16)]
        print('new %s selected: ' % self.reg_map[off]['name'])
        print(self.reg_map[off]['value'])



    def _t_filter_range_evt(self, e):
        off = 2
        self.reg_map[off]['value'] = [int(self.T_Filter_Range_wid.get())-1 for i in range(16)]
        print('new %s selected: ' % self.reg_map[off]['name'])
        print(self.reg_map[off]['value'])



    def _e_filter_range_evt(self, e):
        off = 1
        self.reg_map[off]['value'] = [int(self.E_Filter_Range_wid.get())-1 for i in range(16)]
        print('new %s selected: ' % self.reg_map[off]['name'])
        print(self.reg_map[off]['value'])



    def _slot_evt(self, e):
        self.slot = int(self.slot_wid.get())
        print('new slot selected: %d' % self.slot)


    def _ModNum_evt(self, e):
        print('new mod num: ' + self.ModNum_wid.get())
        print('evt: ')
        print(e)

    def _create_ModTrigSet(self, tab_ctrl):
        self.ModTrigSet = tk.Frame(tab_ctrl)
        tab_ctrl.add(self.ModTrigSet, text='ModTrigSet')
        tab_ctrl.pack(fill=tk.BOTH, expand=1)

        win = self.ModTrigSet
        # create the widgets for each register
        # ModNum
        off = 0
        txt = self.reg_map[off]['name']
        val = self.reg_map[off]['value'][0]
        tk.Label(win, text=txt).place(x=5, y=10, width=60, height=25)
        self.ModNum_wid2 = tk.Entry(win)
        self.ModNum_wid2.place(x=70, y=10, width=20, height=25)
        self.ModNum_wid2.insert(0, str(val))
        self.ModNum_wid2.config(state=tk.DISABLED)

        # FTBPEna_l
        off = 40
        #txt = self.reg_map[off]['name']
        txt = 'FTBPEna:: Send To Left (16bit): 0x'
        val = self.reg_map[off]['value'][self.cur_ch_num]
        tk.Label(win, text=txt, anchor=tk.W).place(x=5, y=40, width=215,
                height=25)
        self.FTBPEna_l_wid = tk.Entry(win)
        self.FTBPEna_l_wid.place(x=220, y=40, width=60, height=25)
        self.FTBPEna_l_wid.insert(0, '%04x'%val)
        self.FTBPEna_l_wid.bind('<Return>',self._ftbpl_evt)

        # FTBPEna_r
        off = 41
        #txt = self.reg_map[off]['name']
        txt = ', Send To Right (16bit): 0x'
        val = self.reg_map[off]['value'][self.cur_ch_num]
        tk.Label(win, text=txt, anchor=tk.W).place(x=280, y=40, width=200,
                height=25)
        self.FTBPEna_r_wid = tk.Entry(win)
        self.FTBPEna_r_wid.place(x=445, y=40, width=60, height=25)
        self.FTBPEna_r_wid.insert(0, '%04x'%val)
        self.FTBPEna_r_wid.bind('<Return>',self._ftbpr_evt)

        # Ch Trig set
        tk.Label(win, bg='yellow', text='Ch Trig set').place(x=10, y=75,
                width=80, height=20)
        self.Ch_Trig_set = tk.Frame(win, height=280, width=780, relief =
                tk.GROOVE, borderwidth = 2)
        self.Ch_Trig_set.place(x=10, y=95)
        win = self.Ch_Trig_set

        # channel number 
        off = 39
        txt = self.reg_map[off]['name']
        val = self.reg_map[off]['value'][self.cur_ch_num]
        tk.Label(win, text=txt, anchor=tk.W).place(x=10, y=5, width=60,
                height=25)
        self.ChNum_wid1 = tk.Entry(win)
        self.ChNum_wid1.place(x=70, y=5, width=25, height=25)
        self.ChNum_wid1.insert(0, str(val))
        self.ChNum_wid1.config(state=tk.DISABLED)
        #self.ChNum_wid1.bind('<Return>',self._ChNum_evt)
        # decrease/increase current ch num buttons
        tk.Button(win, text='+', command=self._inc_ch_num,
                bg='gray').place(x=95, y=5, width=20,height=13)
        tk.Button(win, text='-', command=self._dec_ch_num,
                bg='gray').place(x=95, y=18, width=20,height=13)

        # ChFT
        off = 42
        txt = self.reg_map[off]['name']
        val = self.reg_map[off]['value'][self.cur_ch_num]
        gain_val = ['Local FT', 'Mod FT', 'Ch VT']
        tk.Label(win, anchor=tk.W, text=txt).place(x=10, y=35, width=60,
                height=25)
        self.ChFT_wid = ttk.Combobox(win, state='readonly', values=gain_val)
        self.ChFT_wid.set(gain_val[val])
        self.ChFT_wid.place(x=50, y=35, width=70, height=25)
        self.ChFT_wid.bind("<<ComboboxSelected>>", self._chft_evt)

        # ChVT
        off = 43
        txt = self.reg_map[off]['name']
        val = self.reg_map[off]['value'][self.cur_ch_num]
        self.ChVT_var = tk.IntVar()
        self.ChVT_var.set(val)
        tk.Checkbutton(win, text=txt, variable=self.ChVT_var, anchor=tk.W,
                command=self._ChVT_evt).place(x=120, y=35, width=60,
                        height=25)
        self.ChVT_var.set(val)

        # ChVT_sel
        off = 44
        txt = self.reg_map[off]['name']
        val = self.reg_map[off]['value'][self.cur_ch_num]
        gain_val = ['ChanTrig', 'ChInFP']
        #tk.Label(win, anchor=tk.W, text=txt).place(x=10, y=35, width=60,
                #height=25)
        self.ChVT_sel_wid = ttk.Combobox(win, state='readonly',
                values=gain_val)
        self.ChVT_sel_wid.set(gain_val[val])
        self.ChVT_sel_wid.place(x=180, y=35, width=80, height=25)
        self.ChVT_sel_wid.bind("<<ComboboxSelected>>", self._chvt_sel_evt)

        # ModVT
        off = 45
        txt = self.reg_map[off]['name']
        val = self.reg_map[off]['value'][self.cur_ch_num]
        self.ModVT_var = tk.IntVar()
        self.ModVT_var.set(val)
        tk.Checkbutton(win, text=txt, variable=self.ModVT_var, anchor=tk.W,
                command=self._ModVT_evt).place(x=270, y=35, width=70,
                        height=25)
        self.ModVT_var.set(val)

        # ModVT_sel
        off = 46
        txt = self.reg_map[off]['name']
        val = self.reg_map[off]['value'][self.cur_ch_num]
        gain_val = ['Mod_VT', 'ModInFP']
        #tk.Label(win, anchor=tk.W, text=txt).place(x=10, y=35, width=60,
                #height=25)
        self.ModVT_sel_wid = ttk.Combobox(win, state='readonly',
                values=gain_val)
        self.ModVT_sel_wid.set(gain_val[val])
        self.ModVT_sel_wid.place(x=340, y=35, width=80, height=25)
        self.ModVT_sel_wid.bind("<<ComboboxSelected>>", self._modvt_sel_evt)

        # Veto
        off = 47
        txt = self.reg_map[off]['name']
        val = self.reg_map[off]['value'][self.cur_ch_num]
        self.Veto_var = tk.IntVar()
        self.Veto_var.set(val)
        tk.Checkbutton(win, text=txt, variable=self.Veto_var, anchor=tk.W,
                command=self._Veto_evt).place(x=430, y=35, width=70,
                        height=25)
        self.Veto_var.set(val)

        # ChVeto
        off = 48
        txt = self.reg_map[off]['name']
        val = self.reg_map[off]['value'][self.cur_ch_num]
        gain_val = ['ChInFP', 'Ch_VT']
        tk.Label(win, anchor=tk.W, text=txt).place(x=490, y=35, width=60,
                height=25)
        self.ChVeto_sel_wid = ttk.Combobox(win, state='readonly',
                values=gain_val)
        self.ChVeto_sel_wid.set(gain_val[val])
        self.ChVeto_sel_wid.place(x=550, y=35, width=80, height=25)
        self.ChVeto_sel_wid.bind("<<ComboboxSelected>>", self._chveto_sel_evt)

        # ModVeto
        off = 49
        txt = self.reg_map[off]['name']
        val = self.reg_map[off]['value'][self.cur_ch_num]
        gain_val = ['ModIn_FP', 'Mod_VT']
        tk.Label(win, anchor=tk.W, text=txt).place(x=630, y=35, width=70,
                height=25)
        self.ModVeto_wid = ttk.Combobox(win, state='readonly',
                values=gain_val)
        self.ModVeto_wid.set(gain_val[val])
        self.ModVeto_wid.place(x=700, y=35, width=75, height=25)
        self.ModVeto_wid.bind("<<ComboboxSelected>>", self._modveto_sel_evt)

        # ChanTrig(Ch VT)
        tk.Label(win, bg='orange', text='ChanTrig(Ch VT)').place(x=10, y=68,
                width=100, height=20)
        self.ChanTrig_frm = tk.Frame(win, height=190, width=125, relief =
                tk.GROOVE, borderwidth = 2)
        self.ChanTrig_frm.place(x=10, y=85)
        win = self.ChanTrig_frm

        # ChanTrig_ChVT
        off = 50
        val = self.reg_map[off]['value'][self.cur_ch_num]
        self.ChanTrig_ChVT_val = tk.IntVar()
        self.ChanTrig_ChVT_val.set(val)
        self.ChanTrig_ChVT_wid0 = tk.Radiobutton(win, text="Mult_Trig",
                variable=self.ChanTrig_ChVT_val, value=0, anchor=tk.W,
                command=self._ChanTrig_ChVT_evt)
        self.ChanTrig_ChVT_wid1 = tk.Radiobutton(win, text="Coin_Trig",
                variable=self.ChanTrig_ChVT_val, value=1, anchor=tk.W,
                command=self._ChanTrig_ChVT_evt)
        self.ChanTrig_ChVT_wid2 = tk.Radiobutton(win, text="Group_Trig",
                variable=self.ChanTrig_ChVT_val, value=2, anchor=tk.W,
                command=self._ChanTrig_ChVT_evt)
        self.ChanTrig_ChVT_wid3 = tk.Radiobutton(win, text="ExtFT_LocalFT",
                variable=self.ChanTrig_ChVT_val, value=3, anchor=tk.W,
                command=self._ChanTrig_ChVT_evt)
        self.ChanTrig_ChVT_wid0.place(x=2, y=5, width=115, height=25)
        self.ChanTrig_ChVT_wid1.place(x=2, y=35, width=115, height=25)
        self.ChanTrig_ChVT_wid2.place(x=2, y=65, width=115, height=25)
        self.ChanTrig_ChVT_wid3.place(x=2, y=95, width=115, height=25)

        # Mul/Coin Trig Set
        win = self.Ch_Trig_set
        tk.Label(win, bg='orange', text='Mul/Coin Trig Set').place(x=140, y=68,
                width=110, height=20)
        self.Mul_Coin_Trig_frm = tk.Frame(win, height=190, width=240, relief =
                tk.GROOVE, borderwidth = 2)
        self.Mul_Coin_Trig_frm.place(x=140, y=85)
        win = self.Mul_Coin_Trig_frm

        # MaskL
        off = 51
        txt = self.reg_map[off]['name']
        val = self.reg_map[off]['value'][self.cur_ch_num]
        tk.Label(win, text=txt, anchor=tk.W).place(x=5, y=5, width=60,
                height=25)
        self.MaskL_wid = tk.Entry(win)
        self.MaskL_wid.place(x=65, y=5, width=60, height=25)
        self.MaskL_wid.insert(0, '%04x'%val)
        self.MaskL_wid.bind('<Return>',self._maskl_evt)

        # MaskM
        off = 52
        txt = self.reg_map[off]['name']
        val = self.reg_map[off]['value'][self.cur_ch_num]
        tk.Label(win, text=txt, anchor=tk.W).place(x=5, y=35, width=60,
                height=25)
        self.MaskM_wid = tk.Entry(win)
        self.MaskM_wid.place(x=65, y=35, width=60, height=25)
        self.MaskM_wid.insert(0, '%04x'%val)
        self.MaskM_wid.bind('<Return>',self._maskm_evt)

        # MaskR
        off = 53
        txt = self.reg_map[off]['name']
        val = self.reg_map[off]['value'][self.cur_ch_num]
        tk.Label(win, text=txt, anchor=tk.W).place(x=5, y=65, width=60,
                height=25)
        self.MaskR_wid = tk.Entry(win)
        self.MaskR_wid.place(x=65, y=65, width=60, height=25)
        self.MaskR_wid.insert(0, '%04x'%val)
        self.MaskR_wid.bind('<Return>',self._maskr_evt)

        # MulTh
        off = 54
        txt = self.reg_map[off]['name']
        val = self.reg_map[off]['value'][self.cur_ch_num]
        tk.Label(win, text=txt, anchor=tk.W).place(x=5, y=95, width=60,
                height=25)
        self.MulTh_wid = tk.Entry(win)
        self.MulTh_wid.place(x=65, y=95, width=60, height=25)
        self.MulTh_wid.insert(0, str(val))
        self.MulTh_wid.bind('<Return>',self._multh_evt)

        # CoinThL
        off = 55
        txt = self.reg_map[off]['name']
        val = self.reg_map[off]['value'][self.cur_ch_num]
        tk.Label(win, text=txt, anchor=tk.W).place(x=130, y=5, width=60,
                height=25)
        self.CoinThL_wid = tk.Entry(win)
        self.CoinThL_wid.place(x=185, y=5, width=50, height=25)
        self.CoinThL_wid.insert(0, str(val))
        self.CoinThL_wid.bind('<Return>',self._cointhl_evt)

        # CoinThM
        off = 56
        txt = self.reg_map[off]['name']
        val = self.reg_map[off]['value'][self.cur_ch_num]
        tk.Label(win, text=txt, anchor=tk.W).place(x=130, y=35, width=60,
                height=25)
        self.CoinThM_wid = tk.Entry(win)
        self.CoinThM_wid.place(x=185, y=35, width=50, height=25)
        self.CoinThM_wid.insert(0, str(val))
        self.CoinThM_wid.bind('<Return>',self._cointhm_evt)

        # CoinThR
        off = 57
        txt = self.reg_map[off]['name']
        val = self.reg_map[off]['value'][self.cur_ch_num]
        tk.Label(win, text=txt, anchor=tk.W).place(x=130, y=65, width=60,
                height=25)
        self.CoinThR_wid = tk.Entry(win)
        self.CoinThR_wid.place(x=185, y=65, width=50, height=25)
        self.CoinThR_wid.insert(0, str(val))
        self.CoinThR_wid.bind('<Return>',self._cointhr_evt)

        # Group Trig Sel, From Ch#
        win = self.Ch_Trig_set
        tk.Label(win, bg='orange', text='Group Trig Sel, From Ch#').place(x=385,
                y=68, width=160, height=20)
        self.Group_Trig_Sel = tk.Frame(win, height=190, width=180, relief =
                tk.GROOVE, borderwidth = 2)
        self.Group_Trig_Sel.place(x=385, y=85)
        win = self.Group_Trig_Sel

        # Group0
        off = 58
        txt = self.reg_map[off]['name']
        val = self.reg_map[off]['value'][self.cur_ch_num]
        gain_val = ['ChM', 'ChR', 'ChL']
        tk.Label(win, anchor=tk.W, text=txt).place(x=5, y=5, width=60,
                height=25)
        self.Group0_wid = ttk.Combobox(win, state='readonly', values=gain_val)
        self.Group0_wid.set(gain_val[val])
        self.Group0_wid.place(x=58, y=5, width=47, height=25)
        self.Group0_wid.bind("<<ComboboxSelected>>", self._group0_evt)

        # Group1
        off = 59
        txt = self.reg_map[off]['name']
        val = self.reg_map[off]['value'][self.cur_ch_num]
        gain_val = ['ChM', 'ChR', 'ChL']
        tk.Label(win, anchor=tk.W, text=txt).place(x=5, y=35, width=60,
                height=25)
        self.Group1_wid = ttk.Combobox(win, state='readonly', values=gain_val)
        self.Group1_wid.set(gain_val[val])
        self.Group1_wid.place(x=58, y=35, width=47, height=25)
        self.Group1_wid.bind("<<ComboboxSelected>>", self._group1_evt)


        # Group2
        off = 60
        txt = self.reg_map[off]['name']
        val = self.reg_map[off]['value'][self.cur_ch_num]
        gain_val = ['ChM', 'ChR', 'ChL']
        tk.Label(win, anchor=tk.W, text=txt).place(x=5, y=65, width=60,
                height=25)
        self.Group2_wid = ttk.Combobox(win, state='readonly', values=gain_val)
        self.Group2_wid.set(gain_val[val])
        self.Group2_wid.place(x=58, y=65, width=47, height=25)
        self.Group2_wid.bind("<<ComboboxSelected>>", self._group2_evt)


        # Group3
        off = 61
        txt = self.reg_map[off]['name']
        val = self.reg_map[off]['value'][self.cur_ch_num]
        gain_val = ['ChM', 'ChR', 'ChL']
        tk.Label(win, anchor=tk.W, text=txt).place(x=5, y=95, width=60,
                height=25)
        self.Group3_wid = ttk.Combobox(win, state='readonly', values=gain_val)
        self.Group3_wid.set(gain_val[val])
        self.Group3_wid.place(x=58, y=95, width=47, height=25)
        self.Group3_wid.bind("<<ComboboxSelected>>", self._group3_evt)

        # Group0ch
        off = 62
        txt = self.reg_map[off]['name']
        val = self.reg_map[off]['value'][self.cur_ch_num]
        #tk.Label(win, text=txt, anchor=tk.W).place(x=110, y=5, width=60,
                #height=25)
        self.Group0ch_wid = tk.Entry(win)
        self.Group0ch_wid.place(x=110, y=5, width=50, height=25)
        self.Group0ch_wid.insert(0, str(val))
        self.Group0ch_wid.bind('<Return>',self._group0ch_evt)

        # Group1ch
        off = 63
        txt = self.reg_map[off]['name']
        val = self.reg_map[off]['value'][self.cur_ch_num]
        #tk.Label(win, text=txt, anchor=tk.W).place(x=110, y=5, width=60,
                #height=25)
        self.Group1ch_wid = tk.Entry(win)
        self.Group1ch_wid.place(x=110, y=35, width=50, height=25)
        self.Group1ch_wid.insert(0, str(val))
        self.Group1ch_wid.bind('<Return>',self._group1ch_evt)

        # Group2ch
        off = 64
        txt = self.reg_map[off]['name']
        val = self.reg_map[off]['value'][self.cur_ch_num]
        #tk.Label(win, text=txt, anchor=tk.W).place(x=110, y=5, width=60,
                #height=25)
        self.Group2ch_wid = tk.Entry(win)
        self.Group2ch_wid.place(x=110, y=65, width=50, height=25)
        self.Group2ch_wid.insert(0, str(val))
        self.Group2ch_wid.bind('<Return>',self._group2ch_evt)

        # Group3ch
        off = 65
        txt = self.reg_map[off]['name']
        val = self.reg_map[off]['value'][self.cur_ch_num]
        #tk.Label(win, text=txt, anchor=tk.W).place(x=110, y=5, width=60,
                #height=25)
        self.Group3ch_wid = tk.Entry(win)
        self.Group3ch_wid.place(x=110, y=95, width=50, height=25)
        self.Group3ch_wid.insert(0, str(val))
        self.Group3ch_wid.bind('<Return>',self._group3ch_evt)

        # delay and stretch (in us unit)
        win = self.Ch_Trig_set
        tk.Label(win, bg='orange', 
                text='delay and stretch (in us unit)').place(
                        x=570, y=68, width=180, height=20)
        self.delay_stretch = tk.Frame(win, height=190, width=190, relief =
                tk.GROOVE, borderwidth = 2)
        self.delay_stretch.place(x=570, y=85)
        win = self.delay_stretch

        # LCFT Delay
        off = 66
        txt = self.reg_map[off]['name']
        val = self.reg_map[off]['value'][self.cur_ch_num]
        tk.Label(win, text=txt, anchor=tk.W).place(x=5, y=5, width=85,
                height=25)
        self.LCFT_Delay_wid = tk.Entry(win)
        self.LCFT_Delay_wid.place(x=90, y=5, width=60, height=25)
        self.LCFT_Delay_wid.insert(0, '%.3f'%(val/1000.))
        self.LCFT_Delay_wid.bind('<Return>',self._lcft_delay_evt)


        # LCFT Width
        off = 67
        txt = self.reg_map[off]['name']
        val = self.reg_map[off]['value'][self.cur_ch_num]
        tk.Label(win, text=txt, anchor=tk.W).place(x=5, y=35, width=85,
                height=25)
        self.LCFT_Width_wid = tk.Entry(win)
        self.LCFT_Width_wid.place(x=90, y=35, width=60, height=25)
        self.LCFT_Width_wid.insert(0, '%.3f'%(val/1000.))
        self.LCFT_Width_wid.bind('<Return>',self._lcft_width_evt)


        # FIFO Delay
        off = 68
        txt = self.reg_map[off]['name']
        val = self.reg_map[off]['value'][self.cur_ch_num]
        tk.Label(win, text=txt, anchor=tk.W).place(x=5, y=65, width=85,
                height=25)
        self.FIFO_Delay_wid = tk.Entry(win)
        self.FIFO_Delay_wid.place(x=90, y=65, width=60, height=25)
        self.FIFO_Delay_wid.insert(0, '%.3f'%(val/1000.))
        self.FIFO_Delay_wid.bind('<Return>',self._fifi_delay_evt)

        # ChVT Width
        off = 69
        txt = self.reg_map[off]['name']
        val = self.reg_map[off]['value'][self.cur_ch_num]
        tk.Label(win, text=txt, anchor=tk.W).place(x=5, y=95, width=85,
                height=25)
        self.ChVT_Width_wid = tk.Entry(win)
        self.ChVT_Width_wid.place(x=90, y=95, width=60, height=25)
        self.ChVT_Width_wid.insert(0, '%.3f'%(val/1000.))
        self.ChVT_Width_wid.bind('<Return>',self._chvt_width_evt)

        # ModVT Width
        off = 70
        txt = self.reg_map[off]['name']
        val = self.reg_map[off]['value'][self.cur_ch_num]
        tk.Label(win, text=txt, anchor=tk.W).place(x=5, y=125, width=85,
                height=25)
        self.ModVT_Width_wid = tk.Entry(win)
        self.ModVT_Width_wid.place(x=90, y=125, width=60, height=25)
        self.ModVT_Width_wid.insert(0, '%.3f'%(val/1000.))
        self.ModVT_Width_wid.bind('<Return>',self._modvt_width_evt)

        # Veto Width
        off = 71
        txt = self.reg_map[off]['name']
        val = self.reg_map[off]['value'][self.cur_ch_num]
        tk.Label(win, text=txt, anchor=tk.W).place(x=5, y=155, width=85,
                height=25)
        self.Veto_Width_wid = tk.Entry(win)
        self.Veto_Width_wid.place(x=90, y=155, width=60, height=25)
        self.Veto_Width_wid.insert(0, '%.3f'%(val/1000.))
        self.Veto_Width_wid.bind('<Return>',self._veto_width_evt)

        # Ext_ValTrig_In
        win = self.ModTrigSet
        tk.Label(win, bg='yellow', text='Ext_ValTrig_In').place(x=10, y=380,
                width=90, height=20)
        self.Ext_ValTrig_In = tk.Frame(win, height=180, width=210, relief =
                tk.GROOVE, borderwidth = 2)
        self.Ext_ValTrig_In.place(x=10, y=400)
        win = self.Ext_ValTrig_In

        off = 72
        val = self.reg_map[off]['value'][self.cur_ch_num]
        self.Ext_ValTrig_In_val = tk.IntVar()
        self.Ext_ValTrig_In_val.set(val)
        self.Ext_ValTrig_In_wid0 = tk.Radiobutton(win, text="Ext_VT_Sel",
                variable=self.Ext_ValTrig_In_val, value=0, anchor=tk.W,
                command=self._Ext_ValTrig_In_evt)
        self.Ext_ValTrig_In_wid1 = tk.Radiobutton(win, text="IntVT_Sel",
                variable=self.Ext_ValTrig_In_val, value=1, anchor=tk.W,
                command=self._Ext_ValTrig_In_evt)
        self.Ext_ValTrig_In_wid2 = tk.Radiobutton(win, text="FTIn_OR",
                variable=self.Ext_ValTrig_In_val, value=2, anchor=tk.W,
                command=self._Ext_ValTrig_In_evt)
        self.Ext_ValTrig_In_wid3 = tk.Radiobutton(win, text="LVDSVT_FP",
                variable=self.Ext_ValTrig_In_val, value=3, anchor=tk.W,
                command=self._Ext_ValTrig_In_evt)
        self.Ext_ValTrig_In_wid4 = tk.Radiobutton(win, text="ChanTrig_Sel",
                variable=self.Ext_ValTrig_In_val, value=4, anchor=tk.W,
                command=self._Ext_ValTrig_In_evt)
        self.Ext_ValTrig_In_wid0.place(x=2, y=5, width=115, height=25)
        self.Ext_ValTrig_In_wid1.place(x=2, y=35, width=115, height=25)
        self.Ext_ValTrig_In_wid2.place(x=2, y=65, width=115, height=25)
        self.Ext_ValTrig_In_wid3.place(x=2, y=95, width=115, height=25)
        self.Ext_ValTrig_In_wid4.place(x=2, y=125, width=115, height=25)

        # Ext_VT_Sel
        off = 73
        txt = self.reg_map[off]['name']
        val = self.reg_map[off]['value'][self.cur_ch_num]
        gain_val = ['EXT_VT_FP', 'EXT_FT_FP']
        #tk.Label(win, anchor=tk.W, text=txt).place(x=10, y=35, width=60,
                #height=25)
        self.Ext_VT_Sel_wid = ttk.Combobox(win, state='readonly',
                values=gain_val)
        self.Ext_VT_Sel_wid.set(gain_val[val])
        self.Ext_VT_Sel_wid.place(x=110, y=5, width=90, height=25)
        self.Ext_VT_Sel_wid.bind("<<ComboboxSelected>>", self._ext_vt_sel_evt)

        # IntVT_Sel
        off = 74
        txt = self.reg_map[off]['name']
        val = self.reg_map[off]['value'][self.cur_ch_num]
        gain_val = ['Local FT%d' % i for i in range(16)]
        #tk.Label(win, anchor=tk.W, text=txt).place(x=10, y=35, width=60,
                #height=25)
        self.IntVT_Sel_wid = ttk.Combobox(win, state='readonly',
                values=gain_val)
        self.IntVT_Sel_wid.set(gain_val[val])
        self.IntVT_Sel_wid.place(x=110, y=35, width=90, height=25)
        self.IntVT_Sel_wid.bind("<<ComboboxSelected>>", self._int_vt_sel_evt)

        txt = 'OR of FTs'
        tk.Label(win, anchor=tk.W, text=txt).place(x=110, y=65, width=90,
                height=25)
        txt = 'LVDS VT In'
        tk.Label(win, anchor=tk.W, text=txt).place(x=110, y=95, width=90,
                height=25)

        # ChanTrig_Sel
        off = 75
        txt = self.reg_map[off]['name']
        val = self.reg_map[off]['value'][self.cur_ch_num]
        gain_val = ['ChanTrig%d' % i for i in range(16)]
        #tk.Label(win, anchor=tk.W, text=txt).place(x=10, y=35, width=60,
                #height=25)
        self.ChanTrig_Selwid = ttk.Combobox(win, state='readonly',
                values=gain_val)
        self.ChanTrig_Selwid.set(gain_val[val])
        self.ChanTrig_Selwid.place(x=110, y=125, width=90, height=25)
        self.ChanTrig_Selwid.bind("<<ComboboxSelected>>",
                self._chan_trig_sel_evt)

        # Ext_FT_In
        win = self.ModTrigSet
        tk.Label(win, bg='yellow', text='Ext_FT_In').place(x=225, y=380,
                width=90, height=20)
        self.Ext_FT_In = tk.Frame(win, height=180, width=210, relief =
                tk.GROOVE, borderwidth = 2)
        self.Ext_FT_In.place(x=225, y=400)
        win = self.Ext_FT_In

        off = 76
        val = self.reg_map[off]['value'][self.cur_ch_num]
        self.Ext_FT_In_val = tk.IntVar()
        self.Ext_FT_In_val.set(val)
        self.Ext_FT_In_wid0 = tk.Radiobutton(win, text="Ext_FT_Sel",
                variable=self.Ext_FT_In_val, value=0, anchor=tk.W,
                command=self._Ext_FT_In_evt)
        self.Ext_FT_In_wid1 = tk.Radiobutton(win, text="IntFT_Sel",
                variable=self.Ext_FT_In_val, value=1, anchor=tk.W,
                command=self._Ext_FT_In_evt)
        self.Ext_FT_In_wid2 = tk.Radiobutton(win, text="FTIn_OR",
                variable=self.Ext_FT_In_val, value=2, anchor=tk.W,
                command=self._Ext_FT_In_evt)
        self.Ext_FT_In_wid3 = tk.Radiobutton(win, text="LVDSFT_FP",
                variable=self.Ext_FT_In_val, value=3, anchor=tk.W,
                command=self._Ext_FT_In_evt)
        self.Ext_FT_In_wid4 = tk.Radiobutton(win, text="ChanTrig_Sel",
                variable=self.Ext_FT_In_val, value=4, anchor=tk.W,
                command=self._Ext_FT_In_evt)
        self.Ext_FT_In_wid0.place(x=2, y=5, width=115, height=25)
        self.Ext_FT_In_wid1.place(x=2, y=35, width=115, height=25)
        self.Ext_FT_In_wid2.place(x=2, y=65, width=115, height=25)
        self.Ext_FT_In_wid3.place(x=2, y=95, width=115, height=25)
        self.Ext_FT_In_wid4.place(x=2, y=125, width=115, height=25)

        # Ext_FT_Sel
        off = 77
        txt = self.reg_map[off]['name']
        val = self.reg_map[off]['value'][self.cur_ch_num]
        gain_val = ['EXT_FT_FP', 'EXT_VT_FP']
        #tk.Label(win, anchor=tk.W, text=txt).place(x=10, y=35, width=60,
                #height=25)
        self.Ext_FT_Sel_wid = ttk.Combobox(win, state='readonly',
                values=gain_val)
        self.Ext_FT_Sel_wid.set(gain_val[val])
        self.Ext_FT_Sel_wid.place(x=110, y=5, width=90, height=25)
        self.Ext_FT_Sel_wid.bind("<<ComboboxSelected>>", self._ext_ft_sel_evt)

        # IntFT_Sel
        off = 78
        txt = self.reg_map[off]['name']
        val = self.reg_map[off]['value'][self.cur_ch_num]
        gain_val = ['Local FT%d' % i for i in range(16)]
        #tk.Label(win, anchor=tk.W, text=txt).place(x=10, y=35, width=60,
                #height=25)
        self.IntFT_Sel_wid = ttk.Combobox(win, state='readonly',
                values=gain_val)
        self.IntFT_Sel_wid.set(gain_val[val])
        self.IntFT_Sel_wid.place(x=110, y=35, width=90, height=25)
        self.IntFT_Sel_wid.bind("<<ComboboxSelected>>", self._int_ft_sel_evt)

        txt = 'OR of FTs'
        tk.Label(win, anchor=tk.W, text=txt).place(x=110, y=65, width=90,
                height=25)
        txt = 'LVDS FT In'
        tk.Label(win, anchor=tk.W, text=txt).place(x=110, y=95, width=90,
                height=25)

        # ChanTrig_Sel
        off = 75
        txt = self.reg_map[off]['name']
        val = self.reg_map[off]['value'][self.cur_ch_num]
        gain_val = ['ChanTrig%d' % i for i in range(16)]
        #tk.Label(win, anchor=tk.W, text=txt).place(x=10, y=35, width=60,
                #height=25)
        self.ChanTrig_Selwid1 = ttk.Combobox(win, state='readonly',
                values=gain_val)
        self.ChanTrig_Selwid1.set(gain_val[val])
        self.ChanTrig_Selwid1.place(x=110, y=125, width=90, height=25)
        self.ChanTrig_Selwid1.config(state=tk.DISABLED)

        # Broadcast Vt and FT
        win = self.ModTrigSet
        tk.Label(win, bg='yellow', text='Broadcast Vt and FT').place(x=440,
                y=380, width=125, height=20)
        self.Broadcast_Vt_FT = tk.Frame(win, height=80, width=145, relief =
                tk.GROOVE, borderwidth = 2)
        self.Broadcast_Vt_FT.place(x=440, y=400)
        win = self.Broadcast_Vt_FT

        # Crate Master
        off = 79
        txt = self.reg_map[off]['name']
        val = self.reg_map[off]['value'][0]
        self.Crate_Master_var = tk.IntVar()
        self.Crate_Master_var.set(val)
        tk.Checkbutton(win, text=txt, variable=self.Crate_Master_var,
                anchor=tk.W, command=self._Crate_Master_evt).place(
                        x=5, y=5, width=130, height=25)
        self.Crate_Master_var.set(val)

        # MulCrate Master
        off = 80
        txt = self.reg_map[off]['name']
        val = self.reg_map[off]['value'][0]
        self.MulCrate_Master_var = tk.IntVar()
        self.MulCrate_Master_var.set(val)
        tk.Checkbutton(win, text=txt, variable=self.MulCrate_Master_var,
                anchor=tk.W, command=self._MulCrate_Master_evt).place(
                        x=5, y=35, width=130, height=25)
        self.MulCrate_Master_var.set(val)

        win = self.ModTrigSet
        # Mod_VT
        off = 81
        txt = self.reg_map[off]['name']
        val = self.reg_map[off]['value'][self.cur_ch_num]
        gain_val = ['Ext_VT_In', 'ET_LC_BP', 'ET_In_BP', 'ET_WireOR']
        tk.Label(win, anchor=tk.W, text=txt).place(x=440, y=500, width=60,
                height=25)
        self.Mod_VT_wid = ttk.Combobox(win, state='readonly', values=gain_val)
        self.Mod_VT_wid.set(gain_val[val])
        self.Mod_VT_wid.place(x=495, y=500, width=90, height=25)
        self.Mod_VT_wid.bind("<<ComboboxSelected>>", self._mod_vt_evt)


        # Mod_FT
        off = 82
        txt = self.reg_map[off]['name']
        val = self.reg_map[off]['value'][self.cur_ch_num]
        gain_val = ['Ext_FT_in', 'FT_LC_BP', 'FT_In_BP', 'FT_WireOr']
        tk.Label(win, anchor=tk.W, text=txt).place(x=440, y=530, width=60,
                height=25)
        self.Mod_FT_wid = ttk.Combobox(win, state='readonly', values=gain_val)
        self.Mod_FT_wid.set(gain_val[val])
        self.Mod_FT_wid.place(x=495, y=530, width=90, height=25)
        self.Mod_FT_wid.bind("<<ComboboxSelected>>", self._mod_ft_evt)

        # FP Output set
        win = self.ModTrigSet
        tk.Label(win, bg='yellow', text='FP Output set').place(x=590, y=380,
                width=90, height=20)
        self.FP_Output_set = tk.Frame(win, height=180, width=180, relief =
                tk.GROOVE, borderwidth = 2)
        self.FP_Output_set.place(x=590, y=400)
        win = self.FP_Output_set

        # TTL FP Ena
        off = 83
        txt = self.reg_map[off]['name']
        val = self.reg_map[off]['value'][self.cur_ch_num]
        gain_val = ['Disable', 'Enable']
        tk.Label(win, anchor=tk.W, text=txt).place(x=5, y=5, width=75,
                height=25)
        self.TTL_FP_wid = ttk.Combobox(win, state='readonly', values=gain_val)
        self.TTL_FP_wid.set(gain_val[val])
        self.TTL_FP_wid.place(x=80, y=5, width=95, height=25)
        self.TTL_FP_wid.bind("<<ComboboxSelected>>", self._TTL_FP_evt)


        # Ch_Sel
        off = 84
        txt = self.reg_map[off]['name']
        val = self.reg_map[off]['value'][self.cur_ch_num]
        gain_val = ['ch%d'%i for i in range(16)]
        tk.Label(win, anchor=tk.W, text=txt).place(x=5, y=35, width=75,
                height=25)
        self.Ch_Sel_wid = ttk.Combobox(win, state='readonly', values=gain_val)
        self.Ch_Sel_wid.set(gain_val[val])
        self.Ch_Sel_wid.place(x=80, y=35, width=95, height=25)
        self.Ch_Sel_wid.bind("<<ComboboxSelected>>", self._Ch_Sel_evt)


        # Output_Sel
        off = 85
        txt = self.reg_map[off]['name']
        val = self.reg_map[off]['value'][self.cur_ch_num]
        gain_val = ['G1', 'G2']
        tk.Label(win, anchor=tk.W, text=txt).place(x=5, y=65, width=75,
                height=25)
        self.Output_Sel_wid = ttk.Combobox(win, state='readonly',
                values=gain_val)
        self.Output_Sel_wid.set(gain_val[val])
        self.Output_Sel_wid.place(x=80, y=65, width=95, height=25)
        self.Output_Sel_wid.bind("<<ComboboxSelected>>", self._Output_Sel_evt)


        # Fo7_sel
        off = 86
        txt = self.reg_map[off]['name']
        val = self.reg_map[off]['value'][self.cur_ch_num]
        gain_val = ['Mod_VT', 'Mod_FT', 'ORDPMFULL', 'SynRdy', 'Ext_FT_FP',
                'Ext_FT_LVDS', 'Ext_VT_IO', 'Ext_VT_LVDS', 'ORChVT']
        tk.Label(win, anchor=tk.W, text=txt).place(x=5, y=95, width=75,
                height=25)
        self.Fo7_sel_wid = ttk.Combobox(win, state='readonly',
                values=gain_val)
        self.Fo7_sel_wid.set(gain_val[val])
        self.Fo7_sel_wid.place(x=80, y=95, width=95, height=25)
        self.Fo7_sel_wid.bind("<<ComboboxSelected>>", self._Fo7_Sel_evt)
        


    def _Fo7_Sel_evt(self, e):
        off = 86
        if self.Fo7_sel_wid.get() == 'Mod_VT':
            val = 0
        elif self.Fo7_sel_wid.get() == 'Mod_FT':
            val = 1
        elif self.Fo7_sel_wid.get() == 'ORDPMFULL':
            val = 2
        elif self.Fo7_sel_wid.get() == 'SynRdy':
            val = 3
        elif self.Fo7_sel_wid.get() == 'Ext_FT_FP':
            val = 4
        elif self.Fo7_sel_wid.get() == 'Ext_FT_LVDS':
            val = 5
        elif self.Fo7_sel_wid.get() == 'Ext_VT_IO':
            val = 6
        elif self.Fo7_sel_wid.get() == 'Ext_VT_LVDS':
            val = 7
        elif self.Fo7_sel_wid.get() == 'ORChVT':
            val = 8
        self.reg_map[off]['value'] = [val for i in range(16)]
        print('new %s selected: ' % self.reg_map[off]['name'])
        print(self.reg_map[off]['value'])


    def _Output_Sel_evt(self, e):
        off = 85
        if self.Output_Sel_wid.get() == 'G1':
            val = 0
        elif self.Output_Sel_wid.get() == 'G2':
            val = 1
        self.reg_map[off]['value'] = [val for i in range(16)]
        print('new %s selected: ' % self.reg_map[off]['name'])
        print(self.reg_map[off]['value'])


    def _Ch_Sel_evt(self, e):
        off = 84
        val = int(self.Ch_Sel_wid.get()[2:])
        self.reg_map[off]['value'] = [val for i in range(16)]
        print('new %s selected: ' % self.reg_map[off]['name'])
        print(self.reg_map[off]['value'])


    def _TTL_FP_evt(self, e):
        off = 83
        if self.TTL_FP_wid.get() == 'Disable':
            val = 0
        elif self.TTL_FP_wid.get() == 'Enable':
            val = 1
        self.reg_map[off]['value'] = [val for i in range(16)]
        print('new %s selected: ' % self.reg_map[off]['name'])
        print(self.reg_map[off]['value'])



    def _mod_ft_evt(self, e):
        off = 82
        if self.Mod_FT_wid.get() == 'Ext_FT_in':
            val = 0
        elif self.Mod_FT_wid.get() == 'FT_LC_BP':
            val = 1
        elif self.Mod_FT_wid.get() == 'FT_In_BP':
            val = 2
        elif self.Mod_FT_wid.get() == 'FT_WireOr':
            val = 3
        self.reg_map[off]['value'] = [val for i in range(16)]
        print('new %s selected: ' % self.reg_map[off]['name'])
        print(self.reg_map[off]['value'])



    def _mod_vt_evt(self, e):
        off = 81
        if self.Mod_VT_wid.get() == 'Ext_VT_In':
            val = 0
        elif self.Mod_VT_wid.get() == 'ET_LC_BP':
            val = 1
        elif self.Mod_VT_wid.get() == 'ET_In_BP':
            val = 2
        elif self.Mod_VT_wid.get() == 'ET_WireOR':
            val = 3
        self.reg_map[off]['value'] = [val for i in range(16)]
        print('new %s selected: ' % self.reg_map[off]['name'])
        print(self.reg_map[off]['value'])




    def _MulCrate_Master_evt(self):
        off = 80
        val = self.MulCrate_Master_var.get()
        self.reg_map[off]['value'] = [val for i in range(16)]
        print('new %s selected: ' % self.reg_map[off]['name'])
        print(self.reg_map[off]['value'])





    def _Crate_Master_evt(self):
        off = 79
        val = self.Crate_Master_var.get()
        self.reg_map[off]['value'] = [val for i in range(16)]
        print('new %s selected: ' % self.reg_map[off]['name'])
        print(self.reg_map[off]['value'])



    def _int_ft_sel_evt(self, e):
        off = 78
        val = int(self.IntFT_Sel_wid.get()[8:])
        self.reg_map[off]['value'] = [val for i in range(16)]
        print('new %s selected: ' % self.reg_map[off]['name'])
        print(self.reg_map[off]['value'])




    def _ext_ft_sel_evt(self, e):
        off = 77
        if self.Ext_FT_Sel_wid.get() == 'EXT_VT_FP':
            val = 1
        elif self.Ext_FT_Sel_wid.get() == 'EXT_FT_FP':
            val = 0
        self.reg_map[off]['value'] = [val for i in range(16)]
        print('new %s selected: ' % self.reg_map[off]['name'])
        print(self.reg_map[off]['value'])




    def _Ext_FT_In_evt(self):
        off = 76
        val = self.Ext_FT_In_val.get()
        self.reg_map[off]['value'] = [val for i in range(16)]
        print('new %s selected: ' % self.reg_map[off]['name'])
        print(self.reg_map[off]['value'])



    def _chan_trig_sel_evt(self, e):
        off = 75
        val = int(self.ChanTrig_Selwid.get()[8:])
        self.reg_map[off]['value'] = [val for i in range(16)]
        print('new %s selected: ' % self.reg_map[off]['name'])
        print(self.reg_map[off]['value'])
        gain_val = ['ChanTrig%d' % i for i in range(16)]
        self.ChanTrig_Selwid1.config(state='readonly')
        self.ChanTrig_Selwid1.set(gain_val[val])
        self.ChanTrig_Selwid1.config(state=tk.DISABLED)




    def _int_vt_sel_evt(self, e):
        off = 74
        val = int(self.IntVT_Sel_wid.get()[8:])
        self.reg_map[off]['value'] = [val for i in range(16)]
        print('new %s selected: ' % self.reg_map[off]['name'])
        print(self.reg_map[off]['value'])



    def _ext_vt_sel_evt(self, e):
        off = 73
        if self.Ext_VT_Sel_wid.get() == 'EXT_VT_FP':
            val = 0
        elif self.Ext_VT_Sel_wid.get() == 'EXT_FT_FP':
            val = 1
        self.reg_map[off]['value'] = [val for i in range(16)]
        print('new %s selected: ' % self.reg_map[off]['name'])
        print(self.reg_map[off]['value'])





    def _Ext_ValTrig_In_evt(self):
        off = 72
        val = self.Ext_ValTrig_In_val.get()
        self.reg_map[off]['value'] = [val for i in range(16)]
        print('new %s selected: ' % self.reg_map[off]['name'])
        print(self.reg_map[off]['value'])




    def _veto_width_evt(self, e):
        off = 71
        val = self._get_rounded_val(self.reg_map[off]['name'],
                float(self.Veto_Width_wid.get()))
        val = int(val*1000)
        self.reg_map[off]['value'][self.cur_ch_num] = val
        self.Veto_Width_wid.delete(0, tk.END)
        self.Veto_Width_wid.insert(0, '%.3f'%(val/1000.))
        print('new %s selected: ' % self.reg_map[off]['name'])
        print(self.reg_map[off]['value'])




    def _modvt_width_evt(self, e):
        off = 70
        val = self._get_rounded_val(self.reg_map[off]['name'],
                float(self.ModVT_Width_wid.get()))
        val = int(val*1000)
        self.reg_map[off]['value'][self.cur_ch_num] = val
        self.ModVT_Width_wid.delete(0, tk.END)
        self.ModVT_Width_wid.insert(0, '%.3f'%(val/1000.))
        print('new %s selected: ' % self.reg_map[off]['name'])
        print(self.reg_map[off]['value'])





    def _chvt_width_evt(self, e):
        off = 69
        val = self._get_rounded_val(self.reg_map[off]['name'],
                float(self.ChVT_Width_wid.get()))
        val = int(val*1000)
        self.reg_map[off]['value'][self.cur_ch_num] = val
        self.ChVT_Width_wid.delete(0, tk.END)
        self.ChVT_Width_wid.insert(0, '%.3f'%(val/1000.))
        print('new %s selected: ' % self.reg_map[off]['name'])
        print(self.reg_map[off]['value'])



    def _fifi_delay_evt(self, e):
        off = 68
        val = self._get_rounded_val(self.reg_map[off]['name'],
                float(self.FIFO_Delay_wid.get()))
        val = int(val*1000)
        self.reg_map[off]['value'][self.cur_ch_num] = val
        self.FIFO_Delay_wid.delete(0, tk.END)
        self.FIFO_Delay_wid.insert(0, '%.3f'%(val/1000.))
        print('new %s selected: ' % self.reg_map[off]['name'])
        print(self.reg_map[off]['value'])



    def _lcft_width_evt(self, e):
        off = 67
        val = self._get_rounded_val(self.reg_map[off]['name'],
                float(self.LCFT_Width_wid.get()))
        val = int(val*1000)
        self.reg_map[off]['value'][self.cur_ch_num] = val
        self.LCFT_Width_wid.delete(0, tk.END)
        self.LCFT_Width_wid.insert(0, '%.3f'%(val/1000.))
        print('new %s selected: ' % self.reg_map[off]['name'])
        print(self.reg_map[off]['value'])



    def _lcft_delay_evt(self, e):
        off = 66
        val = self._get_rounded_val(self.reg_map[off]['name'],
                float(self.LCFT_Delay_wid.get()))
        val = int(val*1000)
        self.reg_map[off]['value'][self.cur_ch_num] = val
        self.LCFT_Delay_wid.delete(0, tk.END)
        self.LCFT_Delay_wid.insert(0, '%.3f'%(val/1000.))
        print('new %s selected: ' % self.reg_map[off]['name'])
        print(self.reg_map[off]['value'])


    def _group3ch_evt(self, e):
        off = 65
        val = self._get_rounded_val(self.reg_map[off]['name'],
                int(self.Group3ch_wid.get()))
        self.reg_map[off]['value'] = [val for i in range(16)]
        self.Group3ch_wid.delete(0, tk.END)
        self.Group3ch_wid.insert(0, str(val))
        print('new %s selected: ' % self.reg_map[off]['name'])
        print(self.reg_map[off]['value'])



    def _group2ch_evt(self, e):
        off = 64
        val = self._get_rounded_val(self.reg_map[off]['name'],
                int(self.Group2ch_wid.get()))
        self.reg_map[off]['value'] = [val for i in range(16)]
        self.Group2ch_wid.delete(0, tk.END)
        self.Group2ch_wid.insert(0, str(val))
        print('new %s selected: ' % self.reg_map[off]['name'])
        print(self.reg_map[off]['value'])




    def _group1ch_evt(self, e):
        off = 63
        val = self._get_rounded_val(self.reg_map[off]['name'],
                int(self.Group1ch_wid.get()))
        self.reg_map[off]['value'] = [val for i in range(16)]
        self.Group1ch_wid.delete(0, tk.END)
        self.Group1ch_wid.insert(0, str(val))
        print('new %s selected: ' % self.reg_map[off]['name'])
        print(self.reg_map[off]['value'])




    def _group0ch_evt(self, e):
        off = 62
        val = self._get_rounded_val(self.reg_map[off]['name'],
                int(self.Group0ch_wid.get()))
        self.reg_map[off]['value'] = [val for i in range(16)]
        self.Group0ch_wid.delete(0, tk.END)
        self.Group0ch_wid.insert(0, str(val))
        print('new %s selected: ' % self.reg_map[off]['name'])
        print(self.reg_map[off]['value'])






    def _group3_evt(self, e):
        off = 61
        if self.Group3_wid.get() == 'ChL':
            val = 2
        elif self.Group3_wid.get() == 'ChM':
            val = 0
        elif self.Group3_wid.get() == 'ChR':
            val = 1
        self.reg_map[off]['value'] = [val for i in range(16)]
        print('new %s selected: ' % self.reg_map[off]['name'])
        print(self.reg_map[off]['value'])



    def _group2_evt(self, e):
        off = 60
        if self.Group2_wid.get() == 'ChL':
            val = 2
        elif self.Group2_wid.get() == 'ChM':
            val = 0
        elif self.Group2_wid.get() == 'ChR':
            val = 1
        self.reg_map[off]['value'] = [val for i in range(16)]
        print('new %s selected: ' % self.reg_map[off]['name'])
        print(self.reg_map[off]['value'])



    def _group1_evt(self, e):
        off = 59
        if self.Group1_wid.get() == 'ChL':
            val = 2
        elif self.Group1_wid.get() == 'ChM':
            val = 0
        elif self.Group1_wid.get() == 'ChR':
            val = 1
        self.reg_map[off]['value'] = [val for i in range(16)]
        print('new %s selected: ' % self.reg_map[off]['name'])
        print(self.reg_map[off]['value'])






    def _group0_evt(self, e):
        off = 58
        if self.Group0_wid.get() == 'ChL':
            val = 2
        elif self.Group0_wid.get() == 'ChM':
            val = 0
        elif self.Group0_wid.get() == 'ChR':
            val = 1
        self.reg_map[off]['value'] = [val for i in range(16)]
        print('new %s selected: ' % self.reg_map[off]['name'])
        print(self.reg_map[off]['value'])




    def _cointhr_evt(self, e):
        off = 57
        val = self._get_rounded_val(self.reg_map[off]['name'],
                int(self.CoinThR_wid.get()))
        self.reg_map[off]['value'][self.cur_ch_num] = val
        self.CoinThR_wid.delete(0, tk.END)
        self.CoinThR_wid.insert(0, str(val))
        print('new %s selected: ' % self.reg_map[off]['name'])
        print(self.reg_map[off]['value'])




    def _cointhm_evt(self, e):
        off = 56
        val = self._get_rounded_val(self.reg_map[off]['name'],
                int(self.CoinThM_wid.get()))
        self.reg_map[off]['value'][self.cur_ch_num] = val
        self.CoinThM_wid.delete(0, tk.END)
        self.CoinThM_wid.insert(0, str(val))
        print('new %s selected: ' % self.reg_map[off]['name'])
        print(self.reg_map[off]['value'])



    def _cointhl_evt(self, e):
        off = 55
        val = self._get_rounded_val(self.reg_map[off]['name'],
                int(self.CoinThL_wid.get()))
        self.reg_map[off]['value'][self.cur_ch_num] = val
        self.CoinThL_wid.delete(0, tk.END)
        self.CoinThL_wid.insert(0, str(val))
        print('new %s selected: ' % self.reg_map[off]['name'])
        print(self.reg_map[off]['value'])




    def _multh_evt(self, e):
        off = 54
        val = self._get_rounded_val(self.reg_map[off]['name'],
                int(self.MulTh_wid.get()))
        self.reg_map[off]['value'][self.cur_ch_num] = val
        self.MulTh_wid.delete(0, tk.END)
        self.MulTh_wid.insert(0, str(val))
        print('new %s selected: ' % self.reg_map[off]['name'])
        print(self.reg_map[off]['value'])




    def _maskr_evt(self, e):
        off = 53
        val = self._get_rounded_val(self.reg_map[off]['name'],
                int('0x'+self.MaskR_wid.get(), base=16))
        self.reg_map[off]['value'][self.cur_ch_num] = val
        self.MaskR_wid.delete(0, tk.END)
        self.MaskR_wid.insert(0, '%04x'%val)
        print('new %s selected: ' % self.reg_map[off]['name'])
        print(self.reg_map[off]['value'])





    def _maskm_evt(self, e):
        off = 52
        val = self._get_rounded_val(self.reg_map[off]['name'],
                int('0x'+self.MaskM_wid.get(), base=16))
        self.reg_map[off]['value'][self.cur_ch_num] = val
        self.MaskM_wid.delete(0, tk.END)
        self.MaskM_wid.insert(0, '%04x'%val)
        print('new %s selected: ' % self.reg_map[off]['name'])
        print(self.reg_map[off]['value'])





    def _maskl_evt(self, e):
        off = 51
        val = self._get_rounded_val(self.reg_map[off]['name'],
                int('0x'+self.MaskL_wid.get(), base=16))
        self.reg_map[off]['value'][self.cur_ch_num] = val
        self.MaskL_wid.delete(0, tk.END)
        self.MaskL_wid.insert(0, '%04x'%val)
        print('new %s selected: ' % self.reg_map[off]['name'])
        print(self.reg_map[off]['value'])




    def _ChanTrig_ChVT_evt(self):
        off = 50
        val = self.ChanTrig_ChVT_val.get()
        self.reg_map[off]['value'][self.cur_ch_num] = val
        print('new %s selected: ' % self.reg_map[off]['name'])
        print(self.reg_map[off]['value'])




    def _modveto_sel_evt(self, e):
        off = 49
        if self.ModVeto_wid.get() == 'Mod_VT':
            val = 1
        elif self.ModVeto_wid.get() == 'ModIn_FP':
            val = 0
        self.reg_map[off]['value'][self.cur_ch_num] = val
        print('new %s selected: ' % self.reg_map[off]['name'])
        print(self.reg_map[off]['value'])




    def _chveto_sel_evt(self, e):
        off = 48
        if self.ChVeto_sel_wid.get() == 'Ch_VT':
            val = 1
        elif self.ChVeto_sel_wid.get() == 'ChInFP':
            val = 0
        self.reg_map[off]['value'][self.cur_ch_num] = val
        print('new %s selected: ' % self.reg_map[off]['name'])
        print(self.reg_map[off]['value'])




    def _Veto_evt(self):
        off = 47
        self.reg_map[off]['value'][self.cur_ch_num] = self.Veto_var.get()
        print('new %s selected: ' % self.reg_map[off]['name'])
        print(self.reg_map[off]['value'])





    def _modvt_sel_evt(self, e):
        off = 46
        if self.ModVT_sel_wid.get() == 'Mod_VT':
            val = 0
        elif self.ModVT_sel_wid.get() == 'ModInFP':
            val = 1
        self.reg_map[off]['value'][self.cur_ch_num] = val
        print('new %s selected: ' % self.reg_map[off]['name'])
        print(self.reg_map[off]['value'])





    def _ModVT_evt(self):
        off = 45
        self.reg_map[off]['value'][self.cur_ch_num] = self.ModVT_var.get()
        print('new %s selected: ' % self.reg_map[off]['name'])
        print(self.reg_map[off]['value'])






    def _chvt_sel_evt(self, e):
        off = 44
        if self.ChVT_sel_wid.get() == 'ChanTrig':
            val = 0
        elif self.ChVT_sel_wid.get() == 'ChInFP':
            val = 1
        self.reg_map[off]['value'][self.cur_ch_num] = val
        print('new %s selected: ' % self.reg_map[off]['name'])
        print(self.reg_map[off]['value'])





    def _ChVT_evt(self):
        off = 43
        self.reg_map[off]['value'][self.cur_ch_num] = self.ChVT_var.get()
        print('new %s selected: ' % self.reg_map[off]['name'])
        print(self.reg_map[off]['value'])




    def _chft_evt(self, e):
        off = 42
        if self.ChFT_wid.get() == 'Local FT':
            val = 0
        elif self.ChFT_wid.get() == 'Mod FT':
            val = 1
        elif self.ChFT_wid.get() == 'Ch VT':
            val = 2
        self.reg_map[off]['value'][self.cur_ch_num] = val
        print('new %s selected: ' % self.reg_map[off]['name'])
        print(self.reg_map[off]['value'])




    def _ftbpr_evt(self, e):
        off = 41
        val = self._get_rounded_val(self.reg_map[off]['name'],
                int('0x'+self.FTBPEna_r_wid.get(), base=16))
        self.reg_map[off]['value'] = [val for i in range(16)]
        self.FTBPEna_r_wid.delete(0, tk.END)
        self.FTBPEna_r_wid.insert(0, '%04x'%val)
        print('new %s selected: ' % self.reg_map[off]['name'])
        print(self.reg_map[off]['value'])




    def _ftbpl_evt(self, e):
        off = 40
        val = self._get_rounded_val(self.reg_map[off]['name'],
                int('0x'+self.FTBPEna_l_wid.get(), base=16))
        self.reg_map[off]['value'] = [val for i in range(16)]
        self.FTBPEna_l_wid.delete(0, tk.END)
        self.FTBPEna_l_wid.insert(0, '%04x'%val)
        print('new %s selected: ' % self.reg_map[off]['name'])
        print(self.reg_map[off]['value'])






    def _init_layouts(self):
        self.win = tk.Toplevel()
        self.win.geometry('800x650')
        self.notebook_frm = tk.Frame(self.win)
        self.notebook_frm.pack()
        self.tab_ctrl = ttk.Notebook(self.notebook_frm, width=800, height=600)

        # create the frontend tab
        self._create_ModParSet(self.tab_ctrl)

        # create the event builder tab
        self._create_ModTrigSet(self.tab_ctrl)



        #self.win = tk.Toplevel()
        #self.frm = tk.Frame(self.win, width=800, height=600)
        #self.frm.pack()
        #common settings 

        # The 'OK' and 'Cancel' buttons
        self.stat = 'cancel'
        self.butt_ok = tk.Button(self.win, text='OK', command=self._OK)
        self.butt_cl = tk.Button(self.win, text='Cancel', command=self._cancel)
        self.butt_ok.place(x=0, y=630, width=60, height=25)
        self.butt_cl.place(x=80, y=630, width=60, height=25)

    
    # Callback functions of the scrollbars
    def _frm_set_conf(self, evt):
        self.canv_set.configure(scrollregion=self.canv_set.bbox('all'))
    def _frm_reg_conf(self, evt):
        self.canv_reg.configure(scrollregion=self.canv_reg.bbox('all'))

    def _OK(self):
        self.stat = 'OK'
        self.win.destroy()
    def _cancel(self):
        self.stat = 'cancel'
        self.win.destroy()


    # init the widgets for module specific settings, here we initialize the
    # widgets according to the reg_map variable. 
    def _init_set(self):
        # place all the labels and entry(or combobox or other..) accoring to
        # the reg_map
        self.set_labels = []
        self.reg_labels = []
        for reg in self.reg_map:
            if not reg['has_set_wid']:
                # this register doesn't want the base class to create the
                # widgets for it, the derived class will do that.
                continue
            tmp_lable_wid = tk.Label(self.frm_set, bg='green', text=reg['name'])
            self.set_labels.append(tmp_lable_wid)
            self._place_win_set(tmp_lable_wid)
            if reg['set_wid_type'] == 'entry':
                win = tk.Entry(self.frm_set)
            elif reg['set_wid_type'] == 'comb':
                win = ttk.Combobox(self.frm_set, state='readonly',
                        values=reg['set_wid_values'])
            self._place_win_set(win)
            reg['set_wid'] = win

        # update the values of the widgets according to the settings
        self._update_set_wid()
        # place a button
        self.app_set_butt = tk.Button(self.frm_set, text='Apply', 
                command=self._app_set)
        self._place_win_set(self.app_set_butt)

    def _init_reg(self):
        # place the labels and entry boxes.
        for reg in self.reg_map:
            tmp_wid = tk.Label(self.frm_reg, bg='green', 
                    text=('0x%04x' % reg['off']))
            self.reg_labels.append(tmp_wid)
            self._place_win_reg(tmp_wid)
            win = ttk.Entry(self.frm_reg)
            self._place_win_reg(win)
            reg['reg_wid'] = win
        
        # update the values of the widgets according to the settings
        self._update_reg_wid()
        # place a button
        self.app_reg_butt = tk.Button(self.frm_reg, text='Apply', 
                command=self._app_reg)
        self._place_win_reg(self.app_reg_butt)


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

    # call back function of the apply buttons
    def _app_set(self):
        self._update_data_set()
        self._update_reg_wid() 
        self._update_set_wid() 
        self._show_msg('changes applied!')
    def _app_reg(self):
        self._update_data_reg()
        self._update_reg_wid() 
        self._update_set_wid()
        self._show_msg('changes applied!')
    def _app_com(self):
        # crate number
        self.crate = int(self.crate_w.get())
        # slot number
        self.slot = int(self.slot_w.get())
        # base address
        tmp = self.base_w.get()
        val, flag = self._str2int(tmp)
        if flag == 'ERR':
            self._show_msg('invilid base address!', 'err')
        else:
            self.base = val
        #address modifiers for register access
        tmp = self.am_reg_w.get()
        if tmp == self.AM[0xFFFF]:
            self._show_msg('invalid address modifier', 'err')
        for am, txt in self.AM.items():
            if txt == tmp:
                self.am_reg = am
                break
        # type (trigger or scaler)
        tmp = self.type_w.get()
        if tmp.startswith('T'):
            self.type = 'T'
        else:
            self.type = 'S'

        # period to read scaler-type modules
        tmp = self.period_w.get()
        val, flag = self._str2int(tmp)
        if flag == 'ERR':
            self._show_msg('invilid period!', 'err')
        else:
            self.period = val

        self._show_msg('modifications applied')

    # init the widgets for common settings based on their values
    def _init_com(self):
        # crate number
        tk.Label(self.frm_com, bg='green', 
                text='Crate #').place(x=10, y=10, width=50, height=25)
        crates = []
        for i in range(self.max_crate):
            crates.append('%d' % i)
        self.crate_w = ttk.Combobox(self.frm_com, state='readonly', 
                values=crates)
        self.crate_w.set(crates[self.crate])
        self.crate_w.place(x=10, y=35, width=50, height=25)

        # slot number
        tk.Label(self.frm_com, bg='green', 
                text='Slot #').place(x=70, y=10, width=50, height=25)
        slots = []
        for i in range(21):
            slots.append('%d' % i)
        self.slot_w = ttk.Combobox(self.frm_com, state='readonly', 
                values=slots)
        self.slot_w.set(slots[self.slot])
        self.slot_w.place(x=70, y=35, width=50, height=25)

        # base address
        tk.Label(self.frm_com, bg='green', 
                text='base addr').place(x=130, y=10, width=100, height=25)
        self.base_w = tk.Entry(self.frm_com)
        self.base_w.insert(0, '0x%08x' % self.base)
        self.base_w.place(x=130, y=35, width=100, height=25)

        #address modifiers for register access
        tk.Label(self.frm_com, bg='green', 
                text='am').place(x=250, y=10, width=100, height=25)
        self.AM = {
             0x2D : "0x2D A16_S       ",
             0x29 : "0x29 A16_U       ",
             0x2C : "0x2C A16_LCK     ",
             0x3F : "0x3F A24_S_BLT   ",
             0x3E : "0x3E A24_S_PGM   ",
             0x3D : "0x3D A24_S_DATA  ",
             0x3C : "0x3C A24_S_MBLT  ",
             0x3B : "0x3B A24_U_BLT   ",
             0x3A : "0x3A A24_U_PGM   ",
             0x39 : "0x39 A24_U_DATA  ",
             0x38 : "0x38 A24_U_MBLT  ",
             0x32 : "0x32 A24_LCK     ",
             0x0F : "0x0F A32_S_BLT   ",
             0x0E : "0x0E A32_S_PGM   ",
             0x0D : "0x0D A32_S_DATA  ",
             0x0C : "0x0C A32_S_MBLT  ",
             0x0B : "0x0B A32_U_BLT   ",
             0x0A : "0x0A A32_U_PGM   ",
             0x09 : "0x09 A32_U_DATA  ",
             0x08 : "0x08 A32_U_MBLT  ",
             0x05 : "0x05 A32_LCK     ",
             0x2F : "0x2F CR_CSR      ",
             0x37 : "0x37 A40_BLT     ",
             0x35 : "0x35 A40_LCK     ",
             0x34 : "0x34 A40         ",
             0x01 : "0x01 A64         ",
             0x03 : "0x03 A64_BLT     ",
             0x00 : "0x00 A64_MBLT    ",
             0x04 : "0x04 A64_LCK     ",
             0x21 : "0x21 A3U_2eVME   ",
             0x20 : "0x20 A6U_2eVME   ",
             0xFFFF:"NOT VALID        "}
        tmp = list(self.AM.values())
        tmp.sort()
        self.am_reg_w = ttk.Combobox(self.frm_com, state='readonly', values=tmp)
        if self.am_reg in self.AM:
            value = self.AM[self.am_reg]
        else:
            value = self.AM[0xFFFF]
        self.am_reg_w.set(value)
        self.am_reg_w.place(x=250, y=35, width=150, height=25)

        # type (trigger or scaler)
        tk.Label(self.frm_com, bg='green', 
                text='type(T/S)').place(x=410, y=10, width=100, height=25)
        types = ['T (trigger)', 'S (scaler)']
        self.type_w = ttk.Combobox(self.frm_com, state='readonly', 
                values=types)
        value = types[0]
        if self.type == 'S' or self.type == 's':
            value = types[1]
        self.type_w.set(value)
        self.type_w.place(x=410, y=35, width=100, height=25)

        # reading period for scaler-type modules
        tk.Label(self.frm_com, bg='green', 
                text='period').place(x=520, y=10, width=50, height=25)
        self.period_w = tk.Entry(self.frm_com)
        self.period_w.insert(0, str(self.period))
        self.period_w.place(x=520, y=35, width=50, height=25)

        # the 'apply' button
        self.app_com_butt = tk.Button(self.frm_com, text='Apply', 
                command=self._app_com)
        self.app_com_butt.place(x=600, y=35, width=100, height=25)
        
        # the 'advanced...' button
        self.adv_butt = tk.Button(self.frm_com, text='advanced...', 
                command=self._adv_conf)
        self.adv_butt.place(x=600, y=65, width=100, height=25)


    def _adv_conf(self):
        self.adv_conf.show_win()
        self.adv_conf.get_win().grab_set()
        self.win.wait_window(self.adv_conf.get_win())
        self.win.grab_set()
    # determin the value for the corresponding widget in the settings area
    def _get_set_wid_val(self, reg):
        value = reg['value']
        if value == 'default':
            return 'default'
        value &= ((1 << reg['nbit']) - 1)
        if reg['set_wid_type'] == 'entry':
            return '%d' % value 
        elif reg['set_wid_type'] == 'comb':
            return reg['set_wid_values'][value]

    # determin the value for the corresponding widget in the registers area
    def _get_reg_wid_val(self, reg):
        value = reg['value']
        if value == 'default':
            return 'default'
        if reg['nbit'] <= 16:
            return '0x%04x' % value
        else:
            return '0x%08x' % value

    # update the widgets in the module specific settings area accoring to its
    # register values.
    def _update_set_wid(self):
        for reg in self.reg_map:
            if not reg['has_set_wid']:
                # the derived class will update it because it is special and
                # not supposed to be updated here
                continue
            fun = self._get_set_wid_val
            if reg['get_set_wid_val']:
                fun = reg['get_set_wid_val']
            val = fun(reg)
            if reg['set_wid_type'] == 'entry':
                reg['set_wid'].delete(0, tk.END)
                reg['set_wid'].insert(0, val)
            elif reg['set_wid_type'] == 'comb':
                reg['set_wid'].set(val)
            # if the register has non-default values, change the color of the
            # lable.
            tmp_wid = self._get_label_wid(reg['name'])
            if val != 'default':
                 tmp_wid.config(bg='yellow')
            else:
                 tmp_wid.config(bg='green')


    # get the label widget with the given name
    def _get_label_wid(self, label_name):
        for label_wid in self.set_labels + self.reg_labels:
            if label_wid.config()['text'][-1] == label_name:
                return label_wid
        # to avoid exception:
        return self.set_labels[0]



    # update the widgets in the module specific registers area accoring to its
    # register values.
    def _update_reg_wid(self):
        for reg in self.reg_map:
            fun = self._get_reg_wid_val
            if reg['get_reg_wid_val']:
                fun = reg['get_reg_wid_val']
            val = fun(reg)
            reg['reg_wid'].delete(0, tk.END)
            reg['reg_wid'].insert(0, val)
            # if the register has non-default values, change the color of the
            # lable.
            tmp_wid = self._get_label_wid('0x%04x'%reg['off'])
            if val != 'default':
                 tmp_wid.config(bg='yellow')
            else:
                 tmp_wid.config(bg='green')

    # update the internale configuration data accoring to the widgets in the
    # module specific settings area 
    def _update_data_set(self):
        for reg in self.reg_map:
            if not reg['has_set_wid']:
                # the derived subclass will update, we will skip here.
                continue
            fun = self._str2int
            if reg['str2int']:
                fun = reg['str2int']
            if reg['set_wid_type'] == 'entry':
                val,flag = fun(reg['set_wid'].get())
                if flag == 'DEF':
                    reg['value'] = 'default'
                elif flag == 'ERR':
                    self._show_msg('invilid value for %s' % reg['name'], 'err')
                else:
                    reg['value'] = val
            elif reg['set_wid_type'] == 'comb':
                values = reg['set_wid_values']
                selected = reg['set_wid'].get()
                if selected == 'default':
                    reg['value'] = 'default'
                else:
                    reg['value'] = values.index(selected)
    
    # update the internal configuration data accoring widgets in the module
    # specific registers area 
    def _update_data_reg(self):
        for reg in self.reg_map:
            tmp = reg['reg_wid'].get()
            fun = self._str2int
            if reg['str2int']:
                fun = reg['str2int']
            val,flag = fun(tmp)
            if flag == 'ERR':
                self._show_msg('invilid value for register at 0x%04x' % reg['off'], 'err')
            elif flag == 'DEF':
                reg['value'] = 'default'
            else:
                reg['value'] = val

    # find the index of a register (by its address) in the reg_map. 
    # return the index, return -1 if not found.
    def _find_reg(self, addr):
        for i,reg in enumerate(self.reg_map):
            if reg['off'] == addr:
                return i,reg
        return -1,None

    # pops up a messagebox to show the message:
    def _show_msg(self, msg, flag='info'):
        if flag == 'err':
            messagebox.showerror('error', msg, parent=self.frm)
        else:
            messagebox.showinfo('info', msg, parent=self.frm)

    # check if all the settings are appropriate:
    def check(self):
        pass # to be implemented by subclasses
        return 'OK' 

    def _init_reg_map(self):
        # some of the register values are of float type. The initial design of
        # the register map requires that all the register values representable
        # by 64-bit integers. In the GUI, the allowed accuracy for the
        # float-type numbers are 0.001, therefore, we convert them to integers
        # by expressions like 'int(float_val*1000)' or backwords:
        # 'float(int_val)/1000'
        self.reg_map = []
        self.reg_map.append({
                 'off'    : 0,
                 'value'  : [0 for i in range(16)],
                 'name'   : 'ModNum',
                 'comment': 'Module number (starts from 0)'
                 })
        self.reg_map.append({
                 'off'    : 1,
                 'value'  : [2 for i in range(16)],
                 'name'   : 'E_Filter_Range',
                 'comment': 'Energy filter range'
                 })
        self.reg_map.append({
                 'off'    : 2,
                 'value'  : [0 for i in range(16)],
                 'name'   : 'T_Filter_Range',
                 'comment': 'Time filter range'
                 })
        self.reg_map.append({
                 'off'    : 3,
                 'value'  : [0 for i in range(16)],
                 'name'   : 'CPLD',
                 'comment': 'CPLD'
                 })
        self.reg_map.append({
                 'off'    : 4,
                 'value'  : [0 for i in range(16)],
                 'name'   : 'SortEvent',
                 'comment': 'SortEvent'
                 })
        self.reg_map.append({
                 'off'    : 5,
                 'value'  : [0 for i in range(16)],
                 'name'   : 'InhibitEna',
                 'comment': 'InhibitEna'
                 })
        self.reg_map.append({
                 'off'    : 6,
                 'value'  : [0 for i in range(16)],
                 'name'   : 'MulCrate Enable',
                 'comment': 'MulCrate Enable'
                 })
        self.reg_map.append({
                 'off'    : 7,
                 'value'  : [60 for i in range(16)],
                 'name'   : 'XDT (us)',
                 'comment': 'XDT (us)',
                 # need an initial xwait value (1000 == 10.0 us)
                 'xwait'  : [6 for i in range(16)]
                 })
        self.reg_map.append({
                 'off'    : 8,
                 'value'  : [0 for i in range(16)],
                 'name'   : 'Good Ch',
                 'comment': 'Good Ch'
                 })
        self.reg_map.append({
                 'off'    : 9,
                 'value'  : [1 for i in range(16)],
                 'name'   : 'POL',
                 'comment': 'POL'
                 })
        self.reg_map.append({
                 'off'    : 10,
                 'value'  : [1 for i in range(16)],
                 'name'   : 'Gain',
                 'comment': 'Gain'
                 })
        self.reg_map.append({
                 'off'    : 11,
                 'value'  : [10 for i in range(16)],
                 'name'   : 'Offset(%)',
                 'comment': 'Offset(%)'
                 })
        self.reg_map.append({
                 'off'    : 12,
                 'value'  : [0.1*1000 for i in range(16)],
                 'name'   : 'T_Risetime(us)',
                 'comment': 'T_Risetime(us)'
                 })
        self.reg_map.append({
                 'off'    : 13,
                 'value'  : [0.1*1000 for i in range(16)],
                 'name'   : 'T_FlatTop(us)',
                 'comment': 'T_FlatTop(us)'
                 })
        self.reg_map.append({
                 'off'    : 14,
                 'value'  : [30 for i in range(16)],
                 'name'   : 'T_Thresh',
                 'comment': 'T_Thresh'
                 })
        self.reg_map.append({
                 'off'    : 15,
                 'value'  : [5.04*1000 for i in range(16)],
                 'name'   : 'E_Risetime(us)',
                 'comment': 'E_Risetime(us)',
                 'paf_len': [1656 for i in range(16)],
                 'trig_dl': [656 for i in range(16)],
                 'peak_sample': [81 for i in range(16)],
                 'peak_sep'   : [83 for i in range(16)],
                 })
        self.reg_map.append({
                 'off'    : 16,
                 'value'  : [1.6*1000 for i in range(16)],
                 'name'   : 'E_FlatTop(us)',
                 'comment': 'E_FlatTop(us)'
                 })
        self.reg_map.append({
                 'off'    : 17,
                 'value'  : [46.25*1000 for i in range(16)],
                 'name'   : 'Tau(us)',
                 'comment': 'Tau(us)'
                 })
        self.reg_map.append({
                 'off'    : 18,
                 'value'  : [0 for i in range(16)],
                 'name'   : 'PileUp',
                 'comment': 'PileUp'
                 })
        self.reg_map.append({
                 'off'    : 19,
                 'value'  : [0 for i in range(16)],
                 'name'   : 'SyncData',
                 'comment': 'SyncData'
                 })
        self.reg_map.append({
                 'off'    : 20,
                 'value'  : [0 for i in range(16)],
                 'name'   : 'EnExtTS',
                 'comment': 'EnExtTS'
                 })
        self.reg_map.append({
                 'off'    : 21,
                 'value'  : [1 for i in range(16)],
                 'name'   : 'ESumEna',
                 'comment': 'ESumEna'
                 })
        self.reg_map.append({
                 'off'    : 22,
                 'value'  : [1 for i in range(16)],
                 'name'   : 'CFDEna',
                 'comment': 'CFDEna'
                 })
        self.reg_map.append({
                 'off'    : 23,
                 'value'  : [0 for i in range(16)],
                 'name'   : 'CFD_Scaler',
                 'comment': 'CFD_Scaler'
                 })
        self.reg_map.append({
                 'off'    : 24,
                 'value'  : [0.08*1000 for i in range(16)],
                 'name'   : 'CFD_Dealy(us)',
                 'comment': 'CFD_Dealy(us)'
                 })
        self.reg_map.append({
                 'off'    : 25,
                 'value'  : [120 for i in range(16)],
                 'name'   : 'CFD_Threshol',
                 'comment': 'CFD_Threshol'
                 })
        self.reg_map.append({
                 'off'    : 26,
                 'value'  : [1 for i in range(16)],
                 'name'   : 'RecordTrace',
                 'comment': 'RecordTrace'
                 })
        self.reg_map.append({
                 'off'    : 27,
                 'value'  : [10*1000 for i in range(16)],
                 'name'   : 'Trace_PreTrig(us)',
                 'comment': 'Trace_PreTrig(us)'
                 })
        self.reg_map.append({
                 'off'    : 28,
                 'value'  : [50*1000 for i in range(16)],
                 'name'   : 'Trace_Length(us)',
                 'comment': 'Trace_Length(us)'
                 })
        self.reg_map.append({
                 'off'    : 29,
                 'value'  : [1 for i in range(16)],
                 'name'   : 'RecordQDC',
                 'comment': 'RecordQDC'
                 })
        self.reg_map.append({
                 'off'    : 30,
                 'value'  : [0.3*1000 for i in range(16)],
                 'name'   : 'QDC Len0(us)',
                 'comment': 'QDC Len0(us)'
                 })
        self.reg_map.append({
                 'off'    : 31,
                 'value'  : [0.63*1000 for i in range(16)],
                 'name'   : 'QDC Len1(us)',
                 'comment': 'QDC Len1(us)'
                 })
        self.reg_map.append({
                 'off'    : 32,
                 'value'  : [0.88*1000 for i in range(16)],
                 'name'   : 'QDC Len2(us)',
                 'comment': 'QDC Len2(us)'
                 })
        self.reg_map.append({
                 'off'    : 33,
                 'value'  : [1.13*1000 for i in range(16)],
                 'name'   : 'QDC Len3(us)',
                 'comment': 'QDC Len3(us)'
                 })
        self.reg_map.append({
                 'off'    : 34,
                 'value'  : [1.38*1000 for i in range(16)],
                 'name'   : 'QDC Len4(us)',
                 'comment': 'QDC Len4(us)'
                 })
        self.reg_map.append({
                 'off'    : 35,
                 'value'  : [1.63*1000 for i in range(16)],
                 'name'   : 'QDC Len5(us)',
                 'comment': 'QDC Len5(us)'
                 })
        self.reg_map.append({
                 'off'    : 36,
                 'value'  : [1.88*1000 for i in range(16)],
                 'name'   : 'QDC Len6(us)',
                 'comment': 'QDC Len6(us)'
                 })
        self.reg_map.append({
                 'off'    : 37,
                 'value'  : [2.13*1000 for i in range(16)],
                 'name'   : 'QDC Len7(us)',
                 'comment': 'QDC Len7(us)'
                 })
        self.reg_map.append({
                 'off'    : 38,
                 'value'  : [5 for i in range(16)],
                 'name'   : 'BLCUT',
                 'comment': 'BLCUT'
                 })
        self.reg_map.append({
                 'off'    : 39,
                 'value'  : [i for i in range(16)],
                 'name'   : 'ChNum',
                 'comment': 'ChNum'
                 })
        self.reg_map.append({
                 'off'    : 40,
                 'value'  : [0 for i in range(16)],
                 'name'   : 'FTBPEna_l',
                 'comment': 'FTBPEna_l'
                 })
        self.reg_map.append({
                 'off'    : 41,
                 'value'  : [0 for i in range(16)],
                 'name'   : 'FTBPEna_r',
                 'comment': 'FTBPEna_r'
                 })
        self.reg_map.append({
                 'off'    : 42,
                 'value'  : [0 for i in range(16)],
                 'name'   : 'ChFT',
                 'comment': 'ChFT'
                 })
        self.reg_map.append({
                 'off'    : 43,
                 'value'  : [0 for i in range(16)],
                 'name'   : 'ChVT',
                 'comment': 'ChVT'
                 })
        self.reg_map.append({
                 'off'    : 44,
                 'value'  : [0 for i in range(16)],
                 'name'   : 'ChVT_sel',
                 'comment': 'ChVT_sel'
                 })
        self.reg_map.append({
                 'off'    : 45,
                 'value'  : [0 for i in range(16)],
                 'name'   : 'ModVT',
                 'comment': 'ModVT'
                 })
        self.reg_map.append({
                 'off'    : 46,
                 'value'  : [0 for i in range(16)],
                 'name'   : 'ModVT_sel',
                 'comment': 'ModVT_sel'
                 })
        self.reg_map.append({
                 'off'    : 47,
                 'value'  : [0 for i in range(16)],
                 'name'   : 'Veto',
                 'comment': 'Veto'
                 })
        self.reg_map.append({
                 'off'    : 48,
                 'value'  : [0 for i in range(16)],
                 'name'   : '1.ChVeto',
                 'comment': '1.ChVeto'
                 })
        self.reg_map.append({
                 'off'    : 49,
                 'value'  : [0 for i in range(16)],
                 'name'   : '2.ModVeto',
                 'comment': '2.ModVeto'
                 })
        self.reg_map.append({
                 'off'    : 50,
                 'value'  : [1 for i in range(16)],
                 'name'   : 'ChanTrig_ChVT',
                 'comment': 'ChanTrig_ChVT'
                 })
        self.reg_map.append({
                 'off'    : 51,
                 'value'  : [0 for i in range(16)],
                 'name'   : 'MaskL 0x',
                 'comment': 'MaskL 0x'
                 })
        self.reg_map.append({
                 'off'    : 52,
                 'value'  : [0 for i in range(16)],
                 'name'   : 'MaskM 0x',
                 'comment': 'MaskM 0x'
                 })
        self.reg_map.append({
                 'off'    : 53,
                 'value'  : [0 for i in range(16)],
                 'name'   : 'MaskR 0x',
                 'comment': 'MaskR 0x'
                 })
        self.reg_map.append({
                 'off'    : 54,
                 'value'  : [0 for i in range(16)],
                 'name'   : 'MulTh',
                 'comment': 'MulTh'
                 })
        self.reg_map.append({
                 'off'    : 55,
                 'value'  : [0 for i in range(16)],
                 'name'   : 'CoinThL',
                 'comment': 'CoinThL'
                 })
        self.reg_map.append({
                 'off'    : 56,
                 'value'  : [0 for i in range(16)],
                 'name'   : 'CoinThM',
                 'comment': 'CoinThM'
                 })
        self.reg_map.append({
                 'off'    : 57,
                 'value'  : [0 for i in range(16)],
                 'name'   : 'CoinThR',
                 'comment': 'CoinThR'
                 })
        self.reg_map.append({
                 'off'    : 58,
                 'value'  : [0 for i in range(16)],
                 'name'   : 'Group0',
                 'comment': 'Group0'
                 })
        self.reg_map.append({
                 'off'    : 59,
                 'value'  : [0 for i in range(16)],
                 'name'   : 'Group1',
                 'comment': 'Group1'
                 })
        self.reg_map.append({
                 'off'    : 60,
                 'value'  : [0 for i in range(16)],
                 'name'   : 'Group2',
                 'comment': 'Group2'
                 })
        self.reg_map.append({
                 'off'    : 61,
                 'value'  : [0 for i in range(16)],
                 'name'   : 'Group3',
                 'comment': 'Group3'
                 })
        self.reg_map.append({
                 'off'    : 62,
                 'value'  : [0 for i in range(16)],
                 'name'   : 'Group0ch',
                 'comment': 'Group0ch'
                 })
        self.reg_map.append({
                 'off'    : 63,
                 'value'  : [0 for i in range(16)],
                 'name'   : 'Group1ch',
                 'comment': 'Group1ch'
                 })
        self.reg_map.append({
                 'off'    : 64,
                 'value'  : [0 for i in range(16)],
                 'name'   : 'Group2ch',
                 'comment': 'Group2ch'
                 })
        self.reg_map.append({
                 'off'    : 65,
                 'value'  : [0 for i in range(16)],
                 'name'   : 'Group3ch',
                 'comment': 'Group3ch'
                 })
        self.reg_map.append({
                 'off'    : 66,
                 'value'  : [0 for i in range(16)],
                 'name'   : 'LCFT Delay',
                 'comment': 'LCFT Delay'
                 })
        self.reg_map.append({
                 'off'    : 67,
                 'value'  : [0.1*1000 for i in range(16)],
                 'name'   : 'LCFT Width',
                 'comment': 'LCFT Width'
                 })
        self.reg_map.append({
                 'off'    : 68,
                 'value'  : [0.2*1000 for i in range(16)],
                 'name'   : 'FIFO Delay',
                 'comment': 'FIFO Delay'
                 })
        self.reg_map.append({
                 'off'    : 69,
                 'value'  : [0 for i in range(16)],
                 'name'   : 'ChVT Width',
                 'comment': 'ChVT Width'
                 })
        self.reg_map.append({
                 'off'    : 70,
                 'value'  : [1.5*1000 for i in range(16)],
                 'name'   : 'ModVT Width',
                 'comment': 'ModVT Width'
                 })
        self.reg_map.append({
                 'off'    : 71,
                 'value'  : [0.3*1000 for i in range(16)],
                 'name'   : 'Veto Width',
                 'comment': 'Veto Width'
                 })
        self.reg_map.append({
                 'off'    : 72,
                 'value'  : [0 for i in range(16)],
                 'name'   : 'Ext_ValTrig_In',
                 'comment': 'Ext_ValTrig_In'
                 })
        self.reg_map.append({
                 'off'    : 73,
                 'value'  : [0 for i in range(16)],
                 'name'   : 'Ext_VT_Sel',
                 'comment': 'Ext_VT_Sel'
                 })
        self.reg_map.append({
                 'off'    : 74,
                 'value'  : [0 for i in range(16)],
                 'name'   : 'IntVT_Sel',
                 'comment': 'IntVT_Sel'
                 })
        self.reg_map.append({
                 'off'    : 75,
                 'value'  : [0 for i in range(16)],
                 'name'   : 'ChanTrig_Sel',
                 'comment': 'ChanTrig_Sel'
                 })
        self.reg_map.append({
                 'off'    : 76,
                 'value'  : [0 for i in range(16)],
                 'name'   : 'Ext_FT_In',
                 'comment': 'Ext_FT_In'
                 })
        self.reg_map.append({
                 'off'    : 77,
                 'value'  : [0 for i in range(16)],
                 'name'   : 'Ext_FT_Sel',
                 'comment': 'Ext_FT_Sel'
                 })
        self.reg_map.append({
                 'off'    : 78,
                 'value'  : [0 for i in range(16)],
                 'name'   : 'IntFT_Sel',
                 'comment': 'IntFT_Sel'
                 })
        self.reg_map.append({
                 'off'    : 79,
                 'value'  : [0 for i in range(16)],
                 'name'   : 'Crate Master',
                 'comment': 'Crate Master'
                 })
        self.reg_map.append({
                 'off'    : 80,
                 'value'  : [0 for i in range(16)],
                 'name'   : 'MulCrate Master',
                 'comment': 'MulCrate Master'
                 })
        self.reg_map.append({
                 'off'    : 81,
                 'value'  : [0 for i in range(16)],
                 'name'   : 'Mod_VT',
                 'comment': 'Mod_VT'
                 })
        self.reg_map.append({
                 'off'    : 82,
                 'value'  : [0 for i in range(16)],
                 'name'   : 'Mod_FT',
                 'comment': 'Mod_FT'
                 })
        self.reg_map.append({
                 'off'    : 83,
                 'value'  : [0 for i in range(16)],
                 'name'   : 'TTL FP Ena',
                 'comment': 'TTL FP Ena'
                 })
        self.reg_map.append({
                 'off'    : 84,
                 'value'  : [0 for i in range(16)],
                 'name'   : 'Ch_Sel',
                 'comment': 'Ch_Sel'
                 })
        self.reg_map.append({
                 'off'    : 85,
                 'value'  : [0 for i in range(16)],
                 'name'   : 'Output_Sel',
                 'comment': 'Output_Sel'
                 })
        self.reg_map.append({
                 'off'    : 86,
                 'value'  : [0 for i in range(16)],
                 'name'   : 'Fo7_sel',
                 'comment': 'Fo7_sel'
                 })
        self.reg_map.append({
                 'off'    : 87,
                 'value'  : [1 for i in range(16)],
                 'name'   : 'AutoBLCUT',
                 'comment': 'AutoBLCUT'
                 })
        self.reg_map.append({
                 'off'    : 88,
                 'value'  : [3 for i in range(16)],
                 'name'   : 'revision',
                 'comment': 'revision'
                 })
        
        # below are other registers not exposed to the user.
        # off = 1002: read only, fifo status of all modules (sum of total
        #             number of words of all modules to be read out)
        # off = 1003: write only, a dumy write to this register will start
        #             NEW_RUN Syncronisely. This includes the following
        #             actions: 
        #                 1, Pixie16WriteSglModPar("SYNCH_WAIT")
        #                 2, Pixie16WriteSglModPar("IN_SYNCH")
        #                 3, Pixie16StartListModeRun(Number_Of_Modules)
        #                 4, Pixie16CheckRunStatus(Number_Of_Modules)
        #                 5, Pixie16ReadSglModPar("IN_SYNCH")
        # off = 1007: write only, a dumy write to this register stops list mode
        #             run. Before returning, run status will be checked for
        #             every module to make sure all modules are stopped.
        # off = 1008: read/write, dsp slot number
        # off = 1009: read/write, dsp crate number




    def _get_rounded_val(self, name, val):


        # the code below are translated from the pixie16 api library
        # (channel.cpp)
        new_val = val
        if name == 'XDT (us)':
            # convert to int
            off = 7
            value = float(val)
            current_xwait = self._get_reg_int_val(off)
            xwait = round(value * hw_limit_DSP_CLOCK_MHZ)
            if config_adc_msps == 100 or config_adc_msps == 500:
                multiple = 6
            else:
                multiple = 8
            if xwait < multiple:
                xwait = multiple
            if xwait > current_xwait:
                xwait = int(math.ceil(float(xwait) / multiple) * multiple)
            else:
                xwait = int(math.floor(float(xwait) / multiple) * multiple)
            self.reg_map[off]['xwait'][self.cur_ch_num] = xwait
            
            # convert back to float
            value = xwait
            new_val = float(value) / hw_limit_DSP_CLOCK_MHZ
        elif name == 'Offset(%)':
            if val < 1:
                val = 1
            if val > 99:
                val = 99
            new_val = val
        elif name == 'T_Risetime(us)':
            # convert to int
            value = float(val)
            off = 2
            ffr_mask = 1<<self.reg_map[off]['value'][self.cur_ch_num]
            off = 13
            fast_gap = self._get_reg_int_val(off)
            fast_length = int(round((value * config_fpga_clk_mhz) /
                ffr_mask))
            if (fast_length + fast_gap) > hw_limit_FASTFILTER_MAX_LEN:
                fast_length = hw_limit_FASTFILTER_MAX_LEN - fast_gap
            if fast_length < hw_limit_FASTLENGTH_MIN_LEN:
                fast_length = hw_limit_FASTLENGTH_MIN_LEN
                if (fast_length + fast_gap) > hw_limit_FASTFILTER_MAX_LEN:
                    fast_gap = hw_limit_FASTFILTER_MAX_LEN - hw_limit_FASTLENGTH_MIN_LEN

            # convert back to float
            fast_length = float(fast_length)
            new_val = (fast_length * ffr_mask) / config_fpga_clk_mhz

            # also need to change T_FlatTop value
            fast_gap = float(fast_gap)
            fast_gap = (fast_gap * ffr_mask) / config_fpga_clk_mhz
            fast_gap *= 1000
            off = 13
            self.reg_map[off]['value'][self.cur_ch_num] = fast_gap
            self.T_FlatTop_wid.delete(0, tk.END)
            self.T_FlatTop_wid.insert(0, '%.3f'%(fast_gap/1000.))
        elif name == 'T_FlatTop(us)':
            # convert to int
            value = float(val)
            off = 2
            ffr_mask = 1<<self.reg_map[off]['value'][self.cur_ch_num]
            off = 12
            fast_length = self._get_reg_int_val(off)
            fast_gap = int(round((value * config_fpga_clk_mhz) / ffr_mask))
            if (fast_length + fast_gap) > hw_limit_FASTFILTER_MAX_LEN:
                fast_gap = hw_limit_FASTFILTER_MAX_LEN - fast_length
            
            # convert back to float
            fast_gap = float(fast_gap)
            new_val = (fast_gap * ffr_mask) / config_fpga_clk_mhz
        elif name == 'T_Thresh':
            # convert to int
            value = float(val)
            off = 12
            fast_length = self._get_reg_int_val(off)
            print('fast_length: (should be 63): ' + str(fast_length))
            fast_thresh = int(value * fast_length * config_adc_clk_div)
            if fast_thresh >= hw_limit_FAST_THRESHOLD_MAX:
                dbl_fast_thresh = float(fast_thresh)
                value = (float(hw_limit_FAST_THRESHOLD_MAX) / (dbl_fast_thresh - 0.5)) * dbl_fast_thresh
                fast_thresh = int(value)

            # convert back to float
            fast_thresh = float(fast_thresh)
            new_val = fast_thresh / (fast_length * float(config_adc_clk_div))
            new_val = int(new_val)
        elif name == 'E_Risetime(us)':
            # convert to int
            off = 1
            sfr = self.reg_map[off]['value'][self.cur_ch_num] + 1
            sfr_mask = 1 << sfr
            value = float(val)
            slow_length = int(round((value * config_fpga_clk_mhz) / sfr_mask))
            off = 16
            slow_gap = self._get_reg_int_val(off)
            if (slow_length + slow_gap) > hw_limit_SLOWFILTER_MAX_LEN:
                slow_length = hw_limit_SLOWFILTER_MAX_LEN - slow_gap
            if slow_length < hw_limit_SLOWLENGTH_MIN_LEN:
                slow_length = hw_limit_SLOWLENGTH_MIN_LEN
                if (slow_length + slow_gap) > hw_limit_SLOWFILTER_MAX_LEN:
                    slow_gap = hw_limit_SLOWFILTER_MAX_LEN - hw_limit_SLOWLENGTH_MIN_LEN

            # convert back to float
            slow_length = float(slow_length)
            new_val = (slow_length * sfr_mask) / config_fpga_clk_mhz

            # also need to change E_FlatTop value
            slow_gap = float(slow_gap)
            slow_gap = (slow_gap * sfr_mask) / config_fpga_clk_mhz
            slow_gap *= 1000
            off = 16
            self.reg_map[off]['value'][self.cur_ch_num] = slow_gap
            self.E_FlatTop_wid.delete(0, tk.END)
            self.E_FlatTop_wid.insert(0, '%.3f'%(slow_gap/1000.))
            
            # update fifo (paf_len & trig_delay)
            off = 2
            ffr_mask = 1<<self.reg_map[off]['value'][self.cur_ch_num]
            off = 15
            paf_length = self.reg_map[off]['paf_len'][self.cur_ch_num]
            trigger_delay = self.reg_map[off]['trig_dl'][self.cur_ch_num]
            trace_delay = paf_length - (trigger_delay // ffr_mask)
            peak_sep = slow_length + slow_gap
            if sfr == 1:
                peak_sample = peak_sep - 3
            elif sfr == 5:
                peak_sample = peak_sep
            elif sfr == 4 or sfr == 6:
                peak_sample = peak_sep - 1
            else:
                peak_sample = peak_sep - 2
            self.reg_map[off]['peak_sample'][self.cur_ch_num] = peak_sample
            self.reg_map[off]['peak_sep'][self.cur_ch_num] = peak_sep
            self._update_fifo(trace_delay)
        elif name == 'E_FlatTop(us)':
            # convert to int
            off = 1
            sfr = self.reg_map[off]['value'][self.cur_ch_num] + 1
            sfr_mask = 1 << sfr
            value = float(val)
            slow_gap = int(round((value * config_fpga_clk_mhz) / sfr_mask))
            off = 15
            slow_length = self._get_reg_int_val(off)
            if (slow_length + slow_gap) > hw_limit_SLOWFILTER_MAX_LEN:
                slow_gap = hw_limit_SLOWFILTER_MAX_LEN - slow_length
            if slow_gap < hw_limit_SLOWGAP_MIN_LEN:
                slow_gap = hw_limit_SLOWGAP_MIN_LEN
                if (slow_length + slow_gap) > hw_limit_SLOWFILTER_MAX_LEN:
                    slow_length = hw_limit_SLOWFILTER_MAX_LEN - hw_limit_SLOWGAP_MIN_LEN

            # convert back to float
            slow_gap = float(slow_gap)
            new_val = (slow_gap * sfr_mask) / config_fpga_clk_mhz

            # also need to change E_Risetime value
            slow_length = float(slow_length)
            slow_length = (slow_length * sfr_mask) / config_fpga_clk_mhz
            slow_length *= 1000
            off = 15
            self.reg_map[off]['value'][self.cur_ch_num] = slow_length
            self.E_Risetime_wid.delete(0, tk.END)
            self.E_Risetime_wid.insert(0, '%.3f'%(slow_length/1000.))

            # update fifo (paf_len & trig_delay)
            off = 2
            ffr_mask = 1<<self.reg_map[off]['value'][self.cur_ch_num]
            off = 15
            paf_length = self.reg_map[off]['paf_len'][self.cur_ch_num]
            trigger_delay = self.reg_map[off]['trig_dl'][self.cur_ch_num]
            trace_delay = paf_length - (trigger_delay // ffr_mask)
            peak_sep = slow_length + slow_gap
            if sfr == 1:
                peak_sample = peak_sep - 3
            elif sfr == 5:
                peak_sample = peak_sep
            elif sfr == 4 or sfr == 6:
                peak_sample = peak_sep - 1
            else:
                peak_sample = peak_sep - 2
            self.reg_map[off]['peak_sample'][self.cur_ch_num] = peak_sample
            self.reg_map[off]['peak_sep'][self.cur_ch_num] = peak_sep
            self._update_fifo(trace_delay)
        elif name == 'Tau(us)':
            pass
        elif name == 'CFD_Dealy(us)':
            # convert to int
            value = float(val)
            cfddelay = int(round(value * config_fpga_clk_mhz))
            if cfddelay < hw_limit_CFDDELAY_MIN:
                cfddelay = hw_limit_CFDDELAY_MIN
            if cfddelay > hw_limit_CFDDELAY_MAX:
                cfddelay = hw_limit_CFDDELAY_MAX

            # convert back to float
            value = float(cfddelay)
            new_val = value / config_fpga_clk_mhz
        elif name == 'CFD_Threshol':
            cfdthresh = int(val)
            if cfdthresh < hw_limit_CFDTHRESH_MIN:
                cfdthresh = hw_limit_CFDTHRESH_MIN
            if cfdthresh > hw_limit_CFDTHRESH_MAX:
                cfdthresh = hw_limit_CFDTHRESH_MAX
            new_val = float(cfdthresh)
            new_val = int(new_val)
        elif name == 'Trace_PreTrig(us)':
            # convert to int 
            value = float(val)
            off = 2
            ffr_mask = 1<<self.reg_map[off]['value'][self.cur_ch_num]
            off = 28
            trace_length = self._get_reg_int_val(off)
            trace_delay = int(value * config_fpga_clk_mhz / ffr_mask)
            if trace_delay > trace_length:
                trace_delay = trace_length // 2
            if trace_delay > hw_limit_TRACEDELAY_MAX:
                trace_delay = hw_limit_TRACEDELAY_MAX
            self._update_fifo(trace_delay)

            # convert back to float
            off = 15
            paf_length = float(self.reg_map[off]['paf_len'][self.cur_ch_num])
            trigger_delay = float(self.reg_map[off]['trig_dl'][self.cur_ch_num])
            new_val = (paf_length - (trigger_delay / ffr_mask)) / config_fpga_clk_mhz * ffr_mask
        elif name == 'Trace_Length(us)':
            # convert to int
            off = 2
            ffr_mask = 1<<self.reg_map[off]['value'][self.cur_ch_num]
            fifo_length = config_fifo_length
            value = float(val)
            trace_length = int(value * config_adc_msps / ffr_mask)
            if config_adc_msps == 500:
                trace_length = (trace_length // 10) * 10
            elif config_adc_msps == 250 or config_adc_msps == 100:
                trace_length = (trace_length // 2) * 2
            if trace_length > fifo_length:
                trace_length = fifo_length

            # convert back to float
            trace_len = float(trace_length)
            new_val = trace_len / (float(config_adc_msps) * ffr_mask)
        elif name[:7] == 'QDC Len' and name[8:] == '(us)':
            # QDC Len0-7
            # convert to int
            value = float(val)
            multiplier = config_adc_msps
            if config_adc_msps == 500:
                multiplier = multiplier // 5
            qdclen = int(round(value * multiplier))
            if qdclen < hw_limit_QDCLEN_MIN:
                qdclen = hw_limit_QDCLEN_MIN
            if qdclen > hw_limit_QDCLEN_MAX:
                qdclen = hw_limit_QDCLEN_MAX

            # convert back to float
            divider = config_adc_msps
            if config_adc_msps == 500:
                divider = divider // 5
            value = qdclen
            new_val = float(value) / divider
        elif name == 'LCFT Width':
            # convert to int
            value = float(val)
            fast_trig_blen = int(round(value * config_fpga_clk_mhz))
            if config_adc_msps == 100 or config_adc_msps == 500:
                fast_trig_blen_min = hw_limit_FASTTRIGBACKLEN_MIN_100MHZFIPCL
            else:
                fast_trig_blen_min = hw_limit_FASTTRIGBACKLEN_MIN_125MHZFIPCLK
            if fast_trig_blen < fast_trig_blen_min:
                fast_trig_blen = fast_trig_blen_min
            elif fast_trig_blen > hw_limit_FASTTRIGBACKLEN_MAX:
                fast_trig_blen = hw_limit_FASTTRIGBACKLEN_MAX

            # convert back to float
            value = fast_trig_blen
            new_val = float(value) / config_fpga_clk_mhz
        elif name == 'LCFT Delay':
            # convert to int
            value = float(val)
            ftrigoutdelay = int(round(value * config_fpga_clk_mhz))
            off = 88
            ver = self.reg_map[off]['value'][self.cur_ch_num]
            if ver <= 2:
                ftrigoutdelay_max = hw_limit_EXTDELAYLEN_MAX_REVBCD
            else:
                ftrigoutdelay_max = hw_limit_EXTDELAYLEN_MAX_REVF
            if ftrigoutdelay > ftrigoutdelay_max:
                ftrigoutdelay = ftrigoutdelay_max

            # convert back to float
            value = ftrigoutdelay
            new_val = float(value) / config_fpga_clk_mhz
        elif name == 'FIFO Delay':
            # convert to int
            value = float(val)
            externdelaylen = int(round(value * config_fpga_clk_mhz))
            off = 88
            ver = self.reg_map[off]['value'][self.cur_ch_num]
            if ver <= 2:
                externdelaylen_max = hw_limit_EXTDELAYLEN_MAX_REVBCD
            else:
                externdelaylen_max = hw_limit_EXTDELAYLEN_MAX_REVF
            if externdelaylen < hw_limit_EXTDELAYLEN_MIN:
                externdelaylen + hw_limit_EXTDELAYLEN_MIN
            if externdelaylen > externdelaylen_max:
                externdelaylen = externdelaylen_max

            # convert back to float
            value = externdelaylen
            new_val = float(value) / config_fpga_clk_mhz
        elif name == 'ChVT Width':
            # convert to int
            value = float(val)
            chantrigstretch = int(round(value * config_fpga_clk_mhz))
            if chantrigstretch < hw_limit_CHANTRIGSTRETCH_MIN:
                chantrigstretch = hw_limit_CHANTRIGSTRETCH_MIN
            if chantrigstretch > hw_limit_CHANTRIGSTRETCH_MAX:
                chantrigstretch = hw_limit_CHANTRIGSTRETCH_MAX

            # convert back to float
            value = chantrigstretch
            new_val = float(value) / config_fpga_clk_mhz
        elif name == 'ModVT Width':
            # convert to int
            value = float(val)
            exttrigstretch = int(round(value * config_fpga_clk_mhz))
            if exttrigstretch < hw_limit_EXTTRIGSTRETCH_MIN:
                exttrigstretch = hw_limit_EXTTRIGSTRETCH_MIN
            if exttrigstretch > hw_limit_EXTTRIGSTRETCH_MAX:
                exttrigstretch = hw_limit_EXTTRIGSTRETCH_MAX

            # convert back to float
            value = exttrigstretch
            new_val = float(value) / config_fpga_clk_mhz
        elif name == 'Veto Width':
            # convert to int
            value = float(val)
            vetostretch = int(round(value * config_fpga_clk_mhz))
            if vetostretch < hw_limit_VETOSTRETCH_MIN:
                vetostretch = hw_limit_VETOSTRETCH_MIN
            if vetostretch > hw_limit_VETOSTRETCH_MAX:
                vetostretch = hw_limit_VETOSTRETCH_MAX

            # convert back to float
            value = vetostretch
            new_val = float(value) / config_fpga_clk_mhz

        return new_val

    def _update_fifo(self, trace_delay):
            off = 1
            sfr = self.reg_map[off]['value'][self.cur_ch_num] + 1
            sfr_mask = 1 << sfr
            off = 2
            ffr_mask = 1<<self.reg_map[off]['value'][self.cur_ch_num]
            fifo_length = config_fifo_length
            off = 15
            peak_sep = self.reg_map[off]['peak_sep'][self.cur_ch_num] 
            trigger_delay = (peak_sep - 1) * sfr_mask
            paf_length = (trigger_delay // ffr_mask) + trace_delay
            if paf_length > fifo_length:
                paf_length = fifo_length - 1
                trigger_delay = (paf_length - trace_delay) * ffr_mask
            self.reg_map[off]['trig_dl'][self.cur_ch_num] = trigger_delay
            self.reg_map[off]['paf_len'][self.cur_ch_num] = paf_length


    def _get_reg_int_val(self, off):
        value = float(self.reg_map[off]['value'][self.cur_ch_num])/1000
        if off == 7:
            # XDT
            return self.reg_map[off]['xwait'][self.cur_ch_num]
        if off == 13:
            # T_FlatTop
            off = 2
            ffr_mask = 1<<self.reg_map[off]['value'][self.cur_ch_num]
            fast_gap = int(round((value * config_fpga_clk_mhz) / ffr_mask))
            return fast_gap
        if off == 12:
            # T_Risetime
            off = 2
            ffr_mask = 1<<self.reg_map[off]['value'][self.cur_ch_num]
            fast_length = int(round((value * config_fpga_clk_mhz) / ffr_mask))
            return fast_length
        if off == 16:
            # E_FlatTop
            off = 1
            sfr = self.reg_map[off]['value'][self.cur_ch_num] + 1
            sfr_mask = 1 << sfr
            slow_gap = int(round((value * config_fpga_clk_mhz) / sfr_mask))
            return slow_gap
        if off == 15:
            # E_Risetime
            off = 1
            sfr = self.reg_map[off]['value'][self.cur_ch_num] + 1
            sfr_mask = 1 << sfr
            slow_length = int(round((value * config_fpga_clk_mhz) / sfr_mask))
            return slow_length
        if off == 28:
            # Trace_Length
            off = 2
            ffr_mask = 1<<self.reg_map[off]['value'][self.cur_ch_num]
            trace_length = int(value * config_adc_msps / ffr_mask)
            return trace_length







                    




########## TEST ##########
#mod = vme_mod('test')
#print("OK")
