#ifndef JABBERCLIENT_H
#define JABBERCLIENT_H

#include <QDebug>

#include "QXmppClient.h"
#include "QXmppStanza.h"
#include "QXmppPresence.h"
#include "QXmppMessage.h"


class JabberClient : public QXmppClient
{
Q_OBJECT
public:
    explicit JabberClient(QObject *parent = 0);
    ~JabberClient();


signals:
    void peerOnline(QString jid);
    void peerOffline(QString jid);
    void msgReceived(QString from, QString msg);

public slots:
    void messageReceived(const QXmppMessage &);
    void presenceReceived(QXmppPresence);
    void sendMsg(QString to, QString msg);

private:
    QSet<QString> m_peerjids;
};

#endif // JABBERCLIENT_H
