ifdef CROSS_COMPILE
ifneq ($(CROSS_COMPILE), aarch64-none-linux-gnu-)
$(error '$(CROSS_COMPILE)' is not supported!)
endif
endif

TARGET ?= aesdsocket
CC ?= $(CROSS_COMPILE)gcc
CFLAGS ?= -g -Wall -Werror
LDFLAGS ?= -pthread



default: $(TARGET)

all: $(TARGET)
	
$(TARGET): $(TARGET).c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

clean:
	rm -rf *.o $(TARGET)
	
.PHONY: all default clean
