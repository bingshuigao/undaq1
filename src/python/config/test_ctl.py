#!/bin/python
# -*- coding: UTF-8 -*-
__metaclass__ = type

# This is the test_ctl. This object manages the configuration data of a
# test_ctl module (this is for testing purposes)
###########3
# By B.Gao Jun. 2020

import tkinter as tk
from tkinter import ttk
from vme_mod import vme_mod


class test_ctl(vme_mod):
    def __init__(self, name, mod = None):
        # we need to create the reg_map variable
        self.reg_map = []

        super()._base_init(name, mod)
   
