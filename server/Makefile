ifdef CROSS_COMPILE
ifneq ($(CROSS_COMPILE), aarch64-none-linux-gnu-)
$(error '$(CROSS_COMPILE)' is not supported!)
endif
endif

TARGET ?= aesdsocket
CC ?= $(CROSS_COMPILE)gcc
CFLAGS ?= -g -Wall -Werror
LDFLAGS ?= -pthread -lpthread -lrt



default: $(TARGET)

all: $(TARGET)


# Source files
SRCS = $(TARGET).c

# Object files
OBJS = $(SRCS:.c=.o)

.c.o:
	$(CC) -c $(CFLAGS) $^
	
$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)




clean:
	rm -rf *.o $(TARGET)
	
distclean: clean
	
.PHONY: all default clean distclean




