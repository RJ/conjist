#include <QtCore/QCoreApplication>
#include "httpd.h"
#include "collection.h"



int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QDaap::Collection coll("Default fake");
    Httpd * server = new Httpd(&coll, 8080);
    printf("Starting on 8080\n");
    return a.exec();
}
