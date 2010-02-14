#include <QTime>

#include "servent.h"
#include "controlconnection.h"
#include "proxylistener.h"
#include "proxyconnection.h"
#include "remotecollection.h"
#include "remoteioconnection.h"
#include "remotecollectionconnection.h"



// TODO auth timeout check

Servent::Servent(QHostAddress ha, int port, QObject *parent) :
    QTcpServer(parent), m_port(port), m_externalPort(0)
{
    bool ok = listen(ha, m_port);
    qDebug() << "Listening on port " << m_port << " thread: " << this->thread();
    Q_ASSERT(ok);

    // bonjour setup:

    m_bonjourbrowser = new BonjourServiceBrowser(this);
    connect(m_bonjourbrowser, SIGNAL(currentBonjourRecordsChanged(const QList<BonjourRecord> &)),
            this, SLOT(updateBonjourRecords(const QList<BonjourRecord> &)));

    m_bonjourresolver = new BonjourServiceResolver(this);
    connect(m_bonjourresolver, SIGNAL(bonjourRecordResolved(const QHostInfo &, int)),
            this, SLOT(bonjourRecordResolved(const QHostInfo &, int)));

    m_bonjourregister = new BonjourServiceRegister(this);

    m_bonjourbrowser->browseForServiceType(QLatin1String("_daap._tcp"));

    qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));
    //QTimer::singleShot(0, this, SLOT(saySomething()));
}

void Servent::setExternalAddress(QHostAddress ha, int port)
{
    m_externalAddress = ha;
    m_externalPort = port;
}

void Servent::registerOffer(QString key, Connection * conn)
{
    m_offers[key] = conn;
}

void Servent::registerRemoteCollectionConnection(RemoteCollectionConnection * conn)
{
    m_remotecollectionconnections.append(conn);

    qDebug() << "RemoteCollection registered: " << conn->name() << " numtracks: " << conn->remoteCollection()->numTracks();

    if(!conn->outbound()) return;

    qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));
    int port = qrand() % 50000 + 10000; //TODO
    qDebug() << "Remote Collection listening and advertising on DAAP PORT " << port;
    QDaap::QDAAPd * dd = new QDaap::QDAAPd(conn->remoteCollection(), port);
    connect(conn, SIGNAL(finished()), dd, SLOT(deleteLater()));
    // advertise this daap port:
    BonjourRecord rec(QString("conjist:%1").arg(conn->name()),
                      QLatin1String("_daap._tcp"), QString());
    m_bonjourregister->registerService(rec, port);

    //qDebug() << "TESTTTTTTTTTTTTT";
    //QIODevice *  dev = conn->remoteCollection()->getTrackIODevice(1);


}

void Servent::unregisterRemoteCollectionConnection(RemoteCollectionConnection * conn)
{
    QList<RemoteCollectionConnection*> n;
    foreach(RemoteCollectionConnection * c, m_remotecollectionconnections) if(c!=conn) n.append(c);
    m_remotecollectionconnections = n;
}

void Servent::registerControlConnection(ControlConnection * conn)
{
    m_controlconnections.append(conn);

    if(!conn->outbound())
    {
        qDebug() << "Sending invite for library sharing...";
        QString key = uuid();
        Library * lib = ((conjist*)QCoreApplication::instance())->library();
        RemoteCollectionConnection * rcconn = new RemoteCollectionConnection(lib, conn, this);
        rcconn->setName(conn->name());
        //connect(rcconn, SIGNAL(ready()), this, SLOT(advertiseCollection()));
        connect(rcconn, SIGNAL(finished()), rcconn, SLOT(deleteLater()));
        registerOffer(key, rcconn);
        qDebug() << "Registered a RCConn using " << key;
        QByteArray msg = QString("{\"method\":\"library-offer\", \"key\":\"%1\", \"name\":\"%2\"}")
                         .arg(key).arg(key).toAscii();
        foreach(ControlConnection * cc, m_controlconnections)
        {
            cc->sendMsg(msg);
        }
    }


    if(conn->outbound())
    {
        // TESTING: init a file transfer
        //qDebug() << "Init a file transfer as a test:";
        //QIODevice * dev = m_coll->getTrackIODevice(1);
        /*
        RemoteIOConnection * ioc = new RemoteIOConnection(0, this);
        QIODevice * dev = ioc->iodevice();
        connect(dev, SIGNAL(readyRead()), this ,SLOT(testRR()));
        createParallelConnection(conn, ioc, "FILE_REQUEST_KEY:1");
        */
    }


    /*
    foreach(ProxyConnection * pc, m_proxyconnections)
    {
        qDebug() << "Advertising existing daap proxies to new peer";
        QByteArray msg = QString("{\"method\":\"daap-offer\", \"key\":\"%1\", \"name\":\"%2\"}")
                        .arg(pc->name()).arg(pc->name()).toAscii();
        conn->sendMsg(msg);
    }
    */
}

