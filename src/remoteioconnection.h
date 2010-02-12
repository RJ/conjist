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
        qDebug() << "no setup needed in remioconnection";
        QTimer::singleShot(1000, this, SLOT(writeStuff()));
        QTimer::singleShot(2000, this, SLOT(writeStuff()));
        QTimer::singleShot(3000, this, SLOT(writeStuff()));
        QTimer::singleShot(4000, this, SLOT(writeStuff()));
        return;

        qDebug() << "RemoteIOConnection::setup";
        if(m_fid)
        {
            qDebug() << "We are the sender, for fid " << m_fid;
            Library * lib = ((conjist*)QCoreApplication::instance())->library();
            QVariantMap t = lib->file(m_fid);
            Q_ASSERT(!t.isEmpty()); // TODO
            QString url = t.value("url").toString().remove(QRegExp("^file://"));
            qDebug() << "Opening " << url;
            QFile f(url);
            f.open(QIODevice::ReadOnly);
            if(f.isReadable())
            {
                char buf[4096];
                int i=0;
                while((i=f.read((char*)&buf, 4096))>0)
                {
                    qDebug() << "sending data msg";
                    sendMsg(QByteArray((char*)&buf, i));
                }
                sendMsg(QByteArray(""));
            }
            else
            {
                qDebug() << "WARNING file is not readable :/";
                shutdown();
            }
            // can't close it, not all data flushed down socket.
            // rely on other end to close it for us.
            //shutdown(); // close connection, we've sent the file.
        }
        else
        {
            qDebug() << "We are the receiver";
        }

    };

    void handleMsg(QByteArray msg)
    {
        Q_ASSERT(m_fid == 0); // only one end sends!
        qDebug() << "got data msg, size " << msg.length();
        m_dev->addData(msg);
        //
        if(msg.length() == 0)
        {
            qDebug() << "Closing connection, file transfer complete.";
            shutdown();
        }
    };


    Connection * clone() { Q_ASSERT(false); return 0; };

signals:

private slots:
    void writeStuff()
    {
        qDebug() << "Writing 5 bytes of stuff";
        m_dev->addData(QByteArray("XXXXX"));
    };


private:
    int m_fid;
    Servent * m_servent;
    RemoteIODevice * m_dev;

};

#endif // REMOTEIOCONNECTION_H
