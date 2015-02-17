#!/usr/bin/python
## Copyright (c) 2014 Quanta Research Cambridge, Inc.

## Permission is hereby granted, free of charge, to any person
## obtaining a copy of this software and associated documentation
## files (the "Software"), to deal in the Software without
## restriction, including without limitation the rights to use, copy,
## modify, merge, publish, distribute, sublicense, and/or sell copies
## of the Software, and to permit persons to whom the Software is
## furnished to do so, subject to the following conditions:

## The above copyright notice and this permission notice shall be
## included in all copies or substantial portions of the Software.

## THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
## EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
## MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
## NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
## BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
## ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
## CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
## SOFTWARE.

#more info at: http://serverfault.com/questions/89114/finding-the-public-ip-address-in-a-shell-script
#
# ident.me
# o-o.myaddr.l.google.com
# whoami.akamai.net
# checkmyip.com
# icanhazip.com
# myip.opendns.com
# https://shtuff.it/myip/short
# stunserver.org

#url = "http://ifconfig.me/ip"
#url = "http://www.whatismyip.com"
url = "http://ip.alt.io"
import urllib2
ipaddr = urllib2.urlopen(url).read()
print 'MYPUBLICIP is ', ipaddr
