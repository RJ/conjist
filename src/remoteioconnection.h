#ifndef REMOTEIOCONNECTION_H
#define REMOTEIOCONNECTION_H

#include <QIODevice>
#include <QMutex>
#include <QWaitCondition>
#include <QDebug>
#include "controlconnection.h"
#include "library/library.h"
#include "conjist.h"

#include "remoteiodevice.h"

class RemoteIOConnection : public Connection
{
    Q_OBJECT
public:
    RemoteIOConnection(int fid, Servent * s)
        : Connection(s), m_fid(fid), m_servent(s)
    {
        //setName(QString("%1[%2]").arg(conn->name()).arg(fid));
        m_dev = new RemoteIODevice();
        qDebug() << "CTOR RemoteIOConnection" ;
    };

    ~RemoteIOConnection()
    {
        qDebug() << "DTOR RemoteIOConnection" ;
    };

    RemoteIODevice * iodevice() { return m_dev; };

    void setup()
    {
        if(m_fid)
        {
            qDebug() << "We are the sender, for fid " << m_fid;
            Library * lib = ((conjist*)QCoreApplication::instance())->library();
            QVariantMap t = lib->file(m_fid);
            Q_ASSERT(!t.isEmpty()); // TODO
            QString url = t.value("url").toString();
            qDebug() << "Opening " << url;
            QFile f(url);
            f.open(QIODevice::ReadOnly);
            sendMsg(QString("%1").arg(f.size()).toAscii());
            char buf[4096];
            int i=0;
            while((i=f.read((char*)&buf, 4096))>0)
            {
                sendMsg(QByteArray((char*)&buf, i));
            }
        }
        else
        {
            qDebug() << "We are the receiver";
        }

    };

    void handleMsg(QByteArray msg)
    {
        Q_ASSERT(m_fid == 0); // only one end sends!
        m_dev->addData(msg);
    };


    Connection * clone() { Q_ASSERT(false); return 0; };

private:
    int m_fid;
    Servent * m_servent;
    RemoteIODevice * m_dev;

};

#endif // REMOTEIOCONNECTION_H
