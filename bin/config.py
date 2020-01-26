#!/bin/python
# -*- coding: UTF-8 -*-
__metaclass__ = type


# This is a helper python script to make the creation/manipulation of the
# configuration files easier and less errors. Of cause you can also manually
# create/modify your configuration file (an xml file) using your prefferred
# text editor.
# This usage is
#           ./config.py <file name>
# or
#           ./config.py
# The former one loads an existing configuration file while the latter one
# creates a new configuration file.
# 
# There are many choices in the python world for GUI programming. I just chose
# to use tkinter. To run the script successfully, you may need to install tcl
# and tk pachages in your system. In my system (arch linux), I did it like:
#          pacman -S tcl
#          pacman -S tk
# Also in order to avoid various potential problems, it's better to run the
# script using python 3
#  
# By B.Gao(gaobsh@impcas.ac.cn)


import tkinter as tk
import sys


# gloval vars to be used in the script:
# top window
top_win = ''
# list of supported modules (MADC32, V1190, V830...)
supported_mod = []
# list of added modules
added_mod = []




def err_exit(msg):
    sys.stderr.write(msg)
    sys.exit(-1)


# load the configuration from a given file name and set the corresponding
# variables
def load_conf(f_name):
    pass


# show the main window and enter the main loop
def show_win():
    top_win = tk.Tk()
    supported_mod = get_supported()
    supported_txt = 

if len(sys.argv) > 2:
    err_exit('usage: ./config.py [file name]\n')

if len(sys.argv) == 2:
    # load existing configuration file
    load_conf(sys.argv[1])

# Now it's time to show the main window and enter the event loop:
show_win()

# If we reach here, the user must have terminated the event loop, let's try to
# save the file then.
write_file()


