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
#include "cjuuid.h"

#include "qdaapd/qdaapd.h"

class Connection;
class Connector;
class ControlConnection;
class ProxyConnection;
class RemoteCollectionConnection;

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

    void registerRemoteCollectionConnection(RemoteCollectionConnection * conn);
    void unregisterRemoteCollectionConnection(RemoteCollectionConnection * conn);

    void connectToPeer(QHostAddress ha, int port, Connection * conn, QString key);
    void reverseOfferRequest(Connection * orig_conn, QString key, QString theirkey);
    void createDaapListener(ControlConnection * conn, QString key, QString name);
    void createRemoteCollection(ControlConnection * conn, QString key, QString name);

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

    //void debug_handleLine(QString line);
    //void debug_failed();
    //void debug_connected();
    //void debug_connected_dupe();

private slots:
    void readyRead();
    void updateBonjourRecords(const QList<BonjourRecord> &);
    void bonjourRecordResolved(const QHostInfo &, int);
    void unregisterProxyConnection();
    void advertiseCollection();

    void testRR();

private:
    QJson::Parser parser;
    QList< ControlConnection * > m_controlconnections; // canonical list of authed peers
    QList< RemoteCollectionConnection * > m_remotecollectionconnections; // canonical list of authed peers
    QList< ProxyConnection  * > m_proxyconnections;
    QMap< QString, Connection* > m_offers;
    int m_port, m_externalPort;
    QHostAddress m_externalAddress;

    QSet<BonjourRecord> m_bonjourrecords;
    BonjourServiceBrowser * m_bonjourbrowser;
    BonjourServiceResolver * m_bonjourresolver;
    BonjourServiceRegister * m_bonjourregister;
};



class Reader : public QObject
{
Q_OBJECT
public:
    QMutex mut;
    QWaitCondition wait;
    QIODevice * dev;
    Reader(QIODevice * d) : dev(d)
    {
        connect(dev, SIGNAL(readyRead()), this, SLOT(doread()));
    };

    void waitUntilFinished()
    {
        mut.lock();
        wait.wait(&mut);
        mut.unlock();
    };

private slots:
    void doread()
    {

        if(dev->bytesAvailable()>0)
        {
            QByteArray ba = dev->readAll();
            qDebug() << "READYREAD: Read " << ba.length() << " bytes";
        }
        if(dev->atEnd())
        {
            qDebug() << "REader thread ending, eof";
            wait.wakeAll();
            return;
        }
    }
};

#include <QThread>

class ReaderThread : public QThread
{
    Q_OBJECT
public:
    QIODevice * dev;
    ReaderThread(QIODevice * d)
    {
        dev = d;
    };
    void run()
    {
      Reader r(dev);
      r.waitUntilFinished();
      qDebug() << "ReaderThread ends";
    };
};

#endif // SERVENT_H
