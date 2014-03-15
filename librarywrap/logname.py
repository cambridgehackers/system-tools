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

formatmap = {}
formatmap['WRITE 0x4b,'] = '    TMSW,'
formatmap['WRITE 0x6f,'] = '    TMSRW,'
formatmap['WRITE 0x1b,'] = '    DATAWBIT,'
formatmap['WRITE 0x2e,'] = '    DATARBIT,'
formatmap['WRITE 0x3f,'] = '    DATARWBIT,'
formatmap['WRITE 0x19,'] = '    DATAW,'
formatmap['WRITE 0x2c,'] = '    DATAR,'
formatmap['WRITE 0x3d,'] = '    DATARW,'
formatmap['WRITE    '] = '        '
formatmap['WRITE 0x87,'] = '    SEND_IMMEDIATE,'
formatmap['DATAW, 0x03, 0x00,'] = 'DATAW(4),'
formatmap['DATAW, 0x00, 0x00,'] = 'DATAW(1),'
formatmap['DATAR, 0x03, 0x00,'] = 'DATAR(4),'
formatmap['DATARW, 0x03, 0x00,'] = 'DATARW(4),'
formatmap['DATARW, 0x3e, 0x00,'] = 'DATARW(0x3f),'
formatmap['DATARW, 0x7e, 0x00,'] = 'DATARW(0x7f),'

formatmap['TMSW, 0x00, 0x01'] = 'SHIFT_TO_EXIT1(0, 0)'
formatmap['TMSW, 0x02, 0x03'] = 'SHIFT_TO_UPDATE_TO_IDLE(0, 0)'
formatmap['TMSW, 0x00, 0x81'] = 'SHIFT_TO_EXIT1(0, 0x80)'
formatmap['TMSW, 0x02, 0x83'] = 'SHIFT_TO_UPDATE_TO_IDLE(0, 0x80)'
formatmap['TMSRW, 0x00, 0x01'] = 'SHIFT_TO_EXIT1(DREAD, 0)'
formatmap['TMSRW, 0x02, 0x03'] = 'SHIFT_TO_UPDATE_TO_IDLE(DREAD, 0)'
formatmap['TMSRW, 0x00, 0x81'] = 'SHIFT_TO_EXIT1(DREAD, 0x80)'
formatmap['TMSRW, 0x02, 0x83'] = 'SHIFT_TO_UPDATE_TO_IDLE(DREAD, 0x80)'

formatmap['TMSW, 0x03, 0x03'] = 'IDLE_TO_SHIFT_IR'
formatmap['TMSW, 0x02, 0x01'] = 'IDLE_TO_SHIFT_DR'
formatmap['TMSW, 0x01, 0x01'] = 'EXIT1_TO_IDLE'
formatmap['TMSW, 0x02, 0x07'] = 'IDLE_TO_RESET'
formatmap['TMSW, 0x00, 0x00'] = 'RESET_TO_IDLE'
formatmap['TMSW, 0x00, 0x7f'] = 'IN_RESET_STATE'
formatmap['TMSW, 0x04, 0x1f'] = 'FORCE_RETURN_TO_RESET'
formatmap['TMSW, 0x03, 0x02'] = 'RESET_TO_SHIFT_DR'
formatmap['TMSW, 0x00, 0x01'] = 'RESET_TO_RESET'
formatmap['TMSW, 0x01, 0x01'] = 'PAUSE_TO_SHIFT'
formatmap['TMSW, 0x01, 0x01'] = 'SHIFT_TO_PAUSE'
formatmap['TMSW, 0x04, 0x7f'] = 'TMS_RESET_WEIRD'

if __name__=='__main__':
    parser = optparse.OptionParser("usage: %prog [options] arg")
    parser.add_option("-l", "--library", dest="library")
    (options, args) = parser.parse_args()
    counter = 1000
    for afile in args:
        lines = open(afile).readlines()
        for temp in lines:
            #temp = temp + '  '
            temp = temp.rstrip()
            #print('JJ "' + temp + '"')
            for item in formatmap.iterkeys():
                ind = temp.find(item)
                #if ind >= 0:
                    #print('item "'+item+'"', ind, len(item), 'Z' + temp[ind+len(item):] + 'Z')
                if ind == 0:
                    temp = formatmap[item] + temp[ind+len(item):]
                elif ind >= 0:
                    temp = temp[:ind-1] + formatmap[item] + temp[ind+len(item):]
            print(temp)
