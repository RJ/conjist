#ifndef TESTDRIVER_H
#define TESTDRIVER_H
#include <QThread>
#include <QTextStream>
#include "servent.h"
#include "controlconnection.h"



class TestDriver : public QThread
{
    Q_OBJECT
public:
    TestDriver(Servent * s) : servent(s)
    {
        moveToThread(this);
    };

    void run()
    {
        QTimer::singleShot(0, this, SLOT(loop()));
        exec();
    };

    public slots:
    void loop()
    {
        connect(this,    SIGNAL(readLine(QString)),
                servent, SLOT(debug_handleLine(QString)),
                Qt::QueuedConnection);
        QTextStream stream(stdin);
        QString line;
        do {
            printf("> ");
            line = stream.readLine();
            emit readLine(line);

        } while (line != "quit");
    };
signals:
    void readLine(QString);

    private:
        Servent * servent;
};



#endif // TESTDRIVER_H
