# -------------------------------------------------
# Project created by QtCreator 2010-02-07T18:35:48
# -------------------------------------------------
# QT += network

TEMPLATE = lib
QT -= gui
OBJECTS_DIR = ../build
MOC_DIR = ../build
TARGET = qdaapd
CONFIG += staticlib
CONFIG -= app_bundle
SOURCES += mongoose.c \
    httpd.cpp \
    daaplib/src/registry.cpp \
    daaplib/src/taginput.cpp \
    daaplib/src/tagoutput.cpp \ # daaplib
    collection.cpp
HEADERS += httpd.h \
    mongoose.h \
    daap/basic.h \
    daap/registry.h \
    daap/taginput.h \
    daap/tagoutput.h \ # daaplib
    collection.h
INCLUDEPATH += daaplib/include
