CFLAGS=-Wall -g -levent

all: http

servlet: servlet.o
    
clean:
    rm -f *.o    