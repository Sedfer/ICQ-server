#pragma once

#include <QtNetwork>
#include "user.h"
#include "room.h"

class Server : QObject
{
    Q_OBJECT
private:
    QTcpServer *tcpServer;
    QList<User*> *userList;
    QList<Room*> *roomList;
    int userID;
    int roomID;

private slots:
    void newConnection();
    void readyRead();
    void disconnected();

public:
    Server(int port);
    ~Server();

    void respond(const QString &request, QTcpSocket *socket);
    void registerUser(const QString &name, const QString &password);
    void login(const QString &name, const QString &password,
               QTcpSocket *socket);
    void disconnect(QTcpSocket *socket);
    void disconnect(User *user);

    void showUserList();
};
