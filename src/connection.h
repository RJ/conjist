#ifndef CONNECTION_H
#define CONNECTION_H

#include <QThread>
#include <QTcpSocket>
#include <QHostAddress>
#include <QVariant>
#include <QVariantMap>
//#include <QSharedPointer>
#include <QString>
#include <QDebug>
#include <QDataStream>
#include <QtEndian>
#include <QTimer>

#include <qjson/parser.h>
#include <qjson/serializer.h>
#include <qjson/qobjecthelper.h>

#include "servent.h"

class Servent;

class Connection : public QObject
{
    Q_OBJECT
public:

    Connection(Servent *parent);
    virtual ~Connection();
    virtual Connection * clone() = 0;


    virtual QString id() const;
    void setFirstMessage(QVariantMap m);
    QString firstMessage() const { return m_firstmsg; };
    void takeSocket(QTcpSocket * sock);
    void setOutbound(bool o){ m_outbound = o; };
    bool outbound() const { return m_outbound; }
    Servent * servent() { return m_servent; };

    // get public port of remote peer:
    int peerPort() { return m_peerport; };
    void setPeerPort(int p) { m_peerport=p; };

    //void setFirstMsg(QByteArray ba) { m_firstmsg = ba; };
    QTcpSocket* socket() { return m_sock; };

    void markAsFailed();

    void setName(QString n) { m_name = n; };
    QString name() const { return m_name; };

    void setOnceOnly(bool b){ m_onceonly = b; };
    bool onceOnly() const { return m_onceonly; };

    bool isReady() const { return m_ready; } ;
    void start();

protected:

    virtual void handleMsg(QByteArray msg);
    virtual void setup(){};

signals:
    void ready();
    void failed();
    void finished();

public slots:
    void sendMsg(QByteArray);
    void shutdown(bool waitUntilSentAll = false);



private slots:
    void socketDisconnected();
    void readyRead();
    void doSetup();
    void authCheckTimeout();
    void bytesWritten(qint64);

protected:
    QTcpSocket* m_sock;
    int m_peerport;
    //int m_sd;
    quint32 m_bs;
    QJson::Parser parser;
    Servent * m_servent;
    bool m_outbound, m_ready, m_onceonly;
    QByteArray m_firstmsg;
    QString m_name;

private:
    void actualShutdown();
    bool m_do_shutdown;
    qint64 m_totalsend_actual, m_totalsend_requested;
};

#endif // CONNECTION_H
