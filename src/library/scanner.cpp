#include <QTimer>
#include <QDebug>
#include <QDateTime>
#include "library/scanner.h"

MusicScanner::MusicScanner(QString dir)
    : QThread(), m_dir(dir)
{
    moveToThread(this); // ensure our thread has correct affinity
    m_validExts << ".mp3" << ".m4a" << ".mp4" << ".aac";
}

void MusicScanner::run()
{
    // start scanning once our event loop is setup
    QTimer::singleShot(0, this, SLOT(scan()));
    exec(); // blocks till event loop complete
}

void MusicScanner::scan()
{
    m_scanned = m_skipped = 0;
    qDebug() << "Scanning: " << m_dir;
    QDir dir(m_dir, 0);
    scanDir(dir, 0);
    emit(finished(m_scanned, m_skipped));
    qDebug() << "scanner thread done, scanned " << m_scanned << " skipped " << m_skipped;
}


void MusicScanner::scanDir(QDir dir, int depth)
{
    //qDebug() << "Scan: " << QString("-").repeated(depth*2) << " " << dir.absolutePath();
    dir.setFilter(QDir::Files | QDir::Readable);
    dir.setSorting(QDir::Name);
    QFileInfoList list = dir.entryInfoList();
    for(int i=0; i<list.size(); i++)
    {
        scanFile(list.at(i), depth);
    }
    dir.setFilter(QDir::Dirs | QDir::Readable);
    QStringList dirs = dir.entryList();
    foreach(QString d, dirs)
    {
        //qDebug() << d;
        if(d=="." || d=="..") continue;
        scanDir(dir.filePath(d), depth+1);
    }
}

void MusicScanner::scanFile(const QFileInfo &fi, int depth)
{    
    //qDebug() << "scanFile: "<< fi.absolutePath();
    foreach(QString e, m_validExts)
        if(fi.fileName().endsWith(e, Qt::CaseInsensitive)) goto extOk;
    m_skipped++;
    return; // invalid extension
extOk:
    //qDebug() << "Scan: " << QString("-").repeated(depth*2) << " " << fi.absoluteFilePath();
    TagLib::FileRef f( fi.absoluteFilePath().toUtf8().constData() );
    if(f.isNull() || !f.tag())
    {
        //qDebug() << "no tags, skip";
        m_skipped++;
        return;
    }

    TagLib::Tag *tag = f.tag();
    int bitrate = 0;
    int duration = 0;
    if (f.audioProperties()) {
        TagLib::AudioProperties *properties = f.audioProperties();
        duration = properties->length();
        bitrate = properties->bitrate();
    }
    QString artist = QString(tag->artist().toCString(true)).trimmed();
    QString album  = QString(tag->album().toCString(true)).trimmed();
    QString track  = QString(tag->title().toCString(true)).trimmed();

    if (artist.length()==0 || track.length()==0) {
        //qDebug()<< "NOTAGS";
        m_skipped++;
        return ;
    }

    QString mimetype("audio/mpeg");
    QString url("file://");
    // handle file:///home/rj/foo.mp3 and file:///c:/music/foo.mp3
    if(fi.absoluteFilePath().at(1)==':') url +="/"+fi.absoluteFilePath();
    else url+=fi.absoluteFilePath();

    QVariantMap m;
    m["url"]        = url;
    m["mtime"]      = fi.lastModified().toTime_t();
    m["size"]       = (unsigned int)fi.size();
    m["hash"]       = ""; // TODO
    m["mimetype"]   = mimetype;
    m["duration"]   = duration;
    m["bitrate"]    = bitrate;
    m["artist"]     = artist;
    m["album"]      = album;
    m["track"]      = track;
    m["trackno"]    = tag->track();

    m_scanned++;
    emit(fileScanned(m));
}


