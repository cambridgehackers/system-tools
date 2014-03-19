
CFLAGS=-Wall -fPIC -I/usr/local/include -I/usr/include/libusb-1.0 -g
FTLIB=/home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0
#FTLIB=/scratch/Xilinx/Vivado/2013.2/lib/lnx64.o/libxftdi.orig.so 
# -ldl

%.so: %.o
	gcc -fPIC -shared $(CFLAGS) $< /lib/x86_64-linux-gnu/libdl-2.15.so -o $@
#/usr/lib32/libdl.a -o $@

tester: tester.o ftdiwrap.o Makefile
	#gcc -g -o tester tester.o ftdiwrap.o -ldl -lpthread -lusb-1.0
	gcc -g -o tester tester.o -lpthread -lusb-1.0

trace:
	set -x
	set -e
	./librarywrap.py -o ftdiwrap.h -t -l $FTLIB -p ftdi_reference.h ftdi.wrap
	#/usr/include/ftdi.h
	rm -f ftdiwrap.so ftdiwrap.o
	make ftdiwrap.so
	# when running executable with this library:
	#    LD_PRELOAD=./ftdiwrap.so executablename
