#include <QRegExp>
#include <QStringList>
#include "library/library.h"
/* !!!! You need to manually generate this file when the schema changes:
    cd src/library
   ./gen_schema.h.sh ./schema.sql library > library.sql.h
*/
#include "library/library.sql.h"
#define CURRENT_SCHEMA_VERSION 2

Library::Library(const QString &dbname, QObject *parent)
    : QObject(parent)
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbname);
    if(!db.open())
    {
        qDebug() << "FAILED TO OPEN DB";
        throw;  // TODO
    }
    query = QSqlQuery(db);
    query.exec("SELECT v FROM settings WHERE k='schema_version'");
    if(query.next())
    {
        int v = query.value(0).toInt();
        qDebug() << "Current schema is " << v;
        if(v != CURRENT_SCHEMA_VERSION)
        {
            qDebug() << "Schema version too old: " << v << ". Current version is: " << CURRENT_SCHEMA_VERSION;
            updateSchema(v);
        }
    } else {
        updateSchema(0);
    }
}

Library::~Library(){}

bool Library::updateSchema(int currentver)
{
    qDebug() << "create tables... old version is " << currentver;
    QString sql(  get_library_sql() );
    QStringList statements = sql.split(";",QString::SkipEmptyParts);
    QString s1, s;
    foreach(s1, statements)
    {
        s = s1.trimmed();
        if(s.length()==0) continue;
        qDebug() << "Executing: " << s;
        query.exec(s);
    }
    return true;
}

bool Library::removeFile( const QString& url )
{
    query.prepare("SELECT id FROM file WHERE url = ?");
    query.addBindValue(url);
    query.exec();
    int fileid = 0;
    if( query.next() )
        fileid = query.value(0).toInt();
    else
        return false;
    query.exec(QString("DELETE FROM file_join WHERE file = %1").arg(fileid));
    query.exec(QString("DELETE FROM file WHERE id = %1").arg(fileid));
    return true;
}

void Library::addDir( const QString& url, int mtime)
{
    removeFile(url);
    query.prepare("INSERT INTO file(url, size, mtime) VALUES (?, 0, ?)");
    query.addBindValue(url);
    query.addBindValue(mtime);
    query.exec();
}

QVariantMap Library::file(int fid)
{
    QVariantMap m;
    query.exec(QString("SELECT url, size FROM file WHERE id = %1").arg(fid));
    if(query.next())
    {
        QString url = query.value(0).toString();
        int size= query.value(2).toInt();
        m["url"] = url;
        m["size"] = size;
    }
    return m;
}

int  Library::addFile( QVariantMap m )
{

    QString url         = m["url"].toString();
    int mtime           = m["mtime"].toInt();
    int size            = m["size"].toInt();
    QString hash        = m["hash"].toString();
    QString mimetype    = m["mimetype"].toString();
    int duration        = m["duration"].toInt();
    int bitrate         = m["bitrate"].toInt();
    QString artist      = m["artist"].toString();
    QString album       = m["album"].toString();
    QString track       = m["track"].toString();
    //int tracknum        = m["trackno"].toInt();

    int fileid = 0;
    removeFile(url);
    query.prepare("INSERT INTO file(url, size, mtime, md5, mimetype, duration, bitrate) "
                  "VALUES (?, ?, ?, ?, ?, ?, ?)");
    query.addBindValue(url);
    query.addBindValue(size);
    query.addBindValue(mtime);
    query.addBindValue(hash);
    query.addBindValue(mimetype);
    query.addBindValue(duration);
    query.addBindValue(bitrate);
    if( !query.exec() )
    {
        qDebug() << "Failed to insert to file";
        return 0;
    }

    // get internal IDs for art/alb/trk
    fileid = query.lastInsertId().toInt();
    int artid = artistId(artist);
    if( artid < 1 ){
        return 0;
    }
    int trkid = trackId(artid, track);
    if( trkid < 1 ){
        return 0;
    }
    int albid = albumId(artid, album);

    // Now add the association
    query.prepare("INSERT INTO file_join(file, artist ,album, track) VALUES (?,?,?,?)");
    query.addBindValue(fileid);
    query.addBindValue(artid);
    query.addBindValue(albid);
    query.addBindValue(trkid);
    if( !query.exec() )
    {
        qDebug() << "Error inserting into file_join table";
        return 0;
    }
    qDebug() << "Added fid " << fileid << " to library: " << artist << " - " << track;
    return fileid;
}


int Library::artistId(const QString& name_orig)
{
    int id = 0;
    QString sortname = Library::sortname(name_orig);
    //if((id = m_artistcache[sortname])) return id;
    query.prepare("SELECT id FROM artist WHERE sortname = ?");
    query.addBindValue(sortname);
    query.exec();
    if(query.next())
    {
        id = query.value(0).toInt();
    }
    if(id){
        //m_artistcache[sortname]=id;
        return id;
    }
    // not found, insert it.
    query.prepare("INSERT INTO artist(id,name,sortname) VALUES(NULL,?,?)");
    query.addBindValue(name_orig);
    query.addBindValue(sortname);
    if(!query.exec())
    {
        qDebug() << "Failed to insert artist: " << name_orig ;
        return 0;
    }
    id = query.lastInsertId().toInt();
    //m_artistcache[sortname]=id;
    return id;
}