void Servent::testRR()
{
    QIODevice * dev = (QIODevice*)sender();
    //RemoteCollection * c = (RemoteCollection*)sender();
    // TESTING: init a file transfer
    //qDebug() << "Init a file transfer as a test:";
    //QIODevice * dev = c->getTrack(1);
    int ba = dev->bytesAvailable();
    qDebug() << "Bytes available: " << ba;
    if(ba==0) return;
    QByteArray bytes = dev->readAll();//dev->readAll(); // read(ba);
    qDebug() << "dev->readAll(): " << bytes.length();
}

void Servent::unregisterControlConnection(ControlConnection * conn)
{
    QList<ControlConnection*> n;
    foreach(ControlConnection * c, m_controlconnections) if(c!=conn) n.append(c);
    m_controlconnections = n;
}

void Servent::incomingConnection(int sd)
{
    qDebug() << "Accepting connection";
    QTcpSocketExtra * sock = new QTcpSocketExtra;
    sock->_disowned = false;
    sock->_outbound = false;
    sock->_bs = 0;
    if( !sock->setSocketDescriptor(sd) )
    {
        qDebug() << "Out of system resources for new ports?";
        Q_ASSERT(false);
        return;
    }
    connect(sock, SIGNAL(readyRead()), this, SLOT(readyRead()));
    connect(sock, SIGNAL(disconnected()), sock, SLOT(deleteLater()));
    qDebug() << "connection accepted.";
}

void Servent::readyRead()
{
    qDebug() << "readyReady()";
    QTcpSocketExtra * sock = (QTcpSocketExtra*)sender();
    if(sock->_disowned)
    {
        qDebug() << "Socket already disowned";
        return ;
    }
    if(sock->_bs == 0)
    {
        if(sock->bytesAvailable() < 4) return;
        quint32 lenn;
        sock->read((char*)&lenn, 4);
        sock->_bs = qFromBigEndian(lenn);
    }
    if(sock->bytesAvailable() < sock->_bs) return;
    QByteArray ba = sock->read(sock->_bs);
    qDebug() << "Servent read from new connection: " << QString::fromAscii(ba);
    bool ok;
    QString key, conntype;
    int pport = 0;
    QVariantMap m = parser.parse(ba, &ok).toMap();
    if(!ok)
    {
        qDebug() << "Invalid JSON on new conection, aborting";
        goto closeconnection;
    }
    conntype = m.value("conntype").toString();
    key      = m.value("key").toString();
    pport    = m.value("port").toInt();

    // the connected to us and want something we are offering
    if(conntype == "accept-offer" || "push-offer")
    {
        Connection * conn;
        if(key.startsWith("FILE_REQUEST_KEY:"))
        {
            int fid = key.right(key.length()-17).toInt();
            qDebug() << "Got a request for fid: " << fid;
            conn = new RemoteIOConnection(fid, this);
        }
        else if(!m_offers.contains(key))
        {
            qDebug() << "Invalid offer key on new connection";
            goto closeconnection;
        }
        else
        {
            conn = m_offers[key];
            if(conn->onceOnly())
            {
                // just use this object
                m_offers.remove(key);
            } else {
                // use a copy, leave original there - offer can be reused.
                conn = conn->clone();
            }
        }
        //conn->setName("Incoming-"+key);
        conn->setPeerPort(pport);
        // hand over the socket
        sock->_disowned = true;
        conn->setOutbound(sock->_outbound);
        conn->takeSocket((QTcpSocket*)sock); // this now connects to readyRead
        connect(conn, SIGNAL(finished()), conn, SLOT(deleteLater())); //TODO cleanup fun to unregister
        conn->start();
        disconnect(sock, SIGNAL(readyRead()), this, SLOT(readyRead()));
        disconnect(sock, SIGNAL(disconnected()), sock, SLOT(deleteLater()));
        return;
    }

    qDebug() << "Invalid or unhandled conntype";
    // fallthru to cleanup:

closeconnection:
    qDebug() << "Closing incoming connection, something was wrong.";
    sock->disconnectFromHost();
}


