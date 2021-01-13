#!/bin/python
# -*- coding: UTF-8 -*-
__metaclass__ = type

# This is the base class for the servers. It provides the basic socket
# operations such as (non-blocking)recv, send, etc.
# We use the module 'socket'. See 'https://realpython.com/python-sockets/' and
# 'https://docs.python.org/3/library/socket.html' for documentations.
#
# By B.Gao Feb. 2019

import socket

class svr:
    def __init__(self):
        self.sock = None

    def set_sock(self, sock):
        # the sock is the connection socket, (not listening socket)
        self.sock = sock

    def try_recv(self):
        # try to receive a pending message (if any), if no message, return None
        if not self.sock:
            return None
        msg_all = b''
        while True:
            try:
                msg = self.sock.recv(128, socket.MSG_WAITALL)
            except BlockingIOError:
                return None
            except ConnectionResetError:
                msg = None
            if not msg:
                # broken connection:
                self.sock = None
                # message type
                msg_all += (5).to_bytes(length=4, byteorder='little')
                # message level
                msg_all += (2).to_bytes(length=4, byteorder='little')
                # message texts
                msg_all += bytes('client disconnected!', 'utf-8')
                return msg_all
            end_mark = msg[-1]
            head = int.from_bytes(msg[:4], 'little')
            if head != 1000:
                msg_all += msg[:4]
            msg_all += msg[4:124]
            if end_mark == 0:
                return msg_all

    def send_all(self, msg):
        if not self.sock:
            return
        self.sock.sendall(msg)




