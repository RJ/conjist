#include <QCoreApplication>
#include <QtCore>
#include <QDebug>
#include <QThread>
#include <QTimer>
#include <stdlib.h>

//#include "servent.h"
//#include "testdriver.h"
//#include "proxyconnection.h"

#include "conjist.h"

int main(int argc, char *argv[])
{
    conjist app(argc, argv);
    return app.exec();
    /*

    QCoreApplication app(argc, argv);

    if(argc!=2) return 1;
    int port = QString(argv[1]).toInt();
    Servent servent(port);

    ControlConnection * cc1 = new ControlConnection(&servent);
    servent.registerOffer("key1", cc1);
    qDebug() << "Registered cc using key1";
    ControlConnection * cc2 = new ControlConnection(&servent);
    servent.registerOffer("key2", cc2);
    qDebug() << "Registered cc using key2";

    ProxyConnection * proxc = new ProxyConnection(QHostAddress("127.0.0.1"), 80, &servent);
    proxc->setOnceOnly(false);
    servent.registerOffer("proxykey", proxc);
    qDebug() << "Registered pc using proxykey";


    TestDriver td(&servent);
    td.start();
    return app.exec();
    */
    //  qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));
}





