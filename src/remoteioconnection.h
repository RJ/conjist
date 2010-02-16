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
        //m_dev = new QxtPipe();
        m_dev = new RemoteIODevice();
        qDebug() << "CTOR " << id() ;
    };

    ~RemoteIOConnection()
    {
        qDebug() << "DTOR " << id() ;
    };

    QString id() const { return QString("RemoteIOConnection[%1]").arg(m_fid); };

    RemoteIODevice * iodevice() { return m_dev; };


    void shutdown(bool wait = false)
    {
        if(!wait)
        {
            Connection::shutdown(wait);
            return;
        }
        qDebug() << id() << " shutdown requested - waiting until we've received all data TODO";

    };

    void setup()
    {


        qDebug() << "RemoteIOConnection::setup";
        if(m_fid)
        {
            qDebug() << "We are the sender, for fid " << m_fid;
            Library * lib = ((conjist*)QCoreApplication::instance())->library();
            QVariantMap t = lib->file(m_fid);
            Q_ASSERT(!t.isEmpty()); // TODO
            QString url = t.value("url").toString().remove(QRegExp("^file://"));
            qDebug() << "Opening " << url;
            m_readdev = new QFile(url);
            m_readdev->open(QIODevice::ReadOnly);
            if(!m_readdev->isOpen())
            {
                qDebug() << "WARNING file is not readable :/";
                shutdown();
            }
            // send chunks within our event loop, since we're not in our own thread
            sendSome();
        }
        else
        {
            qDebug() << "We are the receiver";
        }

    };

    void handleMsg(QByteArray msg)
    {
        Q_ASSERT(m_fid == 0); // only one end sends!
        //qDebug() << "got data msg, size " << msg.length();
        m_dev->addData(msg);
        //
        if(msg.length() == 0)
        {
            //qDebug() << "Closing connection, file transfer complete.";
            //shutdown();
        }
    };


    Connection * clone() { Q_ASSERT(false); return 0; };

signals:

private slots:
    void sendSome()
    {
        if(m_readdev->atEnd())
        {
            qDebug() << "Sent all. DONE";
            shutdown(true);
            return;
        }
        QByteArray ba = m_readdev->read(4096);
        //qDebug() << "Sending " << ba.length() << " bytes of audiofile";
        sendMsg(ba);
        QTimer::singleShot(0, this, SLOT(sendSome()));
    };


private:
    QIODevice * m_readdev;
    int m_fid;
    Servent * m_servent;
    RemoteIODevice * m_dev;
    //QxtPipe * m_dev;

};

#endif // REMOTEIOCONNECTION_H
