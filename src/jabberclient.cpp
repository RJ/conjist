#include "jabberclient.h"

JabberClient::JabberClient(QObject *parent) :
    QXmppClient(parent)
{
    connect(this, SIGNAL(messageReceived(const QXmppMessage&)), SLOT(messageReceived(const QXmppMessage&)));
    connect(this, SIGNAL(presenceReceived(QXmppPresence)), this, SLOT(presenceReceived(QXmppPresence)) );
}

JabberClient::~JabberClient()
{

}

void JabberClient::presenceReceived(QXmppPresence p)
{
    QString from = p.getFrom();
    switch(p.getStatus().getType())
    {
    case QXmppPresence::Status::Online:
        case QXmppPresence::Status::Away:
        case QXmppPresence::Status::XA:
        case QXmppPresence::Status::DND:
        case QXmppPresence::Status::Chat:
            qDebug() << "ONLINE presence: " << from << " type: " << p.getType();
            if(from.contains("/conjist"))
            {
                if(!m_peerjids.contains(from))
                {
                    m_peerjids.insert(from);
                    qDebug() << "CONJIST peer comes online: " << from;
                    emit peerOnline(p.getFrom());
                }
            }
            break;

    default:
            qDebug() << "OFFLINE pres: " << p.getFrom();
            if(p.getFrom().contains("/conjist"))
            {
                if(m_peerjids.contains(from))
                {
                    qDebug() << "CONJIST peer goes offline: " << p.getFrom();
                    m_peerjids.remove(from);
                    emit peerOffline(p.getFrom());
                }
            }
    }
}

void JabberClient::messageReceived(const QXmppMessage& message)
{
    QString from = message.getFrom();
    QString msg = message.getBody();
    //qDebug() << "Jabber rcvded from: " << from << " msg: " << msg;
    if(m_peerjids.contains(from))
    {
        emit msgReceived(from, msg);
    }
}

void JabberClient::sendMsg(QString to, QString msg)
{
    //qDebug() << "Jabber sending to " << to << " msg: " << msg;
    sendPacket(QXmppMessage("", to, msg));
}
