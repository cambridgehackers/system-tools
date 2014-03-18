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

# formatmap replacements are ordered, so use a list
formatmap = []
formatmap.append(['WRITE 0x4b,', '    TMSW,'])
formatmap.append(['WRITE 0x6f,', '    TMSRW,'])
formatmap.append(['WRITE 0x1b,', '    DATAWBIT,'])
formatmap.append(['WRITE 0x2e,', '    DATARBIT,'])
formatmap.append(['WRITE 0x3f,', '    DATARWBIT,'])
formatmap.append(['WRITE 0x19,', '    DATAW,'])
formatmap.append(['WRITE 0x2c,', '    DATAR,'])
formatmap.append(['WRITE 0x3d,', '    DATARW,'])
formatmap.append(['WRITE    ', '        '])
formatmap.append(['WRITE 0x87,', '    SEND_IMMEDIATE,'])

formatmap.append(['DATAW, 0x03, 0x00', 'DATAW(0, 4)'])
formatmap.append(['DATAW, 0x00, 0x00', 'DATAW(0, 1)'])
formatmap.append(['DATAR, 0x03, 0x00', 'DATAR(4)'])
formatmap.append(['DATARW, 0x03, 0x00', 'DATAW(DREAD, 4)'])
formatmap.append(['DATARW, 0x3e, 0x00', 'DATAW(DREAD, 0x3f)'])
formatmap.append(['DATARW, 0x7e, 0x00', 'DATAW(DREAD, 0x7f)'])
formatmap.append(['TMSW, 0x00, 0x01', 'SHIFT_TO_EXIT1(0, 0)'])
formatmap.append(['TMSW, 0x02, 0x03', 'SHIFT_TO_UPDATE_TO_IDLE(0, 0)'])
formatmap.append(['TMSW, 0x00, 0x81', 'SHIFT_TO_EXIT1(0, 0x80)'])
formatmap.append(['TMSW, 0x02, 0x83', 'SHIFT_TO_UPDATE_TO_IDLE(0, 0x80)'])
formatmap.append(['TMSRW, 0x00, 0x01', 'SHIFT_TO_EXIT1(DREAD, 0)'])
formatmap.append(['TMSRW, 0x02, 0x03', 'SHIFT_TO_UPDATE_TO_IDLE(DREAD, 0)'])
formatmap.append(['TMSRW, 0x00, 0x81', 'SHIFT_TO_EXIT1(DREAD, 0x80)'])
formatmap.append(['TMSRW, 0x02, 0x83', 'SHIFT_TO_UPDATE_TO_IDLE(DREAD, 0x80)'])
formatmap.append(['TMSW, 0x03, 0x03', 'IDLE_TO_SHIFT_IR'])
formatmap.append(['TMSW, 0x02, 0x01', 'IDLE_TO_SHIFT_DR'])
formatmap.append(['TMSW, 0x01, 0x01', 'EXIT1_TO_IDLE'])
formatmap.append(['TMSW, 0x02, 0x07', 'IDLE_TO_RESET'])
formatmap.append(['TMSW, 0x00, 0x00', 'RESET_TO_IDLE'])
formatmap.append(['TMSW, 0x00, 0x7f', 'IN_RESET_STATE'])
formatmap.append(['TMSW, 0x04, 0x1f', 'FORCE_RETURN_TO_RESET'])
formatmap.append(['TMSW, 0x03, 0x02', 'RESET_TO_SHIFT_DR'])
formatmap.append(['TMSW, 0x00, 0x01', 'RESET_TO_RESET'])
formatmap.append(['TMSW, 0x01, 0x01', 'PAUSE_TO_SHIFT'])
formatmap.append(['TMSW, 0x01, 0x01', 'SHIFT_TO_PAUSE'])
formatmap.append(['TMSW, 0x04, 0x7f', 'TMS_RESET_WEIRD'])

