#include <QTime>
#include "connection.h"


Connection::Connection(Servent *parent)
    : QObject(), m_peerport(0), m_bs(0), m_servent(parent), m_ready(false), m_onceonly(true),
    m_totalsend_actual(0), m_totalsend_requested(0), m_do_shutdown(false), m_sock(0)
{
    qDebug() << "CTOR Connection (super)";
}

Connection::~Connection()
{
    qDebug() << "DTOR connection (super) " << id();
    if(m_sock) m_sock->deleteLater();
}

void Connection::setFirstMessage(QVariantMap m)
{
    QJson::Serializer serializer;
    m_firstmsg = serializer.serialize( m );
    qDebug() << id() << " first msg set to " << QString::fromAscii(m_firstmsg);
}

void Connection::shutdown(bool waitUntilSentAll)
{
    if(m_do_shutdown)
    {
        qDebug() << id() << " already shutting down";
        return;
    }
    m_do_shutdown = true;
    if(!waitUntilSentAll)
    {
        qDebug() << "Shutting down immediately " << id();
        actualShutdown();
    }
    qDebug() << "Shutting down after transfer complete " << id();
    bytesWritten(0); // trigger shutdown if we've already sent everything
    // otherwise the bytesWritten slot will call actualShutdown()
    // once all enqueued data has been properly written to the socket
}

void Connection::actualShutdown()
{
    qDebug() << "Conn " << id() << " actualShutdown()";
    if(m_sock && m_sock->isOpen())
    {
        //connect(m_sock, SIGNAL(disconnected()), this, SLOT(deleteLater()));
        m_sock->disconnectFromHost();
    }
    //quit();
    //m_sock->deleteLater();
    emit finished();
    deleteLater();
    //if(this->isRunning()) QThread::exit(0);
}

void Connection::takeSocket(QTcpSocket * sock)
{
    qDebug() << id() << " taking over socket";
    Q_ASSERT(!m_sock);
    m_sock = sock;
    connect(sock, SIGNAL(bytesWritten(qint64)), this, SLOT(bytesWritten(qint64)));
}

void Connection::markAsFailed()
{
    qDebug() << "Connection " << id() << " FAILED ***************";
    emit failed();
    shutdown();
}

void Connection::start()
{
    Q_ASSERT(m_sock);
    doSetup();
    //QTimer::singleShot(0, this, SLOT(doSetup()));
    //exec();
    //qDebug() << "Connection run() finishing";
}

void Connection::authCheckTimeout()
{
    if(m_ready) return;
    qDebug() << "Closing connection, not authed in time.";
    shutdown();
}

void Connection::doSetup()
{
    qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));
    //moveToThread(this->thread());
    m_sock->moveToThread(this->thread());
    connect(m_sock, SIGNAL(disconnected()), this, SLOT(socketDisconnected()), Qt::QueuedConnection);
    connect(m_sock, SIGNAL(readyRead()), this, SLOT(readyRead()), Qt::QueuedConnection);

    // if connection now authed/setup fast enough, kill it:
    QTimer::singleShot(5000, this, SLOT(authCheckTimeout()));

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

void Connection::socketDisconnected()
{
    qDebug() << id() << " socket disconnected, will shutdown";
    shutdown(true);
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
   //// qDebug() << "Connection::readyRead() RCVD ("<< ba.length() << ") : " << QString::fromAscii(ba);

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
    Q_ASSERT(m_do_shutdown == false);
    if(!m_sock->isOpen())
    {
        qDebug() << "Can't send, socket not open";
        return;
    }
    ////qDebug() << id() << "SENDING: " << QString::fromAscii(msg);
    quint32 size = qToBigEndian(msg.length());
    m_sock->write((const char*)&size, sizeof(quint32));
    m_sock->write(msg);
    m_totalsend_requested += msg.length() + sizeof(quint32);
}

void Connection::handleMsg(QByteArray  msg )
{
    qDebug() << "Connection::handleMsg: " << QString::fromAscii(msg);
}

void Connection::bytesWritten(qint64 i)
{
    m_totalsend_actual += i;
    qDebug() << "Connection - sent: " << m_totalsend_actual << " of: " << m_totalsend_requested;
    if(m_do_shutdown && m_totalsend_actual == m_totalsend_requested) actualShutdown();
}
