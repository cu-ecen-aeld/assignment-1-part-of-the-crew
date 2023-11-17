
TARGET = aesdsocket

#CROSS_COMPILE=aarch64-none-linux-gnu-
CROSS_COMPILE=
CC=$(CROSS_COMPILE)gcc
CFLAGS=-Wall

default: $(TARGET)


all: $(TARGET)
	

$(TARGET): $(TARGET).c
	$(CC) $(CFLAGS) -o $@ $^



clean:
	rm -rf *.o $(TARGET)
	
.PHONY: all default clean