// creates a new tcp connection to peer from conn, handled by given connector
// new_conn is responsible for sending the first msg, if needed
void Servent::createParallelConnection(Connection * orig_conn, Connection * new_conn, QString key)
{
    qDebug() << "Servent::createParallelConnection, key:" << key;
    // if we can connect to them directly:
    if( orig_conn->outbound() )
    {
        connectToPeer(orig_conn->socket()->peerAddress(), orig_conn->peerPort(), new_conn, key);
    }
    else // ask them to connect to us:
    {
        QString tmpkey = uuid(); // random, temp
        m_offers[tmpkey] = new_conn;

        orig_conn->sendMsg(QString("{ \"conntype\" : \"request-offer\", "
                                   "  \"key\" : \"%1\", "
                                   "  \"offer\" : \"%2\" , "
                                   "  \"port\" : %3 }").arg(tmpkey).arg(key).arg(publicPort()).toAscii());

    }
}

/// for outbound connectiona
void Servent::socketConnected()
{
    qDebug() << "Servent::SocketConnected";
    QTcpSocketExtra * sock = (QTcpSocketExtra*)sender();
    disconnect(sock, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(socketError(QAbstractSocket::SocketError)));
    Connection * conn = sock->_conn;
    sock->_disowned = true;
    conn->setOutbound(sock->_outbound);
    conn->setPeerPort(sock->peerPort());
    conn->takeSocket((QTcpSocket*)sock);
    connect(conn, SIGNAL(finished()), conn, SLOT(deleteLater())); //TODO cleanup fun to unregister
    conn->start();
}

void Servent::socketError(QAbstractSocket::SocketError e)
{
    QTcpSocketExtra * sock = (QTcpSocketExtra*)sender();
    if(!sock)
    {
        qDebug() << "SocketError, sock is null";
        return;
    }
    Connection * conn = sock->_conn;
    qDebug() << "Servent::SocketError: " << e << " " << conn->id();
    if(!sock->_disowned)
    {
        conn->takeSocket((QTcpSocket*)sock); // so sock gets deleted by Connection
        conn->markAsFailed(); // will emit failure signal, then delete itself
    }
    //conn->markAsFailed(); // will emit failure signal, then delete itself
}

void Servent::connectToPeer(QHostAddress ha, int port, Connection * conn, QString key)
{
    qDebug() << "Servent::connectToPeer: " << ha.toString() << ":" << port;
    if(key.length() && conn->firstMessage().length()==0)
    {
        QVariantMap m;
        m["conntype"] = "accept-offer";
        m["key"]      = key;
        m["port"]     = publicPort();
        conn->setFirstMessage(m);
    }
    QTcpSocketExtra * sock = new QTcpSocketExtra();
    sock->_disowned = false;
    sock->_conn = conn;
    sock->_outbound = true;
    connect(sock, SIGNAL(connected()), this, SLOT(socketConnected()));
    connect(sock, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(socketError(QAbstractSocket::SocketError)), Qt::DirectConnection);
    sock->connectToHost(ha, port, QTcpSocket::ReadWrite);
    qDebug() << "tried to connectToHost (waiting on a connected signal)";
    QTimer::singleShot(0, this, SLOT(saySomething()));
}

