#ifndef LIBRARY_H
#define LIBRARY_H


#include <QObject>
#include <QString>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QFile>
#include <QMap>
#include <QPair>
#include <QVariant>
#include <QVariantMap>
#include <QDebug>



class Library : public QObject
{
    Q_OBJECT

public:
    Library(const QString & dbname, QObject *parent = 0);
    ~Library();

    bool removeFile( const QString& url );
    void addDir( const QString& url, int mtime);
    int artistId(const QString& name_orig);
    int trackId(int artistid, const QString& name_orig);
    int albumId(int artistid, const QString& name_orig);
    QList< QPair<int, float> > searchByArtist(int artistid, const QString &table, const QString &name_orig);
    QList< QPair<int, float> > searchTable(const QString &table, const QString &name_orig);
    QList<int> getTrackFids(int tid);
    bool buildIndex(const QString &table);
    QMap<QString,int> ngrams(const QString& str_orig);
    QString sortname(const QString& str);
    QList<QVariantMap> tracks(int artistid);
    QList<QVariant> allTracks();
    QList<QVariantMap> artists();
    QVariantMap artist(int id);
    QVariantMap album(int id);
    QVariantMap track(int id);
    QVariantMap file(int fid);

public slots:
    int addFile( QVariantMap m );

private:
    bool updateSchema(int currentver);
    QSqlDatabase db;
    QSqlQuery query;
};


#endif
