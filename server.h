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

    // Signals to server
    void registerUser(const QString &name, const QString &password,
                      Connection *connection);
    void login(const QString &name, const QString &password,
               Connection *connection);
    void logoff(Connection *connection);
    void logoff(User *user);

    void addRoom(Connection *connection);
    void join(Connection *connection, int id);
    void leave(Connection *connection, int id);
    void invite(Connection *connection, int id, const QString &name);

    void addUser(User *user, Room *room, Connection *connection);
    void removeUser(User *user, Room *room);

    void send(Connection *connection, int id);

    void showUserList();

    // Signals to client
    void sendData(QTcpSocket *socket, const QByteArray &data);
    void sendOK(QTcpSocket *socket);
    void sendError(QTcpSocket *socket, int code = 0);
    void sendAddRoom(QTcpSocket *socket, Room *room);
    void sendAddUser(QTcpSocket *socket, Room *room, User *user);
    void sendRemoveRoom(QTcpSocket *socket, Room *room);
    void sendRemoveUser(QTcpSocket *socket, Room *room, User *user);
    void sendText(const QString &name, QTcpSocket *socket, Room *room, const QString &text,
                  User *from = nullptr);

    User* findUser(const QString &name, Room *room = nullptr);
    Room* findRoom(int id, User *user = nullptr);
    bool checkPassword(User *user, const QString &password);
};
