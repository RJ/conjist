#ifndef CONJIST_H
#define CONJIST_H
#include <QCoreApplication>
#include <QVariantMap>
#include <QHostAddress>

#include <qjson/parser.h>
#include <qjson/serializer.h>
#include <qjson/qobjecthelper.h>

#include "jabberclient.h"
#include "servent.h"
#include "getopt_helper.h"
#include "controlconnection.h"

class conjist : public QCoreApplication
{
Q_OBJECT
public:
    explicit conjist(int argc, char *argv[]);

signals:

public slots:
    void newJabberPeer(QString);
    void jabberMsg(QString from, QString msg);

private slots:
    void setup();

private:
    void startServent(bool upnp);
    helper::GetOpt m_go;
    QString m_ourjid;
    JabberClient * m_jabber;
    Servent * m_servent;
    int m_externalPort, m_port;
    QHostAddress m_externalAddress;
    QJson::Parser parser;

};

#endif // CONJIST_H
