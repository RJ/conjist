TEMPLATE = lib
QT -= all
CONFIG += staticlib \
    release
TARGET = miniupnp
OBJECTS_DIR = ../build
MOC_DIR = ../build
SOURCES += igd_desc_parse.c \
    minisoap.c \
    minissdpc.c \
    miniupnpc.c \
    miniwget.c \
    minixml.c \
    minixmlvalid.c \
    upnpc.c \
    upnpcommands.c \
    upnperrors.c \
    upnpreplyparse.c
HEADERS += bsdqueue.h \
    codelength.h \
    declspec.h \
    igd_desc_parse.h \
    minisoap.h \
    minissdpc.h \
    miniupnpc.h \
    miniupnpcstrings.h \
    miniwget.h \
    minixml.h \
    upnpcommands.h \
    upnperrors.h \
    upnpreplyparse.h
