#ifndef CJUUID_H
#define CJUUID_H
#include <QUuid>
inline static QString uuid()
{
    QString q = QUuid::createUuid();
    q.remove(0);
    q.chop(1);
    return q;
}

#endif // CJUUID_H