multiline = []
multiline.append(['TMSW, 0x06, 0x00,\n    TMSW, 0x06, 0x00,\n    TMSW, 0x06, 0x00', 'TMS_WAIT'])
multiline.append(['),\n        INT32(', '), INT32('])
multiline.append(['IDLE_TO_SHIFT_IR,\n    DATA', 'IDLE_TO_SHIFT_IR, DATA'])
multiline.append(['IDLE_TO_SHIFT_IR, DATAWBIT, 0x05, 0xff, DATAWBIT, 0x02', 'TEMPLOADIR(0)'])
multiline.append(['IDLE_TO_SHIFT_DR,\n    DATA', 'IDLE_TO_SHIFT_DR, DATA'])
multiline.append(['IDLE_TO_SHIFT_DR, DATAWBIT, 0x00, 0x00', 'TEMPLOADDR(0)'])

multiline.append(['TEMPLOADDR(0), DATAW(DREAD, 4), INT32(0x18060016),\n    DATARWBIT, 0x01, 0x00,\n    SHIFT_TO_UPDATE_TO_IDLE(DREAD, 0)', 'LOADDR(DREAD, 0x18060016, 0x00, 0)'])
multiline.append(['TEMPLOADDR(0), DATAW(DREAD, 4), INT32(0x18860016),\n    DATARWBIT, 0x01, 0x00,\n    SHIFT_TO_UPDATE_TO_IDLE(DREAD, 0)', 'LOADDR(DREAD, 0x18860016, 0x00, 0)'])
multiline.append(['TEMPLOADDR(0), DATAW(DREAD, 4), INT32(0x07),\n    DATARWBIT, 0x01, 0x00,\n    SHIFT_TO_UPDATE_TO_IDLE(DREAD, 0)', 'LOADDR(DREAD, 0x07, 0x00, 0)'])
multiline.append(['TEMPLOADDR(0), DATAW(DREAD, 4), INT32(0x03),\n    DATARWBIT, 0x01, 0x00,\n    SHIFT_TO_UPDATE_TO_IDLE(DREAD, 0)', 'LOADDR(DREAD, 0x03, 0x00, 0)'])
multiline.append(['TEMPLOADDR(0), DATAW(DREAD, 4), INT32(0x08000004),\n    DATARWBIT, 0x01, 0x00,\n    SHIFT_TO_UPDATE_TO_IDLE(DREAD, 0)', 'LOADDR(DREAD, 0x08000004, 0x00, 0)'])
multiline.append(['TEMPLOADDR(0), DATAW(DREAD, 4), INT32(0x10),\n    DATARWBIT, 0x01, 0x00,\n    SHIFT_TO_UPDATE_TO_IDLE(DREAD, 0)', 'LOADDR(DREAD, 0x10, 0x00, 0)'])
multiline.append(['TEMPLOADDR(0), DATAW(DREAD, 4), INT32(0xc0000802),\n    DATARWBIT, 0x01, 0x07,\n    SHIFT_TO_UPDATE_TO_IDLE(DREAD, 0x80)', 'LOADDR(DREAD, 0xc0000802, 0x07, 0x80)'])
multiline.append(['TEMPLOADDR(0), DATAW(DREAD, 4), INT32(0x00490142),\n    DATARWBIT, 0x01, 0x04,\n    SHIFT_TO_UPDATE_TO_IDLE(DREAD, 0x80)', 'LOADDR(DREAD, 0x00490142, 0x04, 0x80)'])
multiline.append(['TEMPLOADDR(0), DATAW(DREAD, 4), INT32(0x00490002),\n    DATARWBIT, 0x01, 0x04,\n    SHIFT_TO_UPDATE_TO_IDLE(DREAD, 0x80)', 'LOADDR(DREAD, 0x00490002, 0x04, 0x80)'])
multiline.append(['TEMPLOADDR(0), DATAW(DREAD, 4), INT32(0x004918a2),\n    DATARWBIT, 0x01, 0x04,\n    SHIFT_TO_UPDATE_TO_IDLE(DREAD, 0x80)', 'LOADDR(DREAD, 0x004918a2, 0x04, 0x80)'])
multiline.append(['TEMPLOADDR(0), DATAW(DREAD, 4), INT32(0x00480002),\n    DATARWBIT, 0x01, 0x04,\n    SHIFT_TO_UPDATE_TO_IDLE(DREAD, 0x80)', 'LOADDR(DREAD, 0x00480002, 0x04, 0x80)'])
multiline.append(['TEMPLOADDR(0), DATAW(DREAD, 4), INT32(0x004818a2),\n    DATARWBIT, 0x01, 0x04,\n    SHIFT_TO_UPDATE_TO_IDLE(DREAD, 0x80)', 'LOADDR(DREAD, 0x004818a2, 0x04, 0x80)'])
multiline.append(['TEMPLOADDR(0), DATAW(DREAD, 4), INT32(0x00480442),\n    DATARWBIT, 0x01, 0x04,\n    SHIFT_TO_UPDATE_TO_IDLE(DREAD, 0x80)', 'LOADDR(DREAD, 0x00480442, 0x04, 0x80)'])
multiline.append(['TEMPLOADDR(0), DATAW(0, 4), INT32(0x08),\n    DATAWBIT, 0x01, 0x00,\n    SHIFT_TO_UPDATE_TO_IDLE(0, 0)', 'LOADDR(0, 0x08, 0x00, 0)'])
multiline.append(['TEMPLOADDR(0), DATAW(0, 4), INT32(0x03),\n    DATAWBIT, 0x01, 0x00,\n    SHIFT_TO_UPDATE_TO_IDLE(0, 0)', 'LOADDR(0, 0x03, 0x00, 0)'])

