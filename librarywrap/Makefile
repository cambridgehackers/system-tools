
CFLAGS=-fPIC -I/usr/local/include -g
# -ldl

%.so: %.o
	gcc -fPIC -shared $(CFLAGS) $< /lib/x86_64-linux-gnu/libdl-2.15.so -o $@
#/usr/lib32/libdl.a -o $@
