
CFLAGS=-Wall -fPIC -I/usr/local/include -I/usr/include/libusb-1.0 -g
# -ldl

%.so: %.o
	gcc -fPIC -shared $(CFLAGS) $< /lib/x86_64-linux-gnu/libdl-2.15.so -o $@
#/usr/lib32/libdl.a -o $@

tester.o: jcaftdi.c

tester: tester.o ftdiwrap.o Makefile
	gcc -g -o tester tester.o ftdiwrap.o -ldl -lpthread -lusb-1.0
