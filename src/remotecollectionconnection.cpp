#include "remotecollectionconnection.h"


void RemoteCollectionConnection::setup()
{
    m_rc = new RemoteCollection(m_servent, this);

    // send all tracks TODO use a LibraryCollection for this
    // so that in future, we could reproxy easily
    QVariantList all = m_library->allTracks();
    qDebug() << "Sending our library of " << all.length() << " tracks...";

    QVariantMap response;
    response["method"] = "alltracks";
    response["tracks"] = all;

    QJson::Serializer serializer;
    const QByteArray serialized = serializer.serialize( response );
    qDebug() << "Sending full tracklist, size: " << serialized.length();
    sendMsg(serialized);
}
