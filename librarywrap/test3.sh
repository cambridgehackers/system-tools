#
set -x
set -e
FTLIB=/home/jca/git/system-tools/librarywrap/libftdi1-1.1/build/src/libftdi1.so.2.1.0
#FTLIB=/scratch/Xilinx/Vivado/2013.2/lib/lnx64.o/libxftdi.orig.so 
./librarywrap.py -o ftdiwrap.h -t -l $FTLIB -p ftdi_reference.h ftdi.wrap
#/usr/include/ftdi.h
rm -f ftdiwrap.so ftdiwrap.o
make ftdiwrap.so

# when running executable with this library:
#    LD_PRELOAD=./ftdiwrap.so executablename
