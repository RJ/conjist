TEMPLATE = app
OBJECTS_DIR = ../build
MOC_DIR = ../build
QT += network \
    xml \
    sql
QT -= gui
CONFIG -= app_bundle
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
    library/library.h \
    library/scanner.h \
    conjist.h \
    jabberclient.h \
    portfwd.h \
    getopt_helper.h \
    cjuuid.h \
    remotecollection.h \
    remotecollectionconnection.h \
    remoteioconnection.h \
    remoteiodevice.h

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
    library/library.cpp \
    library/scanner.cpp \
    conjist.cpp \
    jabberclient.cpp \
    portfwd.cpp \
    getopt_helper.cpp \
    remotecollection.cpp \
    remotecollectionconnection.cpp \
    remoteioconnection.cpp

# qdaapd library installation, see http://github.com/RJ/qdaapd
LIBPATH += ../../qdaapd/
INCLUDEPATH += ../../qdaapd/include
LIBS += -lqdaapd

# QXMPP (overly simple xmpp lib, will replace at some point)
LIBPATH += ../qxmpp/
INCLUDEPATH += ../qxmpp/
LIBS += -lQXmppClient

# Talks UPnP to set up port fwds on your router
LIBPATH += ../miniupnp/
INCLUDEPATH += ../miniupnp/
LIBS += -lminiupnp

# Taglib reads ID3 and other audio file metadata (use distro pkg)
INCLUDEPATH += /usr/include/taglib/
LIBS += -ltag

# QJson (use distro pkg)
LIBS += -lqjson

LIBS += -L/usr/local/lib

# TODO *don't* link to this on OSX, it's not needed:
# (don't know the qmake syntax for excluding on macs)
LIBS += -ldns_sd



