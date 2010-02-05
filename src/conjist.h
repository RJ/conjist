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
    void startServent();

    QString m_ourjid;
    JabberClient * m_jabber;
    Servent * m_servent;
    int m_publicPort;
    QHostAddress m_publicAddress;
    QJson::Parser parser;
};

#endif // CONJIST_H
