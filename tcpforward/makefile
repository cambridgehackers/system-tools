
CC = gcc
OBJS = tcpforward.o
CFLAGS = -g -Wall

all: tcpforward

clean:
	rm -f *.o tcpforward core

tcpforward:		${OBJS}
	${CC} -o tcpforward ${OBJS} ${LIBS}
