#ifndef MUSICSCANNER_H
#define MUSICSCANNER_H

#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <QVariantMap>
#include <QThread>
#include <QDir>
#include <QFileInfo>
#include <QString>


class MusicScanner : public QThread
{
    Q_OBJECT
public:
    MusicScanner(QString dir);

protected:
    void run();

signals:
    void fileScanned(QVariantMap);
    void finished(int,int);

private:
    void scanDir(QDir dir, int depth);;
    void scanFile(const QFileInfo &fi, int depth);

private slots:
    void scan();

private:
    QString m_dir;
    QStringList m_validExts;
    int m_scanned, m_skipped;
};

#endif