multiline.append(['TEMPLOADDR(0), DATAW(0, 4), INT32(0x08000004),\n    DATAWBIT, 0x01, 0x00,\n    SHIFT_TO_UPDATE_TO_IDLE(0, 0)', 'LOADDR(0, 0x08000004, 0x00, 0)'])
multiline.append(['TEMPLOADDR(0), DATAW(0, 4), INT32(0x04),\n    DATAWBIT, 0x01, 0x00,\n    SHIFT_TO_UPDATE_TO_IDLE(0, 0)', 'LOADDR(0, 0x04, 0x00, 0)'])
multiline.append(['TEMPLOADDR(0), DATAW(DREAD, 4), INT32(0x10),\n    DATARWBIT, 0x01, 0x04,\n    SHIFT_TO_UPDATE_TO_IDLE(DREAD, 0x80)', 'LOADDR(DREAD, 0x10, 0x04, 0x80)'])
multiline.append(['TEMPLOADDR(0), DATAW(DREAD, 4), INT32(0xc0000902),\n    DATARWBIT, 0x01, 0x07,\n    SHIFT_TO_UPDATE_TO_IDLE(DREAD, 0x80)', 'LOADDR(DREAD, 0xc0000902, 0x07, 0x80)'])
multiline.append(['TEMPLOADDR(0), DATAW(0, 4), INT32(0x08000004),\n    DATAWBIT, 0x01, 0x00,\n    SHIFT_TO_UPDATE_TO_IDLE(0, 0)', 'LOADDR(0, 0x00490442, 0x00, 0)'])
multiline.append(['TEMPLOADDR(0), DATAW(DREAD, 4), INT32(0x00490442),\n    DATARWBIT, 0x01, 0x04,\n    SHIFT_TO_UPDATE_TO_IDLE(DREAD, 0x80)', 'LOADDR(DREAD, 0x00490442, 0x04, 0x80)'])
multiline.append(['TEMPLOADDR(0), DATAW(0, 4), INT32(0x00490442),\n    DATAWBIT, 0x01, 0x04,\n    SHIFT_TO_UPDATE_TO_IDLE(0, 0x80)', 'LOADDR(0, 0x00490442, 0x04, 0x80)'])
multiline.append(['TEMPLOADDR(0), DATAW(0, 4), INT32(0x00480422),\n    DATAWBIT, 0x01, 0x04,\n    SHIFT_TO_UPDATE_TO_IDLE(0, 0x80', 'LOADDR(0, 0x00480422, 0x04, 0x80)'])
multiline.append(['TEMPLOADDR(0), DATAW(0, 4), INT32(0x004918a2),\n    DATAWBIT, 0x01, 0x04,\n    SHIFT_TO_UPDATE_TO_IDLE(0, 0x80)', 'LOADDR(0, 0x004918a2, 0x04, 0x80)'])
multiline.append(['TEMPLOADDR(0), DATAW(0, 4), INT32(0x8000019a),\n    DATAWBIT, 0x01, 0x02,\n    SHIFT_TO_UPDATE_TO_IDLE(0, 0)', 'LOADDR(0, 0x8000019a, 0x02, 0)'])