void Servent::reverseOfferRequest(Connection * orig_conn, QString key, QString theirkey)
{
    qDebug() << "Servent::reverseOfferRequest received for " << key;
    if(!m_offers.contains(key))
    {
        qDebug() << "Invalid offer in reverseOfferReqyest";
        return;
    }
    Connection * new_conn = m_offers[key];
    new_conn->setName(orig_conn->name() + "::parallel");
    //new_conn->setName(key);
    // use a push-offer instead of default accept offer:
    QVariantMap m;
    m["conntype"] = "push-offer";
    m["key"]      = theirkey;
    m["port"]     = publicPort();
    new_conn->setFirstMessage(m);
    createParallelConnection(orig_conn, new_conn, QString());
}



void Servent::updateBonjourRecords(const QList<BonjourRecord> &list)
{

    QSet<BonjourRecord> newrecs;
    foreach(BonjourRecord record, list)
    {
        if(m_bonjourrecords.contains(record))
        {
            qDebug() << "Already found: " << record.id();
            newrecs.insert(record);
        } else {
            qDebug() << "New record: " << record.id();
            newrecs.insert(record);
            m_bonjourresolver->resolveBonjourRecord(record);
        }
    }
    QSet<BonjourRecord> removed = m_bonjourrecords.subtract(newrecs);
    foreach(BonjourRecord rec, removed)
    {
        qDebug() << "Vanished record: " << rec.id();
    }
    m_bonjourrecords = newrecs;
}

void Servent::bonjourRecordResolved(const QHostInfo &host, int port)
{
    const QList<QHostAddress> &addresses = host.addresses();

    // TODO this is a blocking DNS lookup, restructure for async
    QHostAddress ha;
    if(addresses.isEmpty())
    {
        QString hostname = host.hostName();
        if(hostname.endsWith('.')) hostname.chop(1);
        QHostInfo hi = QHostInfo::fromName(hostname);
        if(hi.addresses().isEmpty())
        {
            qDebug() << "No address found for " << hostname << " " << hi.errorString();
            return;
        }
        ha = hi.addresses().at(0);
    }
    else
    {
        ha = addresses.at(0);
    }

    qDebug() << "bonjourRecordResolved: " << ha.toString() << " port: " << port;

    // offer to our peers:
    /*
    QString key = uuid();
    ProxyConnection * proxc = new ProxyConnection(ha, port, this);
    proxc->setName(key);
    connect(proxc, SIGNAL(finished()), this, SLOT(unregisterProxyConnection()));
    m_proxyconnections.append(proxc);

    proxc->setOnceOnly(false);
    registerOffer(key, proxc);
    qDebug() << "Registered a proxyconnection using " << key;

    QByteArray msg = QString("{\"method\":\"daap-offer\", \"key\":\"%1\", \"name\":\"%2\"}")
                     .arg(key).arg(host.hostName()).toAscii();
    foreach(ControlConnection * cc, m_controlconnections)
    {
        cc->sendMsg(msg);
    }
    */

}

void Servent::createDaapListener(ControlConnection * conn, QString key, QString name)
{
    ProxyListener * pl = new ProxyListener(this, conn, key);
    BonjourRecord rec(QString("DAAP %1 via %2").arg(name).arg(""),
                      QLatin1String("_daap._tcp"), QString());
    m_bonjourregister->registerService(rec, pl->serverPort());
}

void Servent::createRemoteCollection(ControlConnection * conn, QString key, QString name)
{
    qDebug() << "Accepting an offer of a library from remote peer";
    Library * lib = ((conjist*)QCoreApplication::instance())->library();

    RemoteCollectionConnection * rcconn = new RemoteCollectionConnection(lib, conn, this);
    rcconn->setName(conn->name());
    connect(rcconn, SIGNAL(ready()), this, SLOT(advertiseCollection()));
    connect(rcconn, SIGNAL(finished()), rcconn, SLOT(deleteLater()));
    createParallelConnection(conn, rcconn, key);


}

