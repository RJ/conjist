#include "remotecollection.h"
//#include "httpd.h"

RemoteCollection::RemoteCollection(Servent * s, RemoteCollectionConnection * conn) :
        QDaap::Collection(conn->name(), conn), m_servent(s), m_rcconn(conn)
{
    int port = (int)(qrand()*50000)/RAND_MAX;
    qDebug() << "CTOR RemoteCollection, listening on " << port;
    m_h = new Httpd(this, port);
}

RemoteCollection::~RemoteCollection()
{
    qDebug() << "DTOR RemoteCollection";
    m_h->deleteLater();
}


QList<QDaap::Track> RemoteCollection::loadTracks()
{
    // wait till connection is ready...
    qDebug() << "Is RemoteCollectionConnection ready?";
    if(!m_rcconn->isReady())
    {
        qDebug() << "Waiting.....";
        connect(m_rcconn, SIGNAL(ready()), this, SLOT(connReady()));
        m_mut_ready.lock();
        m_waitcond_ready.wait(&m_mut_ready);
        m_mut_ready.unlock();
    }
    qDebug() << "It's ready!";

    // request all tracks:
    QByteArray msg("ALLTRACKSREQUEST");
    m_rcconn->sendMsg(msg);

    // wait till all tracks are loaded
    connect(m_rcconn, SIGNAL(allTracks(QList<QDaap::Track>)),
            this,     SLOT(connAllTracks(QList<QDaap::Track>)));
    m_mut_all.lock();
    m_waitcond_all.wait(&m_mut_all);
    m_mut_all.unlock();
    qDebug() << "all tracks received";
    return m_tmp_tracks;
}
