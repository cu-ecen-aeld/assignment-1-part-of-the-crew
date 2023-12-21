QT -= gui
#
TEMPLATE = lib
DEFINES += CHAR_1_LIBRARY
#
CONFIG += c11
#
## The following define makes your compiler emit warnings if you use
## any Qt feature that has been marked deprecated (the exact warnings
## depend on your compiler). Please consult the documentation of the
## deprecated API in order to know how to port your code away from it.
#DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += __KERNEL__ DEBUG MODULE
# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0



#LINUX_HEADERS_PATH += /usr/src/linux-headers-$$system(uname -r)
#INCLUDEPATH += $$system(find -L $$LINUX_HEADERS_PATH/arch/x86/include -type d)
#INCLUDEPATH += $$LINUX_HEADERS_PATH/arch/x86/include
#INCLUDEPATH += $$LINUX_HEADERS_PATH/arch/x86/include/generated
#INCLUDEPATH += $$LINUX_HEADERS_PATH/arch/x86/include/asm-generic
#INCLUDEPATH += $$system(find -L $$LINUX_HEADERS_PATH/include -type d)
#
#CONFIG -= qt
#INCLUDEPATH += $$system(find -L include/ -type d)
#SOURCES += $$system(find -L . -type f -iname \'*.c\')
#HEADERS += $$system(find -L . -type f -iname \'*.h\')
#INCLUDEPATH += include/ arch/x86/include

SOURCES += \
        main.c \
        aesd-circular-buffer.c

#KERNEL_RELEASE = $$system(uname -r)
#
#INCLUDEPATH = \
#    /usr/lib/modules/$${KERNEL_RELEASE}/build/include \
#    /usr/lib/modules/$${KERNEL_RELEASE}/build/include/linux \
#    /usr/lib/modules/$${KERNEL_RELEASE}/build/arch/x86/include \
#    /usr/lib/modules/$${KERNEL_RELEASE}/build/arch/x86/include/generated

HEADERS += \
    aesdchar.h \
    aesd-circular-buffer.h


LINUX_HEADERS_PATH = /usr/src/linux-headers-$$system(uname -r)
INCLUDEPATH += $$system(find -L $$LINUX_HEADERS_PATH/include -type d)




# Default rules for deployment.
unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target
