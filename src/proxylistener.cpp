#include "proxylistener.h"


ProxyListener::ProxyListener(Servent * servent, Connection * conn, QString key)
    : m_servent(servent), m_conn(conn), m_key(key)
{
    qDebug() << "CTOR ProxyListener";
    connect(conn, SIGNAL(finished()), this, SLOT(shutdown()));
    connect(conn, SIGNAL(failed()), this, SLOT(shutdown()));

    bool ok = listen(QHostAddress::Any, 0);
    //Q_ASSERT(ok);
    if(ok) qDebug() << "ProxyListener listening on port " << this->serverPort();
}

ProxyListener::~ProxyListener()
{
    qDebug() << "DTOR proxylistener";
}

void ProxyListener::incomingConnection(int sd)
{
    qDebug() << "Accepting connection";
    QTcpSocket * sock = new QTcpSocket;
    if( !sock->setSocketDescriptor(sd) )
    {
        qDebug() << "Out of system resources for new ports?";
        Q_ASSERT(false);
        return;
    }
    // establish a p2p ProxyConnection for this socket:
    ProxyConnectionListener * pc = new ProxyConnectionListener(m_servent);
    pc->m_psock = sock;
    connect(pc, SIGNAL(ready()), pc, SLOT(flushBuffer()));
    //connect(pc, SIGNAL(failed()), pc, SLOT(shutdown()));
    //connect(pc, SIGNAL(finished()), pc, SLOT(shutdown()));
    connect(sock, SIGNAL(readyRead()), pc, SLOT(proxyReadyRead()));
    connect(sock, SIGNAL(disconnected()), pc, SLOT(proxyDisconnected()));
    m_servent->createParallelConnection(m_conn, pc, m_key);
    qDebug() << "proxylistener connection accepted.";
}

void ProxyListener::shutdown()
{
    close();
    deleteLater();
}

