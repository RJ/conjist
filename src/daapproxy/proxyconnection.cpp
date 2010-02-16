#include "proxyconnection.h"

ProxyConnection::ProxyConnection( QHostAddress ha, int port, Servent *parent) :
    Connection(parent), m_proxiedaddress(ha), m_proxiedport(port), m_proxysocket(0), m_dead(false)
{
    qDebug() << "CTOR proxyconnection";
}

ProxyConnection::~ProxyConnection()
{
    qDebug() << "DTOR proxyconnection";
    if(m_proxysocket && m_proxysocket->isOpen())
    {
        connect(m_proxysocket, SIGNAL(disconnected()), m_proxysocket, SLOT(deleteLater()));
        m_proxysocket->disconnectFromHost();
    }
}

Connection * ProxyConnection::clone()
{
    ProxyConnection * clone = new ProxyConnection(proxyHost(), proxyPort(), servent());
    clone->setOnceOnly(onceOnly());
    clone->setName(name());
    return clone;
}

void ProxyConnection::setup()
{
    qDebug() << "ProxyConnector::setup (connecting to proxything)";
    m_proxysocket = new QTcpSocket();
    connect(m_proxysocket, SIGNAL(connected()), this, SLOT(proxyConnected()));
    connect(m_proxysocket, SIGNAL(readyRead()), this, SLOT(proxyReadyRead()));
    connect(m_proxysocket, SIGNAL(disconnected()), this, SLOT(proxyDisconnected()));
    m_proxysocket->connectToHost(m_proxiedaddress, m_proxiedport, QTcpSocket::ReadWrite);
}

void ProxyConnection::proxyConnected()
{
    qDebug() << "Connected to our designated proxy destination";
    foreach(QByteArray ba, m_sendbuffer)
    {
        //qDebug() << "flushing buffer";
        m_proxysocket->write(ba);
    }
    m_sendbuffer.clear();
}

void ProxyConnection::handleMsg(QByteArray ba)
{
    if(m_dead) shutdown(); // force die if they send once we're done
    //qDebug() << "Msg to relay";
    if(m_proxysocket->state() == QTcpSocket::ConnectedState)
    {
        //qDebug() << "sending from conn -> proxy("<< ba.length()<<"): " << QString::fromAscii(ba);
        m_proxysocket->write(ba);
    }
    else
    {
        //qDebug() << "sending to buffer";
        m_sendbuffer.append(ba);
    }
}

void ProxyConnection::proxyReadyRead()
{
    //qDebug() << "readyRead from proxy thing";
    QByteArray payload = m_proxysocket->readAll();
    //qDebug() << "RCVD from proxy: " << QString::fromAscii(payload);
    sendMsg(payload);
}

void ProxyConnection::proxyDisconnected()
{
    qDebug() << "disconnected from proxything, now closing our conn";
    m_dead = true;

}