void Servent::unregisterProxyConnection()
{
    qDebug() << "Servent::unregisterProxyConnection";
    ProxyConnection * pc = (ProxyConnection*)sender();
    m_proxyconnections.removeAll(pc);

}

void Servent::advertiseCollection()
{
    /*
    RemoteCollectionConnection * rcconn = (RemoteCollectionConnection*)sender();
    RemoteCollection * rc = rcconn->remoteCollection();
    // advertise this daap port:
    BonjourRecord rec(QString("conjist:%1").arg(rcconn->name()),
                      QLatin1String("_daap._tcp"), QString());
    m_bonjourregister->registerService(rec, rc->port());
*/

    //if(!m_controlconnections.at(0)->outbound()) return;
        
    // TESTING: init a file transfer
//    qDebug() << "Init a file transfer as a test:";
  //  RemoteIOConnection * ioc = new RemoteIOConnection(0, this);
    //QIODevice * dev = ioc->iodevice();
  //  connect(dev, SIGNAL(readyRead()), this ,SLOT(testRR()));
   // createParallelConnection(m_controlconnections.at(0), ioc, "FILE_REQUEST_KEY:1");
}

// debug stuff:


/*


void Servent::debug_handleLine(QString line)
{
    qDebug() << "Servent handling line " << line;
    QStringList list = line.split(" ");
    if(list.length()==0) return;

    if(list.value(0) == "connect" && list.length() == 4)
    {
        ControlConnection * c = new ControlConnection(this);
        c->setName("normal");
        connect(c, SIGNAL(ready()), this, SLOT(debug_connected()));
        connect(c, SIGNAL(failed()), this, SLOT(debug_failed()));
        this->connectToPeer(QHostAddress(list.value(1)), list.value(2).toInt(), c, list.value(3));
        return;
    }

    if(line == "connections")
    {
        foreach(ControlConnection * c, m_controlconnections)
        {
            qDebug() << "ControlConnection: " << c->id();
        }

        return;
    }

    if(line == "dupe")
    {
        ControlConnection * c = m_controlconnections.at(0);
        ControlConnection * cnew = new ControlConnection(this);
        cnew->setName("dupeof-"+c->id());
        connect(cnew, SIGNAL(ready()), this, SLOT(debug_connected_dupe()));
        connect(cnew, SIGNAL(failed()), this, SLOT(debug_failed()));
        createParallelConnection(c, cnew, "key2");
        return;
    }

    if(line == "proxy")
    {
        ControlConnection * c = m_controlconnections.at(0);
        //ProxyListener * pl = new ProxyListener(this, c, "proxykey", 8080);
        return;
    }

    if(list.length() == 2 && list.at(0) == "send")
    {
        foreach(ControlConnection * c, m_controlconnections)
        {
            qDebug() << "Sending to: " << c->id();
            c->sendMsg(list.at(1).toAscii());
        }
        return;
    }


}

void Servent::debug_connected()
{
    ControlConnection * conn = (ControlConnection*)sender();
    qDebug() << "Connnnnnected " << conn->id();
    conn->sendMsg(QByteArray("First post"));


    qDebug() << "Trying to dupe this connection";
    ControlConnection * cc = new ControlConnection(this);
    cc->setName("dupeconn");
    connect(cc, SIGNAL(ready()), this, SLOT(debug_connected_dupe()));
    connect(cc, SIGNAL(failed()), this, SLOT(debug_failed()));
    createParallelConnection(conn, cc, "key2");

}

void Servent::debug_connected_dupe()
{
    ControlConnection * conn = (ControlConnection*)sender();
    qDebug() << "Connnnnnected  " << conn->id();
    conn->sendMsg(QByteArray("First post"));
    
}

void Servent::debug_failed()
{
    ControlConnection * conn = (ControlConnection*)sender();
    qDebug() << "Faileddddddd " << conn->id();
    conn->shutdown();
}


*/

