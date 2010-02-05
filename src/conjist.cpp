#include <QTime>
#include "conjist.h"
#include "portfwd.h"
#include "getopt_helper.h"
#include "QXmppLogger.h"
#include "QXmppConfiguration.h"

// default port, can be set using --listenport on command line
#define CONJISTPORT 50210

conjist::conjist(int argc, char *argv[])
    : QCoreApplication(argc, argv), m_go(argc,argv), m_servent(0), m_externalPort(0), m_port(CONJISTPORT)
{
    qsrand(QTime(0,0,0).secsTo(QTime::currentTime())); // for uuid generator
    // do the setup once event loop started, so we can call exit() correctly
    QTimer::singleShot(0, this, SLOT(setup()));
}

void conjist::setup()
{
    QString domain, username, password, server, sport, slport;
    int jport;
    bool noupnp, xmpplog;
    m_go.addOption('d', "domain", &domain);
    m_go.addOption('u', "user", &username);
    m_go.addOption('p', "pass", &password);
    m_go.addOption('s', "server", &server);
    m_go.addOption('r', "jabberport", &sport);
    m_go.addOption('l', "listenport", &slport);
    m_go.addSwitch("noupnp", &noupnp);
    m_go.addSwitch("xmpplog", &xmpplog);

    bool ok = m_go.parse();
    if(!ok || username=="" || password=="" || domain=="")
    {
        printf("You must supply your jabber details.\n");
        printf("For example, if you were larry@gmail.com using gtalk:\n");
        printf("Usage: conjist.exe --user larry --domain gmail.com --pass 123\n");
        printf("                   --server talk.google.com --port 5222\n");
        printf("\n"
               "Use --noupnp to disable UPnP detection.\n");
        this->exit(1);
        return;
    }
    jport = sport.toInt();
    if(!jport) jport=5222;

    m_port = slport.toInt();
    if(!m_port) m_port = CONJISTPORT;

    if(server.isNull()) server = domain;

    // jabber client setup

    if(xmpplog) QXmppLogger::getLogger()->setLoggingType(QXmppLogger::STDOUT);
    else        QXmppLogger::getLogger()->setLoggingType(QXmppLogger::NONE);

    m_jabber = new JabberClient(this);
    connect(m_jabber, SIGNAL(peerOnline(QString)), this, SLOT(newJabberPeer(QString)));
    connect(m_jabber, SIGNAL(msgReceived(QString,QString)), this, SLOT(jabberMsg(QString,QString)));

    m_ourjid = QString("%1@%2").arg(username).arg(domain);
    QXmppConfiguration conf;
    conf.setHost(server);
    conf.setDomain(domain);
    conf.setUser(username);
    conf.setPasswd(password);
    conf.setPort(jport);
    conf.setResource("conjist");
    conf.setStatus("conjist software (not human)");

    m_jabber->connectToServer( conf );

    startServent(!noupnp);
}


// figure out ports, external ips, and start the servent listening
void conjist::startServent(bool upnp)
{    
    QString pubip;
    Portfwd pf;
    if(!upnp) goto nopublic;

    qDebug() << "Searching for UPnp Router...";
    if(!pf.init(2000))
    {
        qDebug() << "!! Couldn't find gateway.";
        goto nopublic;
    }
    if(!pf.add(m_port))
    {
        qDebug() << "!! Couldn't setup external port fwd for port " << CONJISTPORT;
        goto nopublic;
    }

    pubip = QString(pf.external_ip().c_str());
    m_externalAddress = QHostAddress(pubip);
    m_externalPort = m_port;
    qDebug() << "External servent address detected as " << pubip << ":" << m_externalPort;
    qDebug() << "Max upstream   " << pf.max_upstream_bps() << "bps";
    qDebug() << "Max downstream " << pf.max_downstream_bps() << "bps";

    //printf("External IP: %s\n", pf.external_ip().c_str());
    //printf("LAN IP: %s\n", pf.lan_ip().c_str());
    //printf("Max upstream: %d bps, max downstream: %d bps\n",
    //       pf.max_upstream_bps(), pf.max_downstream_bps() );

    if(false)
    {
nopublic:
        qDebug() << "!! No external IP address configured / detected";
        qDebug() << "!! You will be able to make outbound connections, but not listen for incoming";
    }

    m_servent = new Servent(QHostAddress(QHostAddress::Any), m_port, this);
    if(m_externalPort) m_servent->setExternalAddress(m_externalAddress, m_externalPort);


}

void conjist::newJabberPeer(QString name)
{
    if(name.startsWith(m_ourjid)) return;

    QVariantMap m;
    if(m_servent->visibleExternally())
    {
        QString key = uuid();
        ControlConnection * cc = new ControlConnection(m_servent);
        cc->setName(name);
        m_servent->registerOffer(key, cc);

        m["visible"] = true;
        m["ip"] = m_servent->externalAddress().toString();
        m["port"] = m_servent->externalPort();
        m["key"] = key;

        qDebug() << "Extending an offer to " << name << " using key: " << key;
    }
    else
    {
        m["visible"] = false;
        qDebug() << "Telling " << name << " we are not externally visible. Hope they are.";
    }

    QJson::Serializer serializer;
    const QByteArray ser = serializer.serialize( m );
    m_jabber->sendMsg(name, QString::fromAscii(ser));
}

void conjist::jabberMsg(QString from, QString msg)
{
    qDebug() << "JMSG from " << from << " MSG: " << msg;
    bool ok;
    QVariantMap m = parser.parse(msg.toAscii(), &ok).toMap();
    if(!ok)
    {
        qDebug() << "Malformed msg in jabber msg";
        return;
    }
    bool visible = m.value("visible").toBool();
    if(!visible)
    {
        if(m_servent->visibleExternally())
            qDebug() << from << " is not externally visible, they should to connect to us.";
        else
            qDebug() << from << " is no externally visible, neither are we, no direct connection possible :(";
    }
    else
    {
        QString ip = m.value("ip").toString();
        int port = m.value("port").toInt();
        QString key = m.value("key").toString();
        // if we are also externally visible, decide who will initate the connection
        // based on the IP addresses (arbitrary but deterministic)
        if(m_servent->visibleExternally() && m_servent->externalAddress().toString() < ip)
        {
            qDebug() << "both externally visible, but decided to let " << from << " initiate the connection.";
            return;
        }
        qDebug() << "We will initiate a connection to " << from;
        ControlConnection * cc = new ControlConnection(m_servent);
        cc->setName(from);
        m_servent->connectToPeer(QHostAddress(ip), port, cc, key);
    }
}
