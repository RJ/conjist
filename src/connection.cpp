#include "connection.h"

Connection::Connection(Servent *parent)
    : m_peerport(0), m_bs(0), m_servent(parent), m_ready(false), m_onceonly(true)
{
    qDebug() << "CTOR Connection (super)";
    //moveToThread(this);
}

Connection::~Connection()
{
    qDebug() << "DTOR connection (super) " << id();
    m_sock->deleteLater();
}

void Connection::setFirstMessage(QVariantMap m)
{
    QJson::Serializer serializer;
    m_firstmsg = serializer.serialize( m );
}

void Connection::shutdown()
{
    if(m_sock && m_sock->isOpen())
    {
        qDebug() << "Conn " << id() << " shutdown()";
        m_sock->disconnectFromHost();
    }
    emit finished();
    //if(this->isRunning()) QThread::exit(0);
}

void Connection::takeSocket(QTcpSocket * sock)
{
    m_sock = sock;
}

void Connection::markAsFailed()
{
    qDebug() << "Connection " << id() << " FAILED";
    emit failed();
    shutdown();
}
/*
void Connection::run()
{
    Q_ASSERT(m_sock);
    QTimer::singleShot(0, this, SLOT(run_real()));
    exec();
}
*/

void Connection::start()
{
    moveToThread(this->thread());
    m_sock->moveToThread(this->thread());
    connect(m_sock, SIGNAL(disconnected()), this, SLOT(shutdown()));
    connect(m_sock, SIGNAL(readyRead()), this, SLOT(readyRead()));

    if(outbound()) //) && m_firstmsg.length())
    {
        Q_ASSERT(m_firstmsg.length());
        sendMsg(m_firstmsg);
    }
    else if(outbound() == false)
    {
        sendMsg(QByteArray("{ \"ready\" : true }"));
        //m_ready = true;
        //qDebug() << "Connection " << id() << " READY";
        //setup();
        //emit ready();
    }
}

QString Connection::id() const
{
    return QString("Connection(%1)").arg(m_name);
}

void Connection::readyRead()
{
    //qDebug() << "readyRead, m_bs: " << m_bs << " bytesavail: " << m_sock->bytesAvailable();
    if(m_bs==0)
    {
        if(m_sock->bytesAvailable() < (int)sizeof(quint32))
        {
            //qDebug() << "Not enough bytes for getting the size header";
            return;
        }
        quint32 bs;
        m_sock->read((char*)&bs, sizeof(quint32));
        m_bs = qFromBigEndian(bs);
        //qDebug() << "Packet size: " << m_bs;
    }
    if(m_sock->bytesAvailable() < m_bs){
        //qDebug() << "Not enough bytes available (avail: " << m_sock->bytesAvailable()
        //        << " m_bs: " << m_bs << ")";
        return;
    }
    QByteArray ba = m_sock->read(m_bs);
    m_bs = 0;    
    //qDebug() << "Connection::readyRead() RCVD ("<< ba.length() << ") : " << QString::fromAscii(ba);

    if(outbound() == false && QString::fromAscii(ba) == "{ \"ready2\" : true }")
    {
        m_ready = true;
        qDebug() << "Connection " << id() << " READY";
        setup();
        emit ready();
    }
    else if(!m_ready && outbound() && QString::fromAscii(ba) == "{ \"ready\" : true }")
    {
        sendMsg(QByteArray("{ \"ready2\" : true }"));
        m_ready = true;
        qDebug() << "Connection " << id() << " READY";
        setup();
        emit ready();
    }
    else handleMsg(ba);

    // since there is no explicit threading, use the event loop to schedule this:
    if(m_sock->bytesAvailable()) QTimer::singleShot(0, this, SLOT(readyRead()));
}

void Connection::sendMsg(QByteArray msg)
{
    if(!m_sock->isOpen())
    {
        qDebug() << "Can't send, socket not open";
        return;
    }
    if(msg.at(0)=='{') qDebug() << id() << "SENDING: " << QString::fromAscii(msg);
    quint32 size = qToBigEndian(msg.length());
    m_sock->write((const char*)&size, sizeof(quint32));
    m_sock->write(msg);
}

void Connection::handleMsg(QByteArray  msg )
{
    qDebug() << "Connection::handleMsg: " << QString::fromAscii(msg);
}
