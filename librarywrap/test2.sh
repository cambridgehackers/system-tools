#
set -x
set -e
./librarywrap.py -o ftdiwrap.h -t -l /scratch/Xilinx/Vivado/2013.2/lib/lnx64.o/libxftdi.orig.so -p /usr/include/ftdi.h
rm -f ftdiwrap.so
make ftdiwrap.so

# when running executable with this library:
#    LD_PRELOAD=./ftdiwrap.so executablename
