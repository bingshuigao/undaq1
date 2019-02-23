#!/bin/python
# -*- coding: UTF-8 -*-
__metaclass__ = type

from xml_parser import xml_parser
from svr import svr

a = xml_parser('aaa')
b = svr()
print(type(b))
