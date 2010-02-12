#ifndef REMOTEIODEVICE_H
#define REMOTEIODEVICE_H
#include <QIODevice>
#include <QMutex>
#include <QWaitCondition>
#include <QDebug>
#include <QBuffer>
//#include <QMutexLocker>

class RemoteIODevice : public QIODevice
{
    Q_OBJECT
public:

    RemoteIODevice() :
            m_eof(false)
    {};

    virtual bool open ( OpenMode mode )
    {
        return QIODevice::open(mode & QIODevice::ReadOnly);// & QIODevice::Unbuffered);
    }

    qint64 bytesAvailable () const
    {
        return m_buffer.length();
    };

    virtual bool isSequential () const { return true; };

    virtual bool waitForReadyRead(int msecs)
    {

        qDebug() << "GOTLOCK";
        m_mut_wait.lock();
        if(m_buffer.length() == 0)
        {
            qDebug() << "WAITING (in thread " << this->thread() << ")";
            m_wait.wait(&m_mut_wait);
        }
        m_mut_wait.unlock();
        qDebug() << "UNLOCKED";
        return true;
    };

    virtual bool atEnd() const { return m_eof && m_buffer.length() == 0; };

public slots:

    void addData(QByteArray msg)
    {
        qDebug() << "RemIO::addData (trying..)";
        m_mut_recv.lock();
        qDebug() << "RemIO::addData (got lock) numbytes:" << msg.length();
        if(msg.length()==0)
        {
            m_eof=true;
            qDebug() << "addData finished, entire file received. EOF.";
            m_mut_recv.unlock();
            m_wait.wakeAll();
            return;
        }
        else
        {
            m_buffer.append(msg);
            m_mut_recv.unlock();
            qDebug() << "WAKE ALL - just added bytes to dev: " << msg.length();
            //emit bytesWritten(msg.length());
            m_wait.wakeAll();
            emit readyRead();
            return;
        }
    }

protected:

    virtual qint64 writeData ( const char * data, qint64 maxSize )
    {
        Q_ASSERT(false);
        return 0;
    };

    virtual qint64 readData ( char * data, qint64 maxSize )
    {
        qDebug() << "RemIO::readData, bytes in buffer: " << m_buffer.length();
        m_mut_recv.lock();
        if(m_eof && m_buffer.length() == 0)
        {
            // eof
            qDebug() << "readData called when EOF";
            m_mut_recv.unlock();
            return 0;
        }
        if(!m_buffer.length())// return 0;
        {
            qDebug() << "WARNING readData when buffer is empty";
            m_mut_recv.unlock();
            return 0;
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

private:
    QByteArray m_buffer;
    QMutex m_mut_wait, m_mut_recv;
    QWaitCondition m_wait;
    bool m_eof;
};
#endif // REMOTEIODEVICE_H
