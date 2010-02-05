TEMPLATE = app

OBJECTS_DIR = build
MOC_DIR = build

QT += network
HEADERS += src/servent.h \
    src/connection.h \ 
    src/controlconnection.h \
    src/proxyconnection.h \
    src/proxyconnectionlistener.h \
    src/proxylistener.h \
    src/testdriver.h \
    zeroconf/bonjourrecord.h \
    zeroconf/bonjourservicebrowser.h \
    zeroconf/bonjourserviceregister.h \
    zeroconf/bonjourserviceresolver.h
SOURCES += src/servent.cpp \
    src/connection.cpp \
    src/main.cpp \
    src/proxyconnection.cpp \
    src/proxyconnectionlistener.cpp \
    src/proxylistener.cpp \
    src/controlconnection.cpp \
    zeroconf/bonjourservicebrowser.cpp \
    zeroconf/bonjourserviceregister.cpp \
    zeroconf/bonjourserviceresolver.cpp


LIBS += -L/usr/local/lib \
    -lqjson

!mac:x11:LIBS+=-ldns_sd

win32 {
    LIBS+=-ldnssd
    # Add your path to bonjour here.
    #LIBPATH=C:/...
    #INCLUDEPATH += c:/...
}

unix {
    LIBS += -ldns_sd
}

