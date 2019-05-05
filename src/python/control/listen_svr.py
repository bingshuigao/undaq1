# This is the listening socket (not to be mixed with the connection socket).
# This socket binds to a port, listens and accepts connections (in non-blocking
# mode)
# 
# By B.Gao Feb. 2019

import socket

class listen_svr:
    def __init__(self, port):
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.setblocking(False)
        self.sock.bind(('127.0.0.1', port))
        self.sock.listen(10)

    def try_accept(self):
        # try to accept a connection request (in non-blocking mode), if
        # accepted a connection, return the connection and the name of the
        # client
        try:
            conn,addr = self.sock.accept()
        except BlockingIOError:
            return (None, None)
        # Now we established the connection, let's check the name of the client
        msg_type = (1).to_bytes(length=4, byteorder='little')
        msg_body = bytes([0 for i in range(124)])
        msg = msg_type + msg_body
        conn.sendall(msg)
        while True:
            msg = conn.recv(128, socket.MSG_WAITALL)
            msg_type = int.from_bytes(msg[:4], 'little')
            if msg_type == 1: 
                break
        msg_body = msg[4:]
        if msg_body.startswith(b'front'):
            name = 'frontend'
        elif msg_body.startswith(b'event'):
            name = 'event builder'
        elif msg_body.startswith(b'ana'):
            name = 'analyzer'
        elif msg_body.startswith(b'log'):
            name = 'logger'
        else:
            name = None
        conn.setblocking(False)
        return conn, name
