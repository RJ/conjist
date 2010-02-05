#ifndef PROXYLISTENER_H
#define PROXYLISTENER_H

#include <QTcpServer>

#include "servent.h"
#include "proxyconnectionlistener.h"

class ProxyListener : public QTcpServer
{
    Q_OBJECT
public:
    ProxyListener(Servent * servent, Connection * conn, QString key);

    ~ProxyListener();

signals:


protected:
    void incomingConnection(int sd);

private slots:
    void shutdown();

private:
    Servent * m_servent;
    Connection * m_conn;
    QString m_key;


};

#endif
