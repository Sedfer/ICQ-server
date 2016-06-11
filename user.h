#pragma once

#include <QtNetwork>
#include "room.h"

class Room;

class User : public QObject
{
    Q_OBJECT
public:
    QString name;
    QString password;
    QList<Room*> roomList;
    QTcpSocket *socket;

    User();
    User(const QString &_name, const QString &_password,
         QTcpSocket *_socket = nullptr);
    ~User();
};
