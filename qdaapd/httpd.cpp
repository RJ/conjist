#include "httpd.h"
#include <QFile>
#include <QByteArray>
#include <QTimer>
// avahi-publish-service QDAAPname _daap._tcp 8080


//using namespace std;

Httpd::Httpd( QDaap::Collection * c, quint16 port) :
    m_coll(c), m_port(port)
{
    // don't start the daap server listening until the remote collection has
    // loaded and we know how many tracks there are etc..
    if(m_coll->isLoaded()) QTimer::singleShot(0, this, SLOT(startListening()));
    else connect(m_coll, SIGNAL(finishedLoading()), this, SLOT(startListening()));
}

Httpd::~Httpd()
{
    qDebug() << "DTOR Httpd";
    mg_stop(m_ctx);
}

void Httpd::startListening()
{
    qDebug() << "DAAP Server listening on port " << m_port;
    m_ctx = mg_start();
    mg_set_option(m_ctx, "ports", QString::number(m_port).toAscii().constData());
    // not as efficient, but lets do all url matching in qt for convenience:
    mg_set_uri_callback(m_ctx, "*", &mongoose_handler, (void*)this);
}

void Httpd::handleRequest(struct mg_connection * conn, const struct mg_request_info * request_info)
{
    QString method(request_info->request_method);
    QString path(request_info->uri);
    QString qs(request_info->query_string);

    qDebug() << method << " " << path << " " << qs ;

    if(method != "GET")
    {
        mg_printf(conn, "%s", "HTTP/1.1 404 Not found\r\n\r\nNot found because we only do GET");
        return;
    }

    QStringList headers;
    headers << "DAAP-Server: QDaap"
            << "Content-Type: application/x-dmap-tagged"
            ;
    QByteArray body;
    // regex when a file is requested
    QRegExp rx("^/databases/1/items/([\\d]+)\.([a-zA-Z0-9]+)$");

    int status; // final http response code
    // handle DAAP protocol urls:

    if     (path == "/server-info")             status = do_server_info(headers, body);
    else if(path == "/login")                   status = do_login(headers, body);
    else if(path == "/logout")                  status = 204;
    else if(path == "/update")                  status = do_update(headers, body);
    // server bug that crashes amarok:
    // else if(path == "/databases")               status = do_databases_items(headers, body);
    else if(path == "/databases")               status = do_databases(headers, body);
    else if(path == "/databases/1/items")       status = do_databases_items(headers, body);
    else if(path == "/databases/1/containers")  status = do_databases_containers(headers, body);
    //else if(path == "/databases/1/containers/PLID/items") status = do_databases_items(headers, body);
    else if(rx.indexIn(path)!=-1)
    {
        int fid = QString(rx.cap(1)).toInt();
        QString ext = rx.cap(2);
        qDebug() << "Serving file id " << fid << " with extension " << ext;

        QIODevice * dev = m_coll->getTrack(fid);
        if(!dev) // || !dev->isOpen())
        {
            mg_printf(conn, "HTTP/1.0 503 Some error\r\n\r\n<H1>503 error</H1>");
        }
        else
        {
            dev->waitForReadyRead(0);
            headers << QString("Content-Length: %1").arg(dev->size());
            mg_printf(conn, "HTTP/1.0 200 OK\r\n");
            foreach(QString h, headers)
            {
                mg_printf(conn, "%s\r\n", h.toAscii().constData());
            }
            mg_printf(conn, "\r\n");
            qint64 i;
            char buf[4096];
            while((i = dev->read((char*)&buf, 4096))>0)
            {
                mg_write(conn, &buf, (int)i);
            }
        }
        return;
    }
    else
    {
        mg_printf(conn, "%s", "HTTP/1.1 404 Not found\r\n\r\nNot found, url unhandled");
        return;
    }
    switch(status)
    {
        case 200:
            mg_printf(conn, "HTTP/1.0 200 OK\r\n");
            headers << QString("Content-Length: %1").arg(body.length());
            foreach(QString h, headers)
            {
                mg_printf(conn, "%s\r\n", h.toAscii().constData());
            }
            mg_printf(conn, "\r\n");
            mg_write(conn, body.constData(), body.length());
            return;

        default:
            mg_printf(conn, "HTTP/1.0 503 Some error\r\n\r\n<H1>503 error</H1>");
            return;
    }

}

QByteArray Httpd::cs(const TagOutput &resp)
{
    Chunk chunk = resp.data();
    return QByteArray((const char*)chunk.begin(), (int)chunk.size());
}

int Httpd::do_server_info(QStringList & headers, QByteArray & body)
{
    TagOutput resp;
    resp    << Tag('msrv')
                << Tag('mstt') << (u32)200 << end
                << Tag('mpro') << Version(2,0) << end  // dmap ver
                << Tag('apro') << Version(3,0) << end  // daap ver
                << Tag('minm') << m_coll->name().toAscii().constData() << end
                << Tag('msau') << (u32)0 << end
                << Tag('mstm') << (u32)1800 << end   // timeout
                << Tag('msex') << (u8)0 << end          // extensions
                << Tag('msix') << (u8)0 << end          // index
                << Tag('msbr') << (u8)0 << end          // browse
                << Tag('msqy') << (u8)0 << end          // query
                << Tag('msup') << (u8)0 << end          // update
                << Tag('msdc') << (u32)1 << end      // num databases
            << end
            ;
    body = cs(resp);
    return 200;
}

int Httpd::do_login(QStringList & headers, QByteArray & body)
{
    TagOutput resp;
    resp    << Tag('mlog')
                << Tag('mstt') << (u32)200 << end
                << Tag('mlid') << (u32)1 << end
            << end
            ;
    body = cs(resp);
    return 200;
}

