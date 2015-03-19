#!/usr/bin/env python

# Copyright (c) 2013 Quanta Research Cambridge, Inc.

# Permission is hereby granted, free of charge, to any person
# obtaining a copy of this software and associated documentation
# files (the "Software"), to deal in the Software without
# restriction, including without limitation the rights to use, copy,
# modify, merge, publish, distribute, sublicense, and/or sell copies
# of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
# BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
# ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
# CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

import sys
import os
import socket
import struct
import select
import time
import threading
import argparse
import netifaces

def ip2int(addr):                                                               
    return struct.unpack("!I", socket.inet_aton(addr))[0]                       

def int2ip(addr):                                                               
    return socket.inet_ntoa(struct.pack("!I", addr))

def open_adb_socket(dest_addr):
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.setblocking(0)
    sock.connect_ex((dest_addr,554))
    return sock

# non-Darwin version
def do_work_poll(start, end):
    print "scanning "+int2ip(start)+" to "+int2ip(end)
    connected = []
    total = end-start

    READ_ONLY = select.POLLIN | select.POLLPRI | select.POLLHUP | select.POLLERR
    READ_WRITE = READ_ONLY | select.POLLOUT
    poller = select.poll()

    while (start <= end):
        fd_map = {}
        while (start <= end):
            try:
                s = open_adb_socket(int2ip(start))
            except:
                break
            else:
                fd_map[s.fileno()] = (start,s)
                start = start+1
                poller.register(s, READ_WRITE)
        time.sleep(0.2)
        events = poller.poll(0.1)
        for fd,flag in events:
            (addr,sock) = fd_map[fd]
            if sock.getsockopt(socket.SOL_SOCKET, socket.SO_ERROR) == 0:
                print 'ADDCON', fd, int2ip(addr)
                connected.append(addr)
        for fd,t in fd_map.iteritems():
            poller.unregister(t[1])
            t[1].close()
        sys.stdout.write("\r%d/%d" % (total-(end-start),total))
        sys.stdout.flush()
    print

# Darwin version
def do_work_kqueue(start, end):
    print "scanning "+int2ip(start)+" to "+int2ip(end)
    connected = []
    total = end-start

    while (start <= end):
        kq = select.kqueue()
        fd_map = {}
        kevents = []
        myend = start + 20
        if myend > end:
           myend = end
        while (start <= myend):
            try:
                s = open_adb_socket(int2ip(start))
            except:
                break
            else:
                fd_map[s.fileno()] = (start,s)
                start = start+1
                kevents.append(select.kevent(s,filter=select.KQ_FILTER_WRITE))
            break
        #print 'before kq.control', kq, len(kevents)
#, kevents
        kq.control(kevents,1,0)
        #print 'after kq.control'
        time.sleep(0.2)
        for k in kq.control([],len(kevents),0.1):
            w = fd_map[k.ident][1]
            addr = fd_map[w.fileno()][0]
            if w.getsockopt(socket.SOL_SOCKET, socket.SO_ERROR) == 0:
                print 'ADDCON2', k.ident, w.fileno(), int2ip(addr), fd_map[w.fileno()]
                connected.append(addr)
        for fd,t in fd_map.iteritems():
            t[1].close()
        sys.stdout.write("\r%d/%d" % (total-(end-start),total))
        sys.stdout.flush()
    print


argparser = argparse.ArgumentParser("Discover Zedboards on a network")
argparser.add_argument('-n', '--network', help='xxx.xxx.xxx.xxx/N')

def do_work(start,end):
    if sys.platform == 'darwin':
        do_work_kqueue(start,end)
    else:
        do_work_poll(start,end)

def detect_network():
    global zedboards
    zedboards = []
    for ifc in netifaces.interfaces():
        ifaddrs = netifaces.ifaddresses(ifc)
        if netifaces.AF_INET in ifaddrs.keys():
            af_inet = ifaddrs[netifaces.AF_INET]
            for i in af_inet: 
                if i.get('addr') == '127.0.0.1':
                    print 'skipping localhost'
                else:
                    addr = ip2int(i.get('addr'))
                    netmask = ip2int(i.get('netmask'))
                    start = addr & netmask
                    end = start + (netmask ^ 0xffffffff) 
                    start = start+1
                    end = end-1
                    print (int2ip(start), int2ip(end)) 
                    do_work(start, end) 

if __name__ ==  '__main__':
    zedboards = []
    options = argparser.parse_args()
    if options.network == None:
        detect_network()
    else:
        nw = options.network.split("/")
        start = ip2int(nw[0])
        end = start+(1<<int(nw[1]))-2
        do_work(start+1,end)
