#!/bin/python
# -*- coding: UTF-8 -*-
__metaclass__ = type


 # This is the base class of the vme modules. Each vme module object maintains
 # all configuration data for the corresponding module. The vme module object
 # also manages a top-level window (by calling the show_win method to shown the
 # window) allowing the interaction with user to modify/view its
 # configurations.
 # The variable 'reg_map' is the centrol data structure of the class. It is a
 # list, each element is a dict which has the following structure. (note, the
 # elements in this dict doesn't have to be a physical register of the module,
 # it can be any setting parameter with uniq offset):
 #  {'off':offset, ---> offset of the register
 #   'value':value, ---> value of the registter, a special value 'default' is
 #                       allowed.
 #   'name':name, --->name of the register (will be used as the text of the
 #                    label in the settings area if this register has one
 #                    corresponding widget in the settings area)
 #   'nbit':bits, ---> number of effective bits
 #   'has_set_wid':bool, ---> True if this register has a corresponding
 #                            widget in the setting area.
 #   'set_wid_type':type, ---> The type of the widget in the settings area.
 #                             For example, it can be 'entry' or 'comb'
 #                             which means tk.Entry and ttk.Combobox,
 #                             respectively.
 #   'set_wid':window, ---> The widget in the settings area, it is 
 #                          set when the widget is created. It should be left
 #                          uninitialized when creating the entry
 #   'set_wid_values':values ---> The possible values of the widget in the
 #                                settings area if it is a combobox. It is a
 #                                list in which the ordering of the elements
 #                                matters. Because the value of the register
 #                                is not the selected value of the combobox,
 #                                but the index of the selected value in
 #                                this list.
 #   'reg_wid':window, ---> The widget in the registers area, it is 
 #                          set when the widget is created. It should be left
 #                          uninitialized when creating the entry.
 #   'get_set_wid_val':fun ---> A pointer to a function which determines the
 #                              value of the widget in the settings area
 #                              according to its register value. The function
 #                              takes one parameter which is the element of
 #                              this dict. If it is None, a default function
 #                              will be used.
 #   'get_reg_wid_val':fun ---> A pointer to a function which determines the
 #                              value of the widget in the registers area
 #                              according to its register value. The function
 #                              takes one parameter which is the element of
 #                              this dict. If it is None, a default function
 #                              will be used.
 #   'comment':'comment' ---> Comments (explaination) about this register. 

 #  }
 #
 # To derive a subclass from it, one only needs to create the reg_map dict (in
 # the __init__ function)
 # By B.Gao (Aug. 2018)

import tkinter as tk
from tkinter import messagebox
from tkinter import ttk
from adv_conf_mod import adv_conf_mod
import copy


# define the base class of vme modules (e.g. madc32, v1190...)
class vme_mod:
    def __init__(self, name, mod=None):
        pass
    def _base_init(self, name, mod=None):
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
            # set non-default values of registers
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
            self.am_reg = 0
            self.type = 'T' 
            self.period = 1000
            self.max_crate = 1 
            self.name = name
        # check if all the keys of each of the elements (dict) in the reg_map
        # list are initialized, initialize to None if not.
        list_of_key = ['off', 'value', 'name', 'nbit', 'has_set_wid',
                'set_wid_type', 'set_wid', 'set_wid_values', 'reg_wid',
                'get_set_wid_val', 'get_reg_wid_val', 'comment']
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
        
    # a list of set and get methods.
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
            if reg['value'] != 'default':
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
            value = '0x%x' % reg['value']
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
    def set_conf(self, conf):
        for name, val, com in conf:
            try:
                if name.startswith('glo'):
                    name = name[4:]
                    if name == 'crate_n':
                        self.crate = int(val)
                    elif name == 'slot_n':
                        self.slot = int(val)
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
                    val1 = int(val, 16)
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


   
   # This method should be called when the user want to edit/view the
    # configurations of the module
    def show_win(self):
        self._init_layouts()
        self._init_com()
        self._init_set()
        self._init_reg()

    # place the window in propriate posistion
    def _place_win_set(self, win):
        win.grid(row=self.tot_set//self.col_set, 
                column=(self.tot_set%self.col_set))
        self.tot_set += 1
    def _place_win_reg(self, win):
        win.grid(row=self.tot_reg//self.col_reg, 
                column=(self.tot_reg%self.col_reg))
        self.tot_reg += 1

    def _init_layouts(self):
        self.win = tk.Toplevel()
        self.frm = tk.Frame(self.win, width=800, height=600)
        self.frm.pack()
        #common settings 
        self.frm_com = tk.Frame(self.frm, bg='yellow') 
        self.frm_com.place(x=0, y=0, width=800, height=100)
        # module specific setting and registers, because we need to implement a
        # scrollbar, the codes are much more complecated. The implementation of
        # scrollbar is reffered to
        # https://stackoverflow.com/questions/3085696/adding-a-scrollbar-to-a-group-of-widgets-in-tkinter
        self.canv_set = tk.Canvas(self.frm, bg='gray')
        self.frm_set = tk.Frame(self.canv_set, bg='gray') 
        self.vsb_set = tk.Scrollbar(self.frm, orient='vertical',
                command=self.canv_set.yview)
        self.canv_set.configure(yscrollcommand=self.vsb_set.set)
        self.vsb_set.place(x=780, y=110, width=20, height=200)
        self.canv_set.place(x=0, y=110, width=800, height=200)
        self.canv_set.create_window((0,0), window=self.frm_set, anchor='nw')
        self.frm_set.bind('<Configure>', self._frm_set_conf)
        self.canv_reg = tk.Canvas(self.frm, bg='green')
        self.frm_reg = tk.Frame(self.canv_reg, bg='green') 
        self.vsb_reg = tk.Scrollbar(self.frm, orient='vertical',
                command=self.canv_reg.yview)
        self.canv_reg.configure(yscrollcommand=self.vsb_reg.set)
        self.vsb_reg.place(x=780, y=320, width=20, height=200)
        self.canv_reg.place(x=0, y=320, width=800, height=200)
        self.canv_reg.create_window((0,0), window=self.frm_reg, anchor='nw')
        self.frm_reg.bind('<Configure>', self._frm_reg_conf)
        
        # total number of windows in the module specific settings and registers
        # area.  The layout is col columns,  row number is increased as needed.
        self.tot_set = 0
        self.col_set = 6
        self.tot_reg = 0
        self.col_reg = 8

        # The 'OK' and 'Cancel' buttons
        self.stat = 'cancel'
        self.butt_ok = tk.Button(self.win, text='OK', command=self._OK)
        self.butt_cl = tk.Button(self.win, text='Cancel', command=self._cancel)
        self.butt_ok.place(x=0, y=530, width=60, height=25)
        self.butt_cl.place(x=80, y=530, width=60, height=25)

    
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
            if reg['set_wid_type'] == 'entry':
                val,flag = self._str2int(reg['set_wid'].get())
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
            val,flag = self._str2int(tmp)
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


########## TEST ##########
#mod = vme_mod('test')
#print("OK")
