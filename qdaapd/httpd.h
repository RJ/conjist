#ifndef HTTPD_H
#define HTTPD_H

#include <QDebug>
#include <QString>
#include <QRegExp>
#include <QStringList>
#include <QThread>
#include <QTimer>

#ifdef _WIN32
#include <winsock.h>
#define	snprintf			_snprintf

#ifndef _WIN32_WCE
#ifdef _MSC_VER /* pragmas not valid on MinGW */
#endif /* _MSC_VER */

#else /* _WIN32_WCE */
/* Windows CE-specific definitions */
#pragma comment(lib,"ws2")
//#include "compat_wince.h"
#endif /* _WIN32_WCE */

#else
#include <sys/types.h>
#include <sys/select.h>
#include <sys/wait.h>
#endif

#ifndef _WIN32_WCE /* Some ANSI #includes are not available on Windows CE */
#include <time.h>
#include <errno.h>
#include <signal.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "mongoose.h"


#include "collection.h"

#include "daap/basic.h"
#include "daap/tagoutput.h"
#include "daap/taginput.h"
#include "daap/registry.h"

using namespace std;

typedef QMap<QString,QString> ssmap;

class Httpd :  public QObject
{
    Q_OBJECT
public:
    Httpd(QDaap::Collection * c, quint16 port = 8080);
    ~Httpd();

    void handleRequest(struct mg_connection * conn, const struct mg_request_info * request_info);

private slots:
    void startListening();

private:
    int do_server_info(QStringList & headers, QByteArray & body);
    int do_login(QStringList & headers, QByteArray & body);
    int do_databases(QStringList & headers, QByteArray & body);
    int do_databases_items(QStringList & headers, QByteArray & body);
    int do_databases_containers(QStringList & headers, QByteArray & body);
    int do_databases_containers_items(QStringList & headers, QByteArray & body);
    int do_update(QStringList & headers, QByteArray & body);

    QByteArray cs(const TagOutput &);

    QDaap::Collection * m_coll;
    struct mg_context * m_ctx;

    int m_port;

};

static void mongoose_handler(struct mg_connection * conn,
                             const struct mg_request_info * request_info,
                             void *user_data)
{
    Httpd * httpd = (Httpd*)user_data;
    httpd->handleRequest(conn, request_info);
}



class QDaapdThread : public QThread
{
    Q_OBJECT
public:

    QDaapdThread(QDaap::Collection *c, quint16 p)
        : m_c(c), m_p(p)
    {
        moveToThread(this);
    };

    void run()
    {
        QTimer::singleShot(0, this, SLOT(startServer()));
        exec();
    };

private slots:
    void startServer()
    {
        m_h = new Httpd(m_c, m_p);
    };

private:
    QDaap::Collection * m_c;
    quint16 m_p;
    Httpd * m_h;
};



#endif // HTTPD_H
