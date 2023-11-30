
TARGET = aesdsocket

ifdef CROSS_COMPILE
ifneq ($(CROSS_COMPILE), aarch64-none-linux-gnu-)
$(error '$(CROSS_COMPILE)' is not supported!)
endif
endif


CC=$(CROSS_COMPILE)gcc
CFLAGS=-Wall -pthread -ggdb
#-L/usr/lib/ -I/usr/include/ -L/usr/local/lib

default: $(TARGET)


all: $(TARGET)
	

$(TARGET): $(TARGET).c
	$(CC) $(CFLAGS) -o $@ $^



clean:
	rm -rf *.o $(TARGET)
	
.PHONY: all default clean
