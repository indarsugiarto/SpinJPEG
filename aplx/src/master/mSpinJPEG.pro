TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    mSpinJPEG.c \
    eHandlers.c \ 
    parse.c \
    utils.c \
    tree_vld.c

INCLUDEPATH += /opt/spinnaker_tools_3.1.0/include \
    ../

DISTFILES += \
    Makefile \
    README \
    compile

HEADERS += \
    mSpinJPEG.h
