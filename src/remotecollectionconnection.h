#ifndef REMOTECOLLECTIONCONNECTION_H
#define REMOTECOLLECTIONCONNECTION_H
#include "connection.h"
#include "remotecollection.h"
#include "library/library.h"
#include "remotecollection.h"

class RemoteCollection;

class RemoteCollectionConnection : public Connection
{
    Q_OBJECT
public:
    RemoteCollectionConnection(Library * lib, ControlConnection * cc, Servent * s = 0 )
        : Connection(s), m_library(lib), m_cc(cc)
    {
        qDebug() << "CTOR " << id();
    };

    ~RemoteCollectionConnection()
    {
        qDebug() << "DTOR " << id();
    };

    QString id() const { return "RemoteCollectionConnection::" + name(); };

    Connection * clone() { Q_ASSERT(false); return 0; };

    void setup();

    RemoteCollection * remoteCollection() const { return m_rc; };

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
                    t.extension = "mp3";//m.value("extension").toString();
                    t.bitrate   = m.value("bitrate").toInt();
                    t.duration  = m.value("duration").toInt() * 1000;
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

    ControlConnection * cc() const { return m_cc; };

signals:
    void allTracks(QList<QDaap::Track>);
private:
    Library * m_library;
    QJson::Parser parser;
    RemoteCollection * m_rc;
    ControlConnection * m_cc;
};

#endif // REMOTECOLLECTIONCONNECTION_H
