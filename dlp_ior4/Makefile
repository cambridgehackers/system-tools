
CFLAGS= $(VARIANT) -Wall -fPIC -I/usr/include/libusb-1.0 -I/opt/local/include/libusb-1.0 -g

OBJS = dlpior4.o

dlpior4: $(OBJS) Makefile
	gcc -g -o dlpior4 $(OBJS) -lpthread -lusb-1.0 -lz

mac: $(OBJS) Makefile
	gcc -g -o dlpior4 $(OBJS) -lpthread /opt/local/lib/libusb-1.0.dylib -lz

install: dlpior4
	cp dlpior4 /usr/local/bin

clean:
	rm -f $(OBJS) dlpior4
