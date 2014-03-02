#!/usr/bin/python
# Copyright (c) 2013 Quanta Research Cambridge, Inc.
#
# Permission is hereby granted, free of charge, to any person
# obtaining a copy of this software and associated documentation
# files (the "Software"), to deal in the Software without
# restriction, including without limitation the rights to use, copy,
# modify, merge, publish, distribute, sublicense, and/or sell copies
# of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
# BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
# ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
# CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

from __future__ import print_function
import optparse, os, sys

class Proto:
    def __init__(self):
        self.rettype = ''
        self.name = ''
        self.plist = []

def splittype(name):
    if name[-1] == ')':
        ind = name.find('(')
        if ind > 0:
            temp = name[ind+1:]
            ind = temp.find(')')
            temp = temp[:ind].strip()
            if temp[0] == '*':
                return name, temp[1:].strip()
    ind = len(name) - 1
    while name[ind].isalnum() or name[ind] == '_':
        ind = ind - 1
        if ind < 0:
            return name + ' ', None
    if ind == len(name) - 1:
        return name + ' ', None
    return name, name[ind+1:].strip()

def parse_header(filename):
    readdata = open(filename).read().replace('\r', '\n') + '\n'
    while True:
        ind = readdata.find('/*')
        if ind < 0:
            break
        endind = readdata.find('*/')
        if endind < 0:
            endind = len(readdata)-3
        readdata = readdata[:ind] + ' ' + readdata[endind+2:]
    lines = readdata.split(';')
    for item in lines:
        item = item.replace('\n', ' ').strip()
        if item.startswith('#') or item.startswith('//') or item.startswith('typedef') or item.find('{') >= 0:
            continue
        ind = item.find('(')
        if ind < 0:
            continue
            print('missing "(":', item)
            sys.exit(1)
        name = item[:ind].strip()
        remain = item[ind+1:]
        ind = remain.rfind(')')
        if ind < 0:
            continue
            print('missing ")":', item)
            sys.exit(1)
        remain = remain[:ind]
        p = Proto()
        p.rettype, p.name = splittype(name)
        if p.name == None:
            continue
        #print('JJ', item, p.rettype, p.name)
        params = []
        pstr = ''
        level = 0
        for ch in remain:
            if ch == ',' and level == 0:
                params.append(pstr)
                pstr = ''
            else:
                if ch == '(':
                   level = level + 1
                elif ch == ')':
                   level = level - 1
                pstr = pstr + ch
        if pstr != '':
            params.append(pstr)
        for pitem in params:
            pitem = pitem.strip()
            fulls, name = splittype(pitem)
            if name is not None:
                p.plist.append([fulls, name])
        functionproto[p.name] = p

def getargs(a, include_datatype):
    p = functionproto[a]
    retstr = ''
    sepitem = ''
    for item in p.plist:
        retstr = retstr + sepitem + item[include_datatype]
        sepitem = ', '
    return '(' + retstr + ')'
        
def override(a, b):
    print('#include <stdio.h>')
    print('#include <dlfcn.h>')
    print(functionproto[a].rettype + a + getargs(a, 0) + ' {')
    print('    static void *dlopen_ptr = NULL;')
    print('    static ' + functionproto[a].rettype + '(*dlsym_ptr)' + getargs(a, 0) + ' = NULL;')
    print('    if (!dlopen_ptr) {')
    print('        if (!(dlopen_ptr = dlopen("' + options.library + '", RTLD_LAZY))) {')
    print('            fprintf(stderr, "Failed to dlopen ' + options.library + ', error %s\\n", dlerror());')
    print('            return -1;')
    print('        }')
    print('        dlsym_ptr = dlsym(dlopen_ptr, "' + a + '");')
    print('    }')
    print('    /////////////////////////////////////////\n' + b + '\n    /////////////////////////////////////////')
    print('    return dlsym_ptr' + getargs(a, 1) + ';')
    print('}\n')

if __name__=='__main__':
    parser = optparse.OptionParser("usage: %prog [options] arg")
    parser.add_option("-l", "--library", dest="library")
    parser.add_option("-o", "--output", dest="filename")
    parser.add_option("-p", "--proto", action="append", dest="proto")
    (options, args) = parser.parse_args()
    functionproto = {}
    for argstr in options.proto:
        parse_header(argstr)
    override('sqlite3_unlock_notify', '    {\n        printf("socket called\\n");\n    }')
