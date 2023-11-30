TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
  aesdsocket.c

#HEADERS += \
#  queue.h

LIBS += -lpthread