int Library::trackId(int artistid, const QString& name_orig)
{
    int id = 0;
    QString sortname = Library::sortname(name_orig);
    //if((id = m_artistcache[sortname])) return id;
    query.prepare("SELECT id FROM track WHERE artist = ? AND sortname = ?");
    query.addBindValue(artistid);
    query.addBindValue(sortname);
    query.exec();
    if(query.next())
    {
        id = query.value(0).toInt();
    }
    if(id){
        //m_trackcache[sortname]=id;
        return id;
    }
    // not found, insert it.
    query.prepare("INSERT INTO track(id,artist,name,sortname) VALUES(NULL,?,?,?)");
    query.addBindValue(artistid);
    query.addBindValue(name_orig);
    query.addBindValue(sortname);
    if(!query.exec())
    {
        qDebug() << "Failed to insert track: " << name_orig ;
        return 0;
    }
    id = query.lastInsertId().toInt();
    //m_trackcache[sortname]=id;
    return id;
}

int Library::albumId(int artistid, const QString& name_orig)
{
    int id = 0;
    QString sortname = Library::sortname(name_orig);
    //if((id = m_albumcache[sortname])) return id;
    query.prepare("SELECT id FROM album WHERE artist = ? AND sortname = ?");
    query.addBindValue(artistid);
    query.addBindValue(sortname);
    query.exec();
    if(query.next())
    {
        id = query.value(0).toInt();
    }
    if(id){
        //m_albumcache[sortname]=id;
        return id;
    }
    // not found, insert it.
    query.prepare("INSERT INTO album(id,artist,name,sortname) VALUES(NULL,?,?,?)");
    query.addBindValue(artistid);
    query.addBindValue(name_orig);
    query.addBindValue(sortname);
    if(!query.exec())
    {
        qDebug() << "Failed to insert album: " << name_orig ;
        return 0;
    }
    id = query.lastInsertId().toInt();
    //m_albumcache[sortname]=id;
    return id;
}

QList< QPair<int, float> > Library::searchTable(const QString &table, const QString &name_orig)
{
    QList< QPair<int, float> > results;
    if(table != "artist" && table != "track" && table != "album") return results;
    if(name_orig.length()<3) return results;
    QString name = sortname(name_orig);
    QMap<QString,int> ngrammap = ngrams(name);
    QString qpart(",?");
    QString q("?");
    q += qpart.repeated(ngrammap.size()-1);
    QString sql= "SELECT s.id, sum(s.num) as score "
                 "FROM " + table + "_search_index as s "
                 "WHERE ngram IN (" + q + ") "
                 "GROUP BY s.id ORDER BY sum(s.num) DESC LIMIT 10";
    query.prepare(sql);
    QMapIterator<QString, int> i(ngrammap);
    while (i.hasNext()) {
        i.next();
        query.addBindValue(i.key());
    }
    query.exec();
    while(query.next())
    {
        results.append( QPair<int,float>(query.value(0).toInt(), query.value(1).toFloat()) );
    }
    return results;
}

QList< QPair<int, float> > Library::searchByArtist(int artistid, const QString &table, const QString &name_orig)
{
    QList< QPair<int, float> > results;
    if(table != "track" && table != "album") return results;
    if(name_orig.length()<3) return results;
    QString name = sortname(name_orig);
    QMap<QString,int> ngrammap = ngrams(name);
    QString qpart(",?");
    QString q("?");
    q += qpart.repeated(ngrammap.size()-1);
    QString sql =   "SELECT s.id, sum(s.num) as score "
                    "FROM " + table + "_search_index as s "
                    "JOIN " + table + " ON " +table+ ".id = s.id "
                    "WHERE " + table +".artist = ? AND "
                    "ngram IN (" + q + ") "
                    "GROUP BY s.id ORDER BY sum(s.num) DESC LIMIT 10";
    query.prepare(sql);
    query.bindValue(0, artistid);
    int numn = 1;
    QMapIterator<QString, int> i(ngrammap);
    while (i.hasNext()) {
        i.next();
        query.bindValue(numn++, i.key());
    }
    query.exec();
    while(query.next())
    {
        results.append( QPair<int,float>(query.value(0).toInt(), query.value(1).toFloat()) );
    }
    return results;
}

QList<int> Library::getTrackFids(int tid)
{
    QList<int> ret;
    query.exec(QString("SELECT file.id FROM file, file_join "
                       "WHERE file_join.file=file.id "
                       "AND file_join.track = %1 "
                       "ORDER BY bitrate DESC").arg(tid));
    query.exec();
    while(query.next()) ret.append(query.value(0).toInt());
    return ret;
}



