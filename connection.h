#pragma once

#include <QtNetwork>
#include "user.h"

class Connection : public QObject
{
    Q_OBJECT
public:
    QTcpSocket *socket;
    User *user;

signals:
    void disconnected();
    void readyRead();

public:
    Connection(QTcpSocket *_socket, User *_user = nullptr);
    ~Connection();
};
