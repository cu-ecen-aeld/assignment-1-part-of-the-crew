TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
  aesdsocket.c

HEADERS +=  \
    ./../aesd-char-driver/aesd_ioctl.h

LIBS += -lpthread
