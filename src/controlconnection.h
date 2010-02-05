#ifndef CONTROLCONNECTION_H
#define CONTROLCONNECTION_H
#include "connection.h"
#include "servent.h"

class ControlConnection : public Connection
{
Q_OBJECT
public:
    explicit ControlConnection(Servent *parent = 0);
    ~ControlConnection();
    Connection * clone();


    QString id() const
    {
        return QString("ControlConnection(%1)").arg(m_name);
    }

protected:

    virtual void handleMsg(QByteArray msg);
    virtual void setup();

signals:

public slots:

};

#endif // CONTROLCONNECTION_H
