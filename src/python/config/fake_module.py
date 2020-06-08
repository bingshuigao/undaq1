#!/bin/python
# -*- coding: UTF-8 -*-
__metaclass__ = type

# This is a fake vme module class. This class is used only when there is no
# real trigger-type module. 
###########3
# By B.Gao Jun. 2020

import tkinter as tk
from tkinter import ttk
from vme_mod import vme_mod


class fake_module(vme_mod):
    def __init__(self, name, mod=None):
        # we need to create the reg_map variable
        self.reg_map = []

        # This method must be called at the **end**.
        self._base_init(name, mod)
    
