#include "remotecollection.h"
#include "remoteioconnection.h"

RemoteCollection::RemoteCollection(Servent * s, RemoteCollectionConnection * conn) :
        QDaap::Collection(conn->name(), conn), m_servent(s), m_rcconn(conn)
{
    //connect(m_rcconn, SIGNAL(ready()), this, SLOT(connReady()));
    connect(m_rcconn, SIGNAL(allTracks(QList<QDaap::Track>)), this, SLOT(connAllTracks(QList<QDaap::Track>)));

    m_port = qrand() % 50000 + 10000; //TODO
    qDebug() << "CTOR RemoteCollection, listening on " << m_port;
    QDaapdThread * t = new QDaapdThread(this, m_port);
    t->start();
    //m_h = new Httpd(this, m_port);
}

RemoteCollection::~RemoteCollection()
{
    qDebug() << "DTOR RemoteCollection";
    m_h->deleteLater();
}


void RemoteCollection::loadTracks()
{
    // nothing - the connection is already expecting to receive the full list
}

void RemoteCollection::connAllTracks(QList<QDaap::Track> tracks)
{
    foreach(QDaap::Track t, tracks) m_tracks[t.id] = t;
    m_tracksloaded = true;
    emit finishedLoading();
}

QIODevice * RemoteCollection::getTrack(quint32 id)
{
    Q_ASSERT(m_tracksloaded);
    if(!m_tracks.contains(id))
    {
        qDebug() << "m_tracks DOES NOT CONTAIN id " << id;
        return 0;
    }
    //QDaap::Track t = m_tracks[id];
    RemoteIOConnection * ioc = new RemoteIOConnection(id, m_servent);
    // magic offer key that will serve up the track (doesn't need to be pre-registered):
    m_servent->createParallelConnection(m_rcconn->cc(), ioc, QString("FILE_REQUEST_KEY:%1").arg(id));
    return ioc->iodevice();
};
