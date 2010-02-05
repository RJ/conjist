TEMPLATE = app

OBJECTS_DIR = ../build
MOC_DIR = ../build

QT += network

HEADERS +=  servent.h \
            connection.h \ 
            controlconnection.h \
            proxyconnection.h \
            proxyconnectionlistener.h \
            proxylistener.h \
            testdriver.h \
            zeroconf/bonjourrecord.h \
            zeroconf/bonjourservicebrowser.h \
            zeroconf/bonjourserviceregister.h \
            zeroconf/bonjourserviceresolver.h
SOURCES +=  servent.cpp \
            connection.cpp \
            main.cpp \
            proxyconnection.cpp \
            proxyconnectionlistener.cpp \
            proxylistener.cpp \
            controlconnection.cpp \
            zeroconf/bonjourservicebrowser.cpp \
            zeroconf/bonjourserviceregister.cpp \
            zeroconf/bonjourserviceresolver.cpp

LIBPATH     += ../qxmpp/
INCLUDEPATH += ../qxmpp/

LIBS += -L/usr/local/lib \
        -lqjson \
        -lQXmppClient

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

