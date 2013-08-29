
CC = gcc
OBJS = tcpweb.o
CFLAGS = -g -Wall

all: tcpweb

clean:
	rm -f *.o tcpweb core

tcpweb:		${OBJS}
	${CC} -o tcpweb ${OBJS} ${LIBS}
