#include "remotecollection.h"
#include "httpd.h"

RemoteCollection::RemoteCollection(Servent * s, RemoteCollectionConnection * conn) :
        QDaap::Collection(conn->name(), conn), m_servent(s), m_rcconn(conn)
{
    qDebug() << "CTOR RemoteCollection, listening on 8080";
    Httpd * h = new Httpd(this, 8080);
}
