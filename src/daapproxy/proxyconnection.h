#ifndef PROXYCONNECTION_H
#define PROXYCONNECTION_H

#include "connection.h"
#include "servent.h"

class ProxyConnection : public Connection
{
Q_OBJECT
public:
    explicit ProxyConnection( QHostAddress ha, int port, Servent *parent = 0);
    ~ProxyConnection();
    Connection * clone();

    QString id() const
    {
        return QString("ProxyConnection(%1)").arg(m_name);
    }

    QHostAddress proxyHost() const { return m_proxiedaddress; };
    int proxyPort() const { return m_proxiedport; };


protected:

    virtual void handleMsg(QByteArray msg);
    virtual void setup();

signals:

public slots:
    void proxyReadyRead();
    void proxyDisconnected();
    void proxyConnected();

private:
    QHostAddress m_proxiedaddress;
    int m_proxiedport;
    QTcpSocket * m_proxysocket;
    QList<QByteArray> m_sendbuffer;
    bool m_dead;
};

#endif // PROXYCONNECTION_H