bool Library::buildIndex(const QString &table)
{
    if(table != "artist" && table != "track" && table != "album") return false;

    qDebug() << "Building index for " << table ;
    QString searchtable(table + "_search_index");
    query.exec("DELETE FROM "+searchtable);
    query.exec("SELECT id, sortname FROM "+table);

    QSqlQuery upq,inq;
    inq.prepare("INSERT INTO "+searchtable+" (ngram, id, num) VALUES (?,?,?)");
    upq.prepare("UPDATE "+searchtable+" SET num=num+? WHERE ngram=? AND id=?");

    int num_names = 0;
    int num_ngrams = 0;
    int id;
    QString name;
    QMap<QString,int> ngrammap;

    while(query.next())
    {
        id = query.value(0).toInt();
        name = query.value(1).toString();
        num_names++;
        inq.bindValue(1, id); // set id
        upq.bindValue(2, id); // set id
        ngrammap = ngrams(name);
        QMapIterator<QString, int> i(ngrammap);
        while (i.hasNext())
        {
            i.next();
            num_ngrams++;
            upq.bindValue(0, i.value()); //num
            upq.bindValue(1, i.key());  // ngram
            upq.exec();
            if(upq.numRowsAffected()==0)
            {
                inq.bindValue(0, i.key()); //ngram
                inq.bindValue(2, i.value());//num
                inq.exec();
                //inq.clear();
            }
            //upq.clear();
        }
    }
    qDebug() << "Finished indexing " << table << " - " << num_names <<" names, " << num_ngrams << " ngrams.";
    return true;
}



QMap<QString,int> Library::ngrams(const QString& str_orig)
{
    int n=3;
    QMap<QString,int> ret;
    QString str(" " + sortname(str_orig) + " ");
    int num = str.length();
    QString ngram;
    for(int j = 0; j < num; j++){
         ngram = str.mid(j,n);
         if(ret.contains(ngram))
             ret[ngram]++;
         else ret[ngram]=1;
    }
    return ret;
}


QString Library::sortname(const QString& str)
{
    return str.toLower().trimmed().replace(QRegExp("[\\s]{2,}")," ");
}

QList<QVariantMap> Library::artists()
{
    QList<QVariantMap> results;
    QString sql =   "SELECT id, name, sortname "
                    "FROM artist "
                    "ORDER BY sortname ASC";
    query.exec(sql);
    while(query.next())
    {
        QVariantMap m;
        m["id"] = query.value(0);
        m["name"] = query.value(1);
        m["sortname"] = query.value(2);
        results.append(m);
    }
    return results;
}

QList<QVariant> Library::allTracks()
{
    QList<QVariant> tracks;
    QString sql =   "SELECT file.id, artist.name, album.name, track.name, file.size, file.duration, "
                    "file.bitrate "
                    "FROM file, file_join, artist, album, track "
                    "WHERE file.id = file_join.file AND "
                    "file_join.artist = artist.id AND "
                    "file_join.album = album.id AND "
                    "file_join.track = track.id "
                    "ORDER BY file.id ASC";
    query.prepare(sql);
    query.exec();
    while(query.next())
    {
        QVariantMap t;
        t["id"] = query.value(0).toInt();
        t["artist"] = query.value(1).toString();
        t["album"]  = query.value(2).toString();
        t["track"]  = query.value(3).toString();
        t["filesize"] = query.value(4).toInt();
        t["duration"] = query.value(5).toInt();
        t["bitrate"]  = query.value(6).toInt();
        tracks.append(t);
    }
    return tracks;
}


QList<QVariantMap> Library::tracks(int artistid)
{
    QList<QVariantMap> results;
    QString sql =   "SELECT id, artist ,name, sortname "
                    "FROM track "
                    "WHERE artist = ? "
                    "ORDER BY sortname ASC";
    query.prepare(sql);
    query.addBindValue(artistid);
    query.exec();
    while(query.next())
    {
        QVariantMap m;
        m["id"] = query.value(0);
        m["artist"] = query.value(1);
        m["name"] = query.value(2);
        m["sortname"] = query.value(3);
        results.append(m);
    }
    return results;
}

QVariantMap Library::artist(int id)
{
    query.exec(QString("SELECT id, name, sortname FROM artist WHERE id = %1").arg(id));
    QVariantMap m;
    if(!query.next()) return m;
    m["id"] = query.value(0);
    m["name"] = query.value(1);
    m["sortname"] = query.value(2);
    return m;
}

QVariantMap Library::track(int id)
{
    query.exec(QString("SELECT id, artist, name, sortname FROM track WHERE id = %1").arg(id));
    QVariantMap m;
    if(!query.next()) return m;
    m["id"] = query.value(0);
    m["artist"] = query.value(1);
    m["name"] = query.value(2);
    m["sortname"] = query.value(3);
    return m;
}

QVariantMap Library::album(int id)
{
    query.exec(QString("SELECT id, artist, name, sortname FROM album WHERE id = %1").arg(id));
    QVariantMap m;
    if(!query.next()) return m;
    m["id"] = query.value(0);
    m["artist"] = query.value(1);
    m["name"] = query.value(2);
    m["sortname"] = query.value(3);
    return m;
}




