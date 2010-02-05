#ifndef PROXYCONNECTIONLISTENER_H
#define PROXYCONNECTIONLISTENER_H

#include <QTcpServer>
#include <QTcpSocket>

#include "connection.h"
#include "servent.h"


class ProxyConnectionListener : public Connection
{
    Q_OBJECT
public:
    QTcpSocket * m_psock;
    QList<QByteArray> m_buf;

    ProxyConnectionListener(Servent * s = 0) : Connection(s)
    {
        qDebug() << "CTOR " << id();
    };

    ~ProxyConnectionListener()
    {
        qDebug() << "DTOR " << id();
      if(m_psock->isOpen())
      {
          connect(m_psock, SIGNAL(disconnected()), m_psock, SLOT(deleteLater()));
          m_psock->disconnectFromHost();
      }
    };

    QString id() const { return "ProxyConn"; };

    Connection * clone() { Q_ASSERT(false); return 0; };

    void setup(){};

protected:
    void handleMsg(QByteArray msg)
    {
        if(!m_psock->isOpen())  return;
        m_psock->write(msg);
    };

public slots:

    void flushBuffer()
    {
        if(!m_ready) return;
        foreach(QByteArray ba, m_buf)
        {
            sendMsg(ba);
        }
    };

    void proxyReadyRead()
    {
        QByteArray ba = m_psock->readAll();
        if(m_ready)
        {
            sendMsg(ba);
        } else {
            m_buf.append(ba);
        }
    };

    void proxyDisconnected()
    {
        shutdown();
    };


};

#endif