int Httpd::do_databases(QStringList & headers, QByteArray & body)
{
    TagOutput resp;
    resp    << Tag('avdb')
                << Tag('mstt') << (u32)200 << end
                << Tag('muty') << (u8)0 << end
                << Tag('mtco') << (u32)1 << end // matched items
                << Tag('mrco') << (u32)1 << end // number in msg
                << Tag('mlcl')
                    << Tag('mlit')
                        << Tag('miid') << (u32)1 << end
                        << Tag('mper') << (u64)1 << end
                        << Tag('minm') << m_coll->name().toAscii().constData() << end
                        << Tag('mimc') << (u32)m_coll->numTracks() << end
                        << Tag('mctc') << (u32)m_coll->numPlaylists() << end
                    << end
                << end
            << end
            ;
    body = cs(resp);
    return 200;
}

int Httpd::do_databases_items(QStringList & headers, QByteArray & body)
{
    TagOutput resp;
    resp    << Tag('adbs')
                << Tag('mstt') << (u32)200 << end
                << Tag('muty') << (u8)0 << end
                << Tag('mtco') << (u32)m_coll->numTracks() << end
                << Tag('mrco') << (u32)m_coll->numTracks() << end
                // whole library as a set of mlit's
                << Tag('mlcl');
    foreach(QDaap::Track t, m_coll->tracks())
    {
        resp << Tag('mlit')
                << Tag('mikd') << (u8)2 << end // 2 = music
                << Tag('asdk') << (u8)0 << end // songdatakind
                << Tag('miid') << (u32)t.id << end // itemid
                << Tag('mper') << (u64)t.perid << end // persistentid
                << Tag('minm') << t.track.toAscii().constData() << end
                << Tag('asar') << t.artist.toAscii().constData() << end
                << Tag('asal') << t.album.toAscii().constData() << end
                << Tag('asfm') << t.extension.toAscii().constData() << end
                << Tag('asdb') << (u8)(t.disabled?1:0) << end
                << Tag('astm') << (u32)t.duration << end // duration, in ms
                << Tag('assz') << (u32)t.filesize << end // filesize
                << Tag('asbr') << (u16)t.bitrate << end // bitrate
                << Tag('astc') << (u16)t.trackcount << end
                ;
        if(!t.genre.isEmpty())
            resp<< Tag('asgn') << t.genre.toAscii().constData() << end;
        if(!t.comment.isEmpty())
            resp<< Tag('ascm') << t.comment.toAscii().constData() << end;
        if(t.albumposition)
            resp<< Tag('astn') << (u16)t.albumposition << end;
        if(t.year)
            resp<< Tag('asyr') << (u16)t.year << end;
        if(t.albumdiscnumber)
            resp<< Tag('asdn') << (u16)t.albumdiscnumber << end;

        resp << end; // close mlit
    }
    resp << end // close mlcl
         << end // close adbs
         ;
    body = cs(resp);
    return 200;
}

int Httpd::do_databases_containers(QStringList & headers, QByteArray & body)
{
    TagOutput resp;
    resp    << Tag('apso')
                << Tag('mstt') << (u32)200 << end
                << Tag('muty') << (u8)0 << end
                << Tag('mtco') << (u32)m_coll->numPlaylists() << end //total count
                << Tag('mrco') << (u32)m_coll->numPlaylists() << end // returned count
                << Tag('mlcl')
                    // list all playlists (mlit's):
                    /*
                    << Tag('mlit')
                        << Tag('miid') << (u32)1 << end // dmap.itemid
                        << Tag('mper') << (u64)1 << end // dmap.persistentid
                        << Tag('minm') << (const u8*)"Playlist title lol" << end
                        << Tag('mimc') << (u32)1 << end // num items in playlist
                    << end
                    */
                << end
            << end
            ;
    body = cs(resp);
    return 200;
}

//TODO playlists.
int Httpd::do_databases_containers_items(QStringList & headers, QByteArray & body)
{
    qDebug() << "do_databases_containers_items";
    TagOutput resp;
    resp    << Tag('apso')
                << Tag('mstt') << (u32)200 << end
                << Tag('muty') << (u8)0 << end
                << Tag('mtco') << (u32)0 << end //num
                << Tag('mrco') << (u32)0 << end //num
                // tracks:
                << Tag('mlcl')
                    // a track:
                /*
                    << Tag('mlit')
                        << Tag('mikd') << (u8)2 << end
                        << Tag('miid') << (u32)1 << end // dmap.itemid
                        << Tag('mper') << (u64)1 << end // dmap.persistentid
                        << Tag('minm') << (const u8*)"Track title here lol" << end
                        << Tag('asar') << (u8*)"Artist name here lol" << end
                        << Tag('asal') << (u8*)"Album title here lol" << end
                        << Tag('asfm') << (u8*)"mp3" << end
                        << Tag('asbr') << (u16)128 << end // bitrate
                        << Tag('astm') << (u32)300000 << end // duration, in ms
                        << Tag('ascm') << (u8*)"who looks at song comments?" << end
                    << end
                */
                << end
            << end
            ;
    body = cs(resp);
    return 200;
}

int Httpd::do_update(QStringList & headers, QByteArray & body)
{
    //while(1) dlib::sleep(1000);

    TagOutput resp;
    resp    << Tag('mupd')
                << Tag('mstt') << (u32)200 << end
                << Tag('musr') << (u32)1 << end
            << end
            ;
    body = cs(resp);
    return 200;

    // hang until we have an update (ie, library changes) TODO not implemented!
    //while(1) dlib::sleep(1000);
}






