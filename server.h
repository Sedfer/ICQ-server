#pragma once

#include <QtNetwork>
#include "user.h"
#include "room.h"
#include "connection.h"

class Server : public QObject
{
    Q_OBJECT
private:
    QTcpServer *tcpServer;
    QList<User*> *userList;
    QList<Room*> *roomList;
    int roomID;

private slots:
    void newConnection();
    void readyRead();
    void disconnected();

public:
    Server(int port);
    ~Server();

    void respond(const QString &request, Connection *connection);
    void registerUser(const QString &name, const QString &password);
    void login(const QString &name, const QString &password,
               Connection *connection);
    void logoff(Connection *connection);
    void logoff(User *user);

    void showUserList();

    User* findUser(const QString &name);
    bool checkPassword(User *user, const QString &password);
};
