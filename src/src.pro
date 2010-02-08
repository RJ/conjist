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
    collection.h \ #qdaap
    httpd.h \ #qdaap
    conjist.h \
    jabberclient.h \
    portfwd.h \
    getopt_helper.h \
    cjuuid.h \
    remotecollection.h \
    remotecollectionconnection.h \
    remoteioconnection.h
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
LIBPATH += ../qxmpp/
LIBPATH += ../miniupnp/
LIBPATH += ../qdaapd/
INCLUDEPATH += ../qxmpp/
INCLUDEPATH += ../miniupnp/
INCLUDEPATH += ../qdaapd/
INCLUDEPATH += ../qdaapd/daaplib/include/
INCLUDEPATH += /usr/include/taglib/
LIBS += -L/usr/local/lib \
    -lqjson \
    -lQXmppClient \
    -lminiupnp \
    -ltag \
    -lqdaapd

# don't link to dnssd on OSX (un tested qmake incantation)
# !macx{
# LIBS += -ldnssd
# }
# Add your path to bonjour here.
# LIBPATH=C:/...
# INCLUDEPATH += c:/...
# unix:LIBS += -ldns_sd
LIBS += -ldns_sd
