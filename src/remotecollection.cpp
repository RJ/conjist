#include "remotecollection.h"
#include "remoteioconnection.h"

RemoteCollection::RemoteCollection(Servent * s, RemoteCollectionConnection * conn) :
        QDaap::Collection(conn->name(), conn), m_servent(s), m_rcconn(conn)
{
    connect(m_rcconn, SIGNAL(allTracks(QList<QDaap::Track>)), this, SLOT(connAllTracks(QList<QDaap::Track>)));
    qDebug() << "CTOR RemoteCollection";
}

RemoteCollection::~RemoteCollection()
{
    qDebug() << "DTOR RemoteCollection";
}


void RemoteCollection::loadTracks()
{
    // nothing - the connection is already expecting to receive the full list
}

void RemoteCollection::connAllTracks(QList<QDaap::Track> tracks)
{
    foreach(QDaap::Track t, tracks) m_tracks[t.id] = t;
    emit finishedLoading();
}

QIODevice * RemoteCollection::getTrackIODevice(quint32 id)
{
    Q_ASSERT(m_tracksloaded);
    if(!m_tracks.contains(id))
    {
        qDebug() << "m_tracks DOES NOT CONTAIN id " << id;
        return 0;
    }
    //QFile * f = new QFile("/tmp/test.mp3");
    //return f;

    //return new QFile("/tmp/test.mp3");


    //QDaap::Track t = m_tracks[id];
    RemoteIOConnection * ioc = new RemoteIOConnection(0, m_servent);
    ioc->moveToThread(m_servent->thread());
    // magic offer key that will serve up the track (doesn't need to be pre-registered):
    m_servent->createParallelConnection(m_rcconn->cc(), ioc, QString("FILE_REQUEST_KEY:%1").arg(id));
    return ioc->iodevice();
};
