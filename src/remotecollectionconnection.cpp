#include "remotecollectionconnection.h"


void RemoteCollectionConnection::setup()
{
    m_rc = new RemoteCollection(m_servent, this);
}
