#ifndef COLLECTION_H
#define COLLECTION_H

#include <QObject>
#include <QList>
#include <QMap>
#include <QFile>
#include <QMutex>
#include <QWaitCondition>
#include <QDebug>

namespace QDaap
{


class Track
{
public:
    Track() : extension("mp3"), disabled(false), trackcount(1), albumdiscnumber(1)
    {};

    quint32 id;
    quint64 perid;
    QString genre;
    QString artist;
    QString album;
    QString track;
    QString comment;
    QString extension;
    bool disabled;
    quint16 bitrate;
    quint32 duration; // ms
    quint32 filesize;
    quint16 trackcount;
    quint16 albumposition;
    quint16 year;
    quint16 albumdiscnumber;
};



/*
class Playlist
{
public:
    quint32 id;
    quint64 perid;
    QString title;
    quint32 length; // num tracks in playlist

    virtual QList<Track> getTracks()
    {
        return QList<Track>();
    };
};
*/

class Collection : public QObject
{
Q_OBJECT
public:
    explicit Collection(QString name, QObject *parent = 0);

    QString name() const { return m_name; };

    int numTracks();
    QMap<quint32,Track> & tracks();

    int numPlaylists() const;

    // this should hit DB/peer get full current list of all tracks:
    virtual void loadTracks();

    virtual QIODevice * getTrack(quint32 id);

    //QList<Playlist> playlists() const = 0;

    bool isLoaded() const { return m_tracksloaded; };


signals:
    void finishedLoading();

public slots:

protected:

    QString m_name;

    QMap<quint32,Track> m_tracks;
    bool m_tracksloaded;
};

} // ns

#endif // COLLECTION_H
