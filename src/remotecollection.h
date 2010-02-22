#ifndef REMOTECOLLECTION_H
#define REMOTECOLLECTION_H
#include <QWaitCondition>
#include <QMutex>
#include <QSharedPointer>

#include "qdaapd/qdaapd.h"
#include "qdaapd/collection.h"
#include "connection.h"
#include "controlconnection.h"
#include "servent.h"
#include "library/library.h"
#include "conjist.h"

#include "remotecollectionconnection.h"

class RemoteCollectionConnection;

class RemoteCollection : public QDaap::Collection
{
Q_OBJECT
public:
    explicit RemoteCollection(Servent * s, RemoteCollectionConnection * conn);
    ~RemoteCollection();

    virtual void loadTracks();
    virtual QSharedPointer<QIODevice> getTrackIODevice(quint32 id);

signals:

public slots:
    void connAllTracks(QList<QDaap::Track> tracks);


private:
    Servent * m_servent;
    RemoteCollectionConnection * m_rcconn;
};







#endif // REMOTECOLLECTION_H
