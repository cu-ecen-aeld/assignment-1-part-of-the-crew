#CC=gcc
CFLAGS=-Wall

#CROSS_COMPILE=aarch64-none-linux-gnu-
CROSS_COMPILE=
CC=$(CROSS_COMPILE)gcc

all:
	$(CC) $(CFLAGS) main.c -o writer

clean:
	rm -rf *.o writer
	
