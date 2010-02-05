TEMPLATE = app
OBJECTS_DIR = ../build
MOC_DIR = ../build
QT += network \
    xml
TARGET = ../conjist
HEADERS += servent.h \
    connection.h \
    controlconnection.h \
    proxyconnection.h \
    proxyconnectionlistener.h \
    proxylistener.h \
    testdriver.h \
    zeroconf/bonjourrecord.h \
    zeroconf/bonjourservicebrowser.h \
    zeroconf/bonjourserviceregister.h \
    zeroconf/bonjourserviceresolver.h \
    conjist.h \
    jabberclient.h \
    portfwd.h \
    getopt_helper.h \
    cjuuid.h
SOURCES += servent.cpp \
    connection.cpp \
    main.cpp \
    proxyconnection.cpp \
    proxyconnectionlistener.cpp \
    proxylistener.cpp \
    controlconnection.cpp \
    zeroconf/bonjourservicebrowser.cpp \
    zeroconf/bonjourserviceregister.cpp \
    zeroconf/bonjourserviceresolver.cpp \
    conjist.cpp \
    jabberclient.cpp \
    portfwd.cpp \
    getopt_helper.cpp
LIBPATH += ../qxmpp/
LIBPATH += ../miniupnp/
INCLUDEPATH += ../qxmpp/
INCLUDEPATH += ../miniupnp/
LIBS += -L/usr/local/lib \
    -lqjson \
    -lQXmppClient \
    -lminiupnp
!mac:x11:LIBS += -ldns_sd
win32:LIBS += -ldnssd

# Add your path to bonjour here.
# LIBPATH=C:/...
# INCLUDEPATH += c:/...
unix:LIBS += -ldns_sd
