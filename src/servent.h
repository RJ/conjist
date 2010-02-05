#ifndef SERVENT_H
#define SERVENT_H

#include <QObject>
#include <QTcpServer>
#include <QHostInfo>
#include <QMap>
#include <QDebug>
#include <QtEndian>

#include <qjson/parser.h>
#include <qjson/serializer.h>
#include <qjson/qobjecthelper.h>

#include "zeroconf/bonjourrecord.h"
#include "zeroconf/bonjourservicebrowser.h"
#include "zeroconf/bonjourserviceregister.h"
#include "zeroconf/bonjourserviceresolver.h"

#include "connection.h"




class Connection;
class Connector;
class ControlConnection;
class ProxyConnection;

class QTcpSocketExtra : public QTcpSocket
{
    Q_OBJECT
public:
    quint32 _bs;
    Connection * _conn;
    bool _outbound;
    bool _disowned;
};

class Servent : public QTcpServer
{
Q_OBJECT
public:
    explicit Servent(QHostAddress ha, int port = 5555, QObject *parent = 0);
    int publicPort() const{ return m_port;};
    void createParallelConnection(Connection * orig_conn, Connection * new_conn, QString key);
    void registerOffer(QString key, Connection * conn);
    void registerControlConnection(ControlConnection * conn);
    void unregisterControlConnection(ControlConnection * conn);
    void connectToPeer(QHostAddress ha, int port, Connection * conn, QString key);
    void reverseOfferRequest(Connection * orig_conn, QString key, QString theirkey);
    void createDaapListener(ControlConnection * conn, QString key, QString name);

    void setExternalAddress(QHostAddress ha, int port);
    bool visibleExternally() const { return m_externalPort >0; };
    QHostAddress externalAddress() const { return m_externalAddress; };
    int externalPort() const { return m_externalPort; };

protected:
    void incomingConnection(int sd);

signals:

public slots:
    void socketError(QAbstractSocket::SocketError);
    void socketConnected();

    void debug_handleLine(QString line);
    void debug_failed();
    void debug_connected();
    void debug_connected_dupe();

private slots:
    void readyRead();
    void updateBonjourRecords(const QList<BonjourRecord> &);
    void bonjourRecordResolved(const QHostInfo &, int);
    void unregisterProxyConnection();

private:
    QJson::Parser parser;
    QList< ControlConnection * > m_controlconnections; // canonical list of authed peers
    QList< ProxyConnection  * > m_proxyconnections;
    QMap< QString, Connection* > m_offers;
    int m_port, m_externalPort;
    QHostAddress m_externalAddress;

    QSet<BonjourRecord> m_bonjourrecords;
    BonjourServiceBrowser * m_bonjourbrowser;
    BonjourServiceResolver * m_bonjourresolver;
    BonjourServiceRegister * m_bonjourregister;
};



#endif // SERVENT_H
