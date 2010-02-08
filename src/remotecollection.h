#ifndef REMOTECOLLECTION_H
#define REMOTECOLLECTION_H
#include <QWaitCondition>
#include <QMutex>

#include "collection.h"
#include "connection.h"
#include "controlconnection.h"
#include "servent.h"
#include "library/library.h"
#include "conjist.h"
#include "httpd.h"

#include "remotecollectionconnection.h"

class RemoteCollectionConnection;

class RemoteCollection : public QDaap::Collection
{
Q_OBJECT
public:
    explicit RemoteCollection(Servent * s, RemoteCollectionConnection * conn);
    ~RemoteCollection();
    virtual QList<QDaap::Track> loadTracks();

signals:

public slots:
    void connReady()
    {
        m_waitcond_ready.wakeAll();
    };

    void connAllTracks(QList<QDaap::Track> tracks)
    {
          m_tmp_tracks = tracks;
          m_waitcond_all.wakeAll();
    };

    int port() const { return m_port; };

    QIODevice * getTrack(quint32 id);


private:
    Servent * m_servent;
    RemoteCollectionConnection * m_rcconn;

    QMutex m_mut_ready, m_mut_all;
    QWaitCondition m_waitcond_ready, m_waitcond_all;

    QList<QDaap::Track> m_tmp_tracks;

    Httpd * m_h;
    int m_port;
};







#endif // REMOTECOLLECTION_H
