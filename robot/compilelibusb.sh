wget http://sourceforge.net/projects/libusb/files/libusb-1.0/libusb-1.0.9/libusb-1.0.9.tar.bz2
tar xjf libusb-1.0.9.tar.bz2 
cd libusb-1.0.9
~/klaatu-manifests/scripts/install_sysroot.sh ~/klaatu-rpm 4.0.4 maguro
export PATH=`pwd`/aroot/toolchain/bin:$PATH
CC=arm-bionic-eabi-gcc ./configure --host=arm-linux
make
