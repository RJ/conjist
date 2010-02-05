#include "conjist.h"
#include "portfwd.h"
#include "getopt_helper.h"

#define CONJISTPORT 50210

conjist::conjist(int argc, char *argv[])
    : QCoreApplication(argc, argv), m_servent(0), m_publicPort(0)
{
    QTimer::singleShot(0, this, SLOT(setup()));
}

void conjist::setup()
{
    helper::GetOpt go;
    QString domain, username, password, server, sport;
    int port;
    go.addOption('d', "domain", &domain);
    go.addOption('u', "user", &username);
    go.addOption('p', "pass", &password);
    go.addOption('s', "server", &server);
    go.addOption('r', "port", &sport);

    if(!go.parse())
    {
        printf("You must supply your jabber details.\n");
        printf("For example, if you were larry@gmail.com using gtalk:\n");
        printf("Usage: conjist.exe --user larry --domain gmail.com --pass 123\n");
        printf("                   --server talk.google.com --port 5222\n");
        this->exit(1);
        return;
    }
    port = sport.toInt();

    m_jabber = new JabberClient(this);
    connect(m_jabber, SIGNAL(peerOnline(QString)), this, SLOT(newJabberPeer(QString)));

    m_ourjid = "playdartest2@jabber.org";
    m_jabber->connectToServer(domain, username, password, server, port);

    startServent();
}

// figure out ports, external ips, and start the servent listening
void conjist::startServent()
{
    qDebug() << "Searching for UPnp Router...";
    QString pubip;
    Portfwd pf;
    if(!pf.init(2000))
    {
        qDebug() << "!! Couldn't find gateway.";
        goto nopublic;
    }
    if(!pf.add(CONJISTPORT))
    {
        qDebug() << "!! Couldn't setup external port fwd for port " << CONJISTPORT;
        goto nopublic;
    }

    pubip = QString(pf.external_ip().c_str());
    m_publicAddress = QHostAddress(pubip);
    m_publicPort = CONJISTPORT;
    qDebug() << "Public servent address detected as " << pubip << ":" << m_publicPort;
    qDebug() << "Max upstream   " << pf.max_upstream_bps() << "bps";
    qDebug() << "Max downstream " << pf.max_downstream_bps() << "bps";

    //printf("External IP: %s\n", pf.external_ip().c_str());
    //printf("LAN IP: %s\n", pf.lan_ip().c_str());
    //printf("Max upstream: %d bps, max downstream: %d bps\n",
    //       pf.max_upstream_bps(), pf.max_downstream_bps() );

    if(false)
    {
nopublic:
        qDebug() << "!! You will be able to make outbound connections, but not listen for incoming";
    }

    m_servent = new Servent(QHostAddress(QHostAddress::Any), CONJISTPORT, this);
    if(m_publicPort) m_servent->setExternalAddress(m_publicAddress, m_publicPort);


}

void conjist::newJabberPeer(QString name)
{
    if(name.startsWith(m_ourjid)) return;
    qDebug() << "New jabber peer: " << name;
}

void conjist::jabberMsg(QString from, QString msg)
{
    bool ok;
    QVariantMap m = parser.parse(msg.toAscii(), &ok).toMap();
    if(!ok)
    {
        qDebug() << "Malformed msg in jabber msg";
        return;
    }

}
