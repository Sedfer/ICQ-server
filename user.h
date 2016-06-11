#pragma once

#include <QtNetwork>

class User : QObject
{
    Q_OBJECT
public:
    int id;
    QString name;
    QString password;
    QTcpSocket *socket;

    User();
    User(int _id, const QString &_name, const QString &_password,
         QTcpSocket *_socket = nullptr);
    ~User();
};
