#ifndef REMOTEIOCONNECTION_H
#define REMOTEIOCONNECTION_H

#include <QIODevice>
#include <QMutex>
#include <QWaitCondition>
#include <QDebug>
#include <QSharedPointer>

#include "controlconnection.h"
#include "library/library.h"
#include "conjist.h"

#include "remoteiodevice.h"
class RemoteIODevice;

class RemoteIOConnection : public Connection
{
    Q_OBJECT
public:
    RemoteIOConnection(int fid, Servent * s);
    ~RemoteIOConnection();
    QString id() const;
    QSharedPointer<RemoteIODevice> iodevice();
    void shutdown(bool wait = false);
    void setup();
    void handleMsg(QByteArray msg);
    Connection * clone();

signals:

private slots:
    void sendSome();

private:

    int m_fid;
    Servent * m_servent;
    QSharedPointer<RemoteIODevice> m_dev;
    QSharedPointer<QIODevice> m_readdev;
};

#endif // REMOTEIOCONNECTION_H
