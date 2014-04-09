
usbreset: usbreset.c Makefile
	gcc -o usbreset usbreset.c

install: usbreset
	cp usbreset /usr/local/bin
	chown root:root /usr/local/bin/usbreset
	# setuid so that user doesn't need to sudo for reseting webcam
	chmod 7555 /usr/local/bin/usbreset
