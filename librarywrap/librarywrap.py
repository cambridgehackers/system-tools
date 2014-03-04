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

def splittype(name, generatedname):
    if name[-1] == ')':
        ind = name[:-1].find(')')
        if ind > 0:
            rind = ind - 1
            while name[rind].isalnum() or name[rind] == '_' or name[rind] == ' ':
                rind = rind - 1
            rind = rind + 1
            tname = name[rind:ind].strip()
            if tname == '':
                tname = generatedname
                name = name[:rind] + generatedname + name[rind:]
            return name, tname
    ind = len(name) - 1
    while name[ind].isalnum() or name[ind] == '_':
        ind = ind - 1
        if ind < 0:
            if name.strip() == 'void':
                return name, ''
            return name + ' ' + generatedname, generatedname
    if ind == len(name) - 1:
        if name.startswith('...'):
            return name, 'VARARGS'
        return name + ' ' + generatedname, generatedname
    return name, name[ind+1:].strip()

def parse_header(filename):
    readdata = open(filename).read().replace('\r', '\n').replace('\t', ' ') + '\n'
    while True:
        ind = readdata.find('/*')
        if ind < 0:
            break
        endind = readdata.find('*/')
        if endind < 0:
            endind = len(readdata)-3
        readdata = readdata[:ind] + ' ' + readdata[endind+2:]
    lines = []
    templine = ''
    lastlf = -1
    for ch in readdata:
        if ch == ';':
            lines.append(templine)
            templine = ''
        else:
            if ch == '\n':
                if lastlf >= 0 and templine[lastlf+1:lastlf+2] == '#':
                    lines.append(templine[:lastlf])
                    templine = ''
                lastlf = len(templine)
            templine = templine + ch
    lines.append(templine)
    for item in lines:
        item = item.replace('\n', ' ').strip()
        if item.startswith('#') or item.startswith('//') or item.startswith('typedef')\
          or item.find('{') >= 0 or item.find('DEPRECATED') > 0:
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
        p.rettype, p.name = splittype(name, '')
        if p.name == '':
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
        argindex = 0
        for pitem in params:
            pitem = pitem.strip()
            fulls, name = splittype(pitem, '__generatedarg' + str(argindex))
            argindex = argindex + 1
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
        
def generate_function(fname, wrap_pre, wrap_post):
    rettype = functionproto[fname].rettype[:-len(fname)].strip()
    retassign = 'retval = '
    if rettype == 'void':
        retassign = ''
    conditional_flag = getargs(fname, 1).endswith('VARARGS)')
    if conditional_flag:
        print('#if 0 /* needs manual editing to support "..." */', file=outfile)
    print(functionproto[fname].rettype + getargs(fname, 0) + '\n{', file=outfile)
    print('    static void *dlopen_ptr = NULL;', file=outfile)
    print('    static ' + functionproto[fname].rettype[:-len(fname)] + '(*real_func)' + getargs(fname, 0) + ' = NULL;', file=outfile)
    if retassign != '':
        print('    ' + rettype + ' retval;', file=outfile)
    print('    if (!dlopen_ptr) {', file=outfile)
    print('        openlogfile();', file=outfile)
    print('        if (!(dlopen_ptr = dlopen("' + options.library + '", RTLD_LAZY))) {', file=outfile)
    print('            fprintf(stderr, "Failed to dlopen ' + options.library + ', error %s\\n", dlerror());', file=outfile)
    if rettype[-1] == '*':
        print('            return NULL;', file=outfile)
    elif retassign == '' or rettype.startswith('struct'):
        print('            return;', file=outfile)
    else:
        print('            return -1;', file=outfile)
    print('        }', file=outfile)
    print('        real_func = dlsym(dlopen_ptr, "' + fname + '");', file=outfile)
    print('    }', file=outfile)
    print('    /////////////////////////////////////////\n    {', file=outfile)
    for temp in wrap_pre:
        print(temp, file=outfile)
    print('    }\n    /////////////////////////////////////////', file=outfile)
    print('    ' + retassign + 'real_func' + getargs(fname, 1) + ';', file=outfile)
    for temp in wrap_post:
        print(temp, file=outfile)
    if retassign != '':
        print('    return retval;', file=outfile)
    print('}', file=outfile)
    if conditional_flag:
        print('#endif', file=outfile)
    print('', file=outfile)

if __name__=='__main__':
    parser = optparse.OptionParser("usage: %prog [options] arg")
    parser.add_option("-l", "--library", dest="library")
    parser.add_option("-t", "--trace", action="store_true", dest="trace", default=False)
    parser.add_option("-o", "--output", dest="filename")
    parser.add_option("-p", "--proto", action="append", dest="proto")
    (options, args) = parser.parse_args()
    functionproto = {}
    if options.filename:
        outfile = open(options.filename, 'w')
    else:
        outfile = sys.stdout
    for argstr in options.proto:
        parse_header(argstr)
    pre_list = {}
    post_list = {}
    for afile in args:
        lines = open(afile).readlines()
        fname = ''
        lines_list = []
        in_prelines = True
        for temp in lines:
            temp = temp.rstrip()
            if temp.strip().startswith('POST:'):
                in_prelines = False
                if lines_list != []:
                    pre_list[fname] = lines_list
                lines_list = []
            elif temp.strip().startswith('WRAP:'):
                if lines_list != []:
                    if in_prelines:
                        pre_list[fname] = lines_list
                    else:
                        post_list[fname] = lines_list
                fname = temp.strip()[5:].strip()
                lines_list = []
                in_prelines = True
            else:
                lines_list.append(temp)
        if lines_list != []:
            if in_prelines:
                pre_list[fname] = lines_list
            else:
                post_list[fname] = lines_list
    print('#include <stdio.h>', file=outfile)
    print('#include <dlfcn.h>\n', file=outfile)
    print('static FILE *logfile;', file=outfile)
    print('static void openlogfile(void)', file=outfile)
    print('{', file=outfile)
    print('if (!logfile)', file=outfile)
    print('    logfile = fopen("/tmp/xx.logfile", "w");', file=outfile)
    print('}', file=outfile)
    for key in functionproto.iterkeys():
        preitem = pre_list.get(key)
        postitem = post_list.get(key)
        if preitem is None:
            preitem = []
            if options.trace and postitem is None:
                preitem = [ '    fprintf(logfile, "[%s] called\\n", __FUNCTION__);' ]
        if postitem is None:
            postitem = []
        if preitem != [] or postitem != []:
            generate_function(key, preitem, postitem)

