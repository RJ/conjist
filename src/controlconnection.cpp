#include "controlconnection.h"
#include "proxylistener.h"

ControlConnection::ControlConnection(Servent *parent) :
    Connection(parent)
{
    qDebug() << "CTOR controlconnection";
}
ControlConnection::~ControlConnection()
{
    qDebug() << "DTOR controlconnection";
    m_servent->unregisterControlConnection(this);
}

Connection * ControlConnection::clone()
{
    ControlConnection * clone = new ControlConnection(servent());
    clone->setOnceOnly(onceOnly());
    clone->setName(name());
    return clone;
}

void ControlConnection::setup()
{
    m_servent->registerControlConnection(this);
}

void ControlConnection::handleMsg(QByteArray msg)
{
    qDebug() << id() << " got msg: " << QString::fromAscii(msg);

    bool ok;
    QVariantMap m = parser.parse(msg, &ok).toMap();
    if(ok)
    {
        if(m.value("conntype").toString() == "request-offer")
        {
            QString theirkey = m["key"].toString();
            QString ourkey   = m["offer"].toString();
            servent()->reverseOfferRequest(this, ourkey, theirkey);
        }
        else if(m.value("method").toString() == "daap-offer")
        {
            QString key = m["key"].toString();
            QString name= m["name"].toString();
            m_servent->createDaapListener(this, key, name);
        }
        else if(m.value("method").toString() == "library-offer")
        {
            QString key = m["key"].toString();
            QString name= m["name"].toString();
            m_servent->createRemoteCollection(this, key, name);
        }

        return;
    }
    qDebug() << id() << " Unhandled msg: " << QString::fromAscii(msg);
}

