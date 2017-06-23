TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    wSpinJPEG.c \
    eHandlers.c 

INCLUDEPATH += /opt/spinnaker_tools_3.1.0/include \
    ../


DISTFILES += \
    Makefile \
    README \
    compile 

HEADERS += \
    wSpinJPEG.h
