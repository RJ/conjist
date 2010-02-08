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

private:
    Servent * m_servent;
    RemoteCollectionConnection * m_rcconn;

    QMutex m_mut_ready, m_mut_all;
    QWaitCondition m_waitcond_ready, m_waitcond_all;

    QList<QDaap::Track> m_tmp_tracks;

    Httpd * m_h;
};




class RemoteCollectionConnection : public Connection
{
    Q_OBJECT
public:
    RemoteCollectionConnection(Library * lib, Servent * s = 0 )
        : Connection(s), m_library(lib)
    {
        qDebug() << "CTOR " << id();
    };

    ~RemoteCollectionConnection()
    {
        qDebug() << "DTOR " << id();
    };

    QString id() const { return "RemoteCollectionConnection::" + name(); };

    Connection * clone() { Q_ASSERT(false); return 0; };

    void setup()
    {
        m_rc = new RemoteCollection(m_servent, this);
    };

    void handleMsg(QByteArray msg)
    {
        if(msg == "ALLTRACKSREQUEST")
        {
            // send all tracks
            QVariantList /*QList<QVariantMap>*/ all = m_library->allTracks();
            qDebug() << "Sending our library of " << all.length() << " tracks...";

            QVariantMap response;
            response["method"] = "alltracks";
            response["tracks"] = all;

            QJson::Serializer serializer;
            const QByteArray serialized = serializer.serialize( response );
            sendMsg(serialized);
            return;
        }

        bool ok;
        QVariantMap m = parser.parse(msg, &ok).toMap();
        if(ok)
        {
            if(m.value("method").toString() == "alltracks")
            {
                QList<QDaap::Track> tracks;

                QList<QVariant> all = m.value("tracks").toList();
                foreach(QVariant v, all)
                {
                    QVariantMap m = v.toMap();
                    QDaap::Track t;
                    t.perid     = t.id = m.value("id").toInt();
                    t.genre     = m.value("genre").toString();
                    t.artist    = m.value("artist").toString();
                    t.album     = m.value("album").toString();
                    t.track     = m.value("track").toString();
                    t.comment   = m.value("comment").toString();
                    t.extension = m.value("extension").toString();
                    t.bitrate   = m.value("bitrate").toInt();
                    t.duration  = m.value("duration").toInt();
                    t.filesize  = m.value("filesize").toInt();
                    t.year      = m.value("year").toInt();
                    t.albumdiscnumber   = m.value("albumdiscnumber").toInt();
                    t.albumposition     = m.value("albumposition").toInt();
                    tracks.append(t);
                }
                emit allTracks(tracks);
                return;
            }
        }

        qDebug() << "UNHANDLED msg in rcc: " << QString::fromAscii(msg);
    };

signals:
    void allTracks(QList<QDaap::Track>);
private:
    Library * m_library;
    QJson::Parser parser;
    RemoteCollection * m_rc;
};



#endif // REMOTECOLLECTION_H
