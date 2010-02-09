#ifndef REMOTEIOCONNECTION_H
#define REMOTEIOCONNECTION_H

#include <QIODevice>
#include <QMutex>
#include <QWaitCondition>
#include <QDebug>
#include "controlconnection.h"
#include "library/library.h"
#include "conjist.h"

class RemoteIODevice : public QIODevice
{
    Q_OBJECT
public:

    RemoteIODevice() :
            m_eof(false), m_headers(false), m_filesize(0)
    {};

    // abuse this to only block until headers are available:
    bool waitForReadyRead ( int msecs )
    {
        if(!m_headers)
        {
            qDebug() << "RemoteIODevice::waitForReadyRead";
            m_wait.wait(&m_mut);
        }else
            qDebug() << "RemoteIODevice - no waiting required";

        return true;
    }

    qint64 size () const
    {
        return m_filesize;
    };

    qint64 readData ( char * data, qint64 maxSize )
    {
        m_mut_recv.lock();
        if(m_eof && m_buffer.length() == 0)
        {
            // eof
            m_mut_recv.unlock();
            return 0;
        }
        if(!m_buffer.length())
        {
            // wait for more data to arrive
            m_mut_recv.unlock();
            m_wait.wait(&m_mut);
            m_mut_recv.lock();
        }
        int len;
        if(maxSize>=m_buffer.length()) // whole buffer
        {
            len = m_buffer.length();
            memcpy(data, m_buffer.constData(), len);
            m_buffer.clear();
        } else { // partial
            len = maxSize;
            memcpy(data, m_buffer.constData(), len);
            m_buffer.remove(0,len);
        }
        m_mut_recv.unlock();
        return len;
    };

    qint64 writeData ( const char * data, qint64 maxSize )
    {
        Q_ASSERT(false);
        return 0;
    };

    void addData(QByteArray msg)
    {
        m_mut_recv.lock();
        if(msg.length()==0) m_eof=true;
        else
        {
            if(!m_headers)
            {
                m_filesize = QString::fromAscii(msg).toInt();
                m_headers = true;
                qDebug() << "RemIODev, filesize: " << m_filesize;
            }
            else m_buffer.append(msg);
        }
        m_mut_recv.unlock();
        m_wait.wakeAll();
        qDebug() << "just read: " << msg.length();
        //emit QIODevice::readyRead();
    }

private:
    QByteArray m_buffer;
    QMutex m_mut, m_mut_recv;
    QWaitCondition m_wait;
    bool m_eof, m_headers;
    int m_filesize;
};

class RemoteIOConnection : public Connection
{
    Q_OBJECT
public:
    RemoteIOConnection(int fid, Servent * s)
        : Connection(s), m_fid(fid), m_servent(s)
    {
        //setName(QString("%1[%2]").arg(conn->name()).arg(fid));
        m_dev = new RemoteIODevice();
        qDebug() << "CTOR " << id();
    };

    ~RemoteIOConnection()
    {
        qDebug() << "DTOR " << id();
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