multiline.append(['TEMPLOADDR(0), DATAW(DREAD, 4), INT32(0x01),\n    DATARWBIT, 0x01, 0x00,\n    SHIFT_TO_UPDATE_TO_IDLE(DREAD, 0)', 'LOADDR(DREAD, 0x01, 0x00, 0)'])
multiline.append(['TEMPLOADDR(0), DATAW(0, 4), INT32(0xc0038402),\n    DATAWBIT, 0x01, 0x07,\n    SHIFT_TO_UPDATE_TO_IDLE(0, 0x80)', 'LOADDR(0, 0xc0038402, 0x07, 0x80)'])
multiline.append(['TEMPLOADDR(0), DATAW(DREAD, 4), INT32(0x00480142),\n    DATARWBIT, 0x01, 0x04,\n    SHIFT_TO_UPDATE_TO_IDLE(DREAD, 0x80)', 'LOADDR(DREAD, 0x00480142, 0x04, 0x80)'])
multiline.append(['TEMPLOADDR(0), DATAW(0, 4), INT32(0x00480442),\n    DATAWBIT, 0x01, 0x04,\n    SHIFT_TO_UPDATE_TO_IDLE(0, 0x80)', 'LOADDR(0, 0x00480442, 0x04, 0x80)'])
multiline.append(['TEMPLOADDR(0), DATAW(0, 4), INT32(0x00490422),\n    DATAWBIT, 0x01, 0x04,\n    SHIFT_TO_UPDATE_TO_IDLE(0, 0x80)', 'LOADDR(0, 0x00490422, 0x04, 0x80)'])
multiline.append(['TEMPLOADDR(0), DATAW(0, 4), INT32(0x004818a2),\n    DATAWBIT, 0x01, 0x04,\n    SHIFT_TO_UPDATE_TO_IDLE(0, 0x80)', 'LOADDR(0, 0x004818a2, 0x04, 0x80)'])

multiline.append(['TEMPLOADIR(0), 0xff,\n    TMSW, 0x01, 0x83', 'LOADIR(0xff)'])
multiline.append(['TEMPLOADIR(0), 0xf8,\n    TMSW, 0x01, 0x83', 'LOADIR(0xf8)'])
multiline.append(['TEMPLOADIR(0), 0xfa,\n    TMSW, 0x01, 0x83', 'LOADIR(0xfa)'])
multiline.append(['TEMPLOADIR(0), 0xfb,\n    TMSW, 0x01, 0x83', 'LOADIR(0xfb)'])

int32template = '        0x02, 0x08, 0x00, 0xc0,'
#                          23    89    45    01

if __name__=='__main__':
    parser = optparse.OptionParser("usage: %prog [options] arg")
    parser.add_option("-l", "--library", dest="library")
    (options, args) = parser.parse_args()
    counter = 1000
    for afile in args:
        outline = ''
        lines = open(afile).readlines()
        lastline = ''
        for temp in lines:
            temp = temp.rstrip()
            for item in formatmap:
                ind = temp.find(item[0])
                if ind >= 0:
                    temp = temp[:ind] + item[1] + temp[ind+len(item[0]):]
            if temp.startswith('        ') and len(temp) == len(int32template):
                temp = temp.strip()
                temp = '        INT32(0x' + temp[20:22] + temp[14:16] + temp[8:10] + temp[2:4] + '),'
                temp = temp.replace('INT32(0x000000', 'INT32(0x')
            if temp.startswith('    DATA') and lastline.startswith('    DATA'):
                outline = outline + temp[3:]
            elif temp.startswith('READ') and lastline.startswith('READ'):
                outline = outline + temp[4:]
            else:
                outline = outline + '\n' + temp
            lastline = temp
        for item in multiline:
            outline = outline.replace(item[0], item[1])
        print(outline)

