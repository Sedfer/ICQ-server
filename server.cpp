#include "server.h"
#include <iostream>
#include <sstream>
#include <ctime>

using namespace std;

Server::Server(int port): roomID(0)
{
    tcpServer = new QTcpServer();
    tcpServer->listen(QHostAddress::Any, port);
    connect(tcpServer, SIGNAL(newConnection()), SLOT(newConnection()));


    userList = new QList<User*>();
    roomList = new QList<Room*>();
}

Server::~Server()
{
    delete userList;
    delete roomList;
}

void Server::newConnection()
{
    QTcpSocket *socket = tcpServer->nextPendingConnection();
    Connection *connection = new Connection(socket);

    connect(socket, SIGNAL(disconnected()), connection, SIGNAL(disconnected()));
    connect(connection, SIGNAL(disconnected()), SLOT(disconnected()));

    connect(socket, SIGNAL(readyRead()), connection, SIGNAL(readyRead()));
    connect(connection, SIGNAL(readyRead()), SLOT(readyRead()));

    cerr << QDate::currentDate().toString().toStdString() << " "
         << QTime::currentTime().toString().toStdString() << " "
         << "New connection "
         << socket->peerAddress().toString().toStdString() << ":"
         << socket->peerPort() << endl;
}

void Server::readyRead()
{
    Connection *connection = static_cast<Connection*>(sender());

    while(true)
    {
        if(!(connection->socket) || connection->socket->bytesAvailable() <= 0)
            break;

        QByteArray data = connection->socket->readLine(1024);
        respond(QString(data), connection);
    }
}

void Server::disconnected()
{
    Connection *connection = static_cast<Connection*>(sender());

    cerr << QDate::currentDate().toString().toStdString() << " "
         << QTime::currentTime().toString().toStdString() << " "
         << "Disconnected "
         << connection->socket->peerAddress().toString().toStdString() << ":"
         << connection->socket->peerPort() << endl;

    logoff(connection);
}

void Server::respond(const QString &request, Connection *connection)
{
    cerr << QDate::currentDate().toString().toStdString() << " "
         << QTime::currentTime().toString().toStdString() << " "
         << connection->socket->peerPort() << ":" << request.toStdString() << endl;

    stringstream in(request.toStdString());
    string action;
    in >> action;

    if(!action.compare("login"))
    {
        string name, password;
        in >> name >> password;
        login(QString(name.c_str()), QString(password.c_str()), connection);
        return;
    }

    if(!action.compare("abort"))
    {
        connection->socket->disconnectFromHost();
        return;
    }

    if(!action.compare("reg"))
    {
        string name, password;
        in >> name >> password;
        registerUser(QString(name.c_str()), QString(password.c_str()),
                     connection);
        return;
    }

    if(!action.compare("ulist"))
    {
        showUserList();
        return;
    }

    if(connection->user == nullptr)
    {
        sendError(connection->socket, 1);
        cerr << "Error: connection has to be logged in" << endl;
        return;
    }

    if(!action.compare("send"))
    {
        int id;
        in >> id;
        send(connection, id);
        return;
    }

    if(!action.compare("logoff"))
    {
        logoff(connection);
        return;
    }

    if(!action.compare("addroom"))
    {
        addRoom(connection);
        return;
    }

    if(!action.compare("leave"))
    {
        int id = -1;
        in >> id;
        leave(connection, id);
        return;
    }

    if(!action.compare("invite"))
    {
        int id = -1;
        string name;
        in >> id >> name;
        invite(connection, id, QString(name.c_str()));
        return;
    }

    if(!action.compare("join"))
    {
        int id = -1;
        in >> id;
        join(connection, id);
        return;
    }

    sendError(connection->socket, 2);
    cerr << "Error: command unrecognized: " << action << endl;
}

void Server::registerUser(const QString &name, const QString &password,
                          Connection *connection)
{
    if(connection == nullptr)
    {
        cerr << "Error: can not register" << endl;
        return;
    }

    if(!name.compare("") || !password.compare(""))
    {
        sendError(connection->socket, 8);
        cerr << "Registration error: empty name or password" << endl;
        return;
    }

    User *user = findUser(name);
    if(user != nullptr)
    {
        sendError(connection->socket, 9);
        cerr << "User already exists: " << user->name.toStdString() << ":"
             << user->password.toStdString() << endl;
        return;
    }

    user = new User(name, password);
    userList->append(user);

    sendOK(connection->socket);
    cerr << "User registered: " << user->name.toStdString() << ":"
         << user->password.toStdString() << endl;
}

void Server::login(const QString &name, const QString &password,
                   Connection *connection)
{
    if(connection == nullptr)
    {
        cerr << "Error: can not login" << endl;
        return;
    }

    if(connection->user != nullptr)
    {
        sendError(connection->socket, 7);
        cerr << "Already logged in as: "
             << connection->user->name.toStdString() << endl;
        return;
    }

    User *user = findUser(name);
    if(user == nullptr)
    {
        sendError(connection->socket, 4);
        cerr << "User does not exsist: " << name.toStdString() << endl;
        return;
    }

    if(!checkPassword(user, password))
    {
        sendError(connection->socket, 5);
        cerr << "Incorrect password: " << name.toStdString() << ":"
             << password.toStdString() << endl;
        return;
    }

    if(user->socket != nullptr)
    {
        sendError(connection->socket, 6);
        cerr << "User already logged in: " << name.toStdString() << endl;
        return;
    }

    // Connect socket with user
    user->socket = connection->socket;
    connection->user = user;

    sendOK(connection->socket);
    cerr << "Login: " << user->name.toStdString() << endl;
}

void Server::logoff(Connection *connection)
{
    if(connection == nullptr || connection->user == nullptr)
    {
        cerr << "Error: can not logoff connection" << endl;
        return;
    }

    sendOK(connection->socket);
    cerr << "Logged off: " << connection->user->name.toStdString() << endl;

    logoff(connection->user);
    connection->user->socket = nullptr;
    connection->user = nullptr;
}

void Server::logoff(User *user)
{
    if(user == nullptr)
    {
        cerr << "Error: can not logoff user" << endl;
        return;
    }

    for(auto room = user->roomList.begin(); room != user->roomList.end(); ++room)
    {
        removeUser(user, *room);
    }
}

void Server::addRoom(Connection *connection)
{
    if(connection == nullptr || connection->user == nullptr)
    {
        cerr << "Error: can not add room" << endl;
        return;
    }

    bool uniqueRoom = false;
    int randomRoomID;
    while(!uniqueRoom)
    {
        uniqueRoom = true;
        srand( time( 0 ) );
        randomRoomID = 1 + rand() % 1000;
        for (int i = 0; i < roomList->size(); ++i)
             if(roomList->at(i)->id == randomRoomID)
                 uniqueRoom = false;
    }

    Room *room = new Room(randomRoomID);
    roomList->append(room);

    sendOK(connection->socket);
    cerr << "Room created: " << room->id << endl;

    addUser(connection->user, room, connection);
}

void Server::join(Connection *connection, int id)
{
    if(connection == nullptr || connection->user == nullptr)
    {
        cerr << "Error: can not join" << endl;
        return;
    }

    Room *room = findRoom(id);
    if(room == nullptr)
    {
        sendError(connection->socket, 10);
        cerr << "Error: room not found: " << id << endl;
        return;
    }

    addUser(connection->user, room, connection);
}

void Server::leave(Connection *connection, int id)
{
    if(connection == nullptr || connection->user == nullptr)
    {
        cerr << "Error: can not leave room" << endl;
        return;
    }

    Room *room = findRoom(id, connection->user);
    if(room == nullptr)
    {
        sendError(connection->socket, 10);
        cerr << "Room not found: " << id << endl;
        return;
    }

    removeUser(connection->user, room);
}

void Server::invite(Connection *connection, int id, const QString &name)
{
    if(connection == nullptr || connection->user == nullptr)
    {
        cerr << "Error: can not invite" << endl;
        return;
    }

    User *user = findUser(name);
    if(user == nullptr)
    {
        sendError(connection->socket, 11);
        cerr << "Error: user not found: " << name.toStdString() << endl;
        return;
    }
    if(user->socket == nullptr)
    {
        sendError(connection->socket, 12);
        cerr << "Error: user offline: " << name.toStdString() << endl;
        return;
    }

    Room *room = findRoom(id, connection->user);
    if(room == nullptr)
    {
        sendError(connection->socket, 10);
        cerr << "Error: room not found: " << id << endl;
        return;
    }

    addUser(user, room, connection);
}

void Server::addUser(User *user, Room *room, Connection *connection)
{
    if(user == nullptr || room == nullptr)
    {
        cerr << "Error: can not add user" << endl;
        return;
    }

    if(find(room->userList.begin(), room->userList.end(), user) !=
       room->userList.end())
    {
        sendError(connection->socket, 13);
        cerr << "Error: user " << user->name.toStdString() << " already in room "
             << room->id << endl;
        return;
    }

    user->roomList.append(room);

    sendOK(connection->socket);
    sendAddRoom(user->socket, room);

    //send info about all users in room
    for(auto i = room->userList.begin(); i != room->userList.end(); ++i)
    {
        sendAddUser(user->socket, room, *i);
    }

    room->userList.append(user);

    // Send to all users in that room
    for(auto i = room->userList.begin(); i != room->userList.end(); ++i)
    {
        sendAddUser((*i)->socket, room, user);
    }
}

void Server::removeUser(User *user, Room *room)
{
    if(user == nullptr || room == nullptr)
    {
        cerr << "Error: can not remove user" << endl;
        return;
    }

    if(find(room->userList.begin(), room->userList.end(), user) ==
       room->userList.end())
    {
        cerr << "Error: user not found: " << user->name.toStdString() << endl;
        return;
    }

    room->userList.removeOne(user);
    user->roomList.removeOne(room);

    sendRemoveRoom(user->socket, room);

    if(room->userList.size() == 0)
    {
        cerr << "Empty room deleted: " << room->id << endl;

        roomList->removeOne(room);
        delete room;
        return;
    }

    for(auto i = room->userList.begin(); i != room->userList.end(); ++i)
    {
        sendRemoveUser((*i)->socket, room, user);
    }
}

void Server::send(Connection *connection, int id)
{
    if(connection == nullptr || connection->user == nullptr)
    {
        cerr << "Error: can not recieve message" << endl;
        return;
    }

    QString text;
    while(connection->socket->bytesAvailable() > 0)
    {
        QByteArray line = connection->socket->readLine(1024);
        // '\xFF\n' means end of message
        if(line.at(0) == '\xFF')
        {
            break;
        }
        text += "<br>" + QString(line);
    }

    Room *room = findRoom(id, connection->user);
    if(room == nullptr)
    {
        sendError(connection->socket, 10);
        cerr << "Error: room not found: " << id << endl;
        return;
    }

    sendOK(connection->socket);

    for(auto i = room->userList.begin(); i != room->userList.end(); ++i)
    {
        sendText((*i)->socket, room, text, connection->user, *i);
    }
}

void Server::showUserList()
{
    cerr << "User list:\n";
    for(auto user = userList->begin(); user != userList->end(); ++user)
    {
        cerr << "[" << (*user)->name.toStdString() << ":"
             << (*user)->password.toStdString()
             << ":" << (*user)->socket << "]" << endl;
    }
}

void Server::sendData(QTcpSocket *socket, const QByteArray &data)
{
    if(socket != nullptr && socket->isWritable())
    {
        socket->write(data);
    }
    else
    {
        cerr << "Error: can not write data: " << data.toStdString() << endl;
    }
}

void Server::sendOK(QTcpSocket *socket)
{
    string str = "ok\n";
    sendData(socket, QByteArray(str.c_str()));
}

void Server::sendError(QTcpSocket *socket, int code)
{
    string str = "error " + to_string(code) + "\n";
    sendData(socket, QByteArray(str.c_str()));
}

void Server::sendAddRoom(QTcpSocket *socket, Room *room)
{
    string str = "addroom " + to_string(room->id) + "\n";
    sendData(socket, QByteArray(str.c_str()));
}

void Server::sendAddUser(QTcpSocket *socket, Room *room, User *user)
{
    string str = "adduser " + to_string(room->id) + " " +
            user->name.toStdString() + "\n";
    sendData(socket, QByteArray(str.c_str()));
}

void Server::sendRemoveRoom(QTcpSocket *socket, Room *room)
{
    string str = "delroom " + to_string(room->id) + "\n";
    sendData(socket, QByteArray(str.c_str()));
}

void Server::sendRemoveUser(QTcpSocket *socket, Room *room, User *user)
{
    string str = "deluser " + to_string(room->id) + " " +
            user->name.toStdString() + "\n";
    sendData(socket, QByteArray(str.c_str()));
}

void Server::sendText(QTcpSocket *socket, Room *room,const QString &text,
                      User *from, User *to)
{
    string nameAndTime = from->name.toStdString() + " [" +
            QTime::currentTime().toString().toStdString() + "]:";

    if(from == to)
        nameAndTime = colorText(nameAndTime, "green");

    else
        nameAndTime = colorText(nameAndTime, "blue");

    string str = "send " + to_string(room->id) + "\n" +
            nameAndTime + "\n" + text.toStdString() + "\xFF\n";

    sendData(socket, QByteArray(str.c_str()));
}

string Server::colorText(const string &text, const string &color)
{
    string coloredText = "<b><font color=\"" + color + "\">"
                        + text + "</font></b>";
    return coloredText;
}

User* Server::findUser(const QString &name, Room *room)
{
    QList<User*> *list = userList;
    if(room != nullptr)
    {
        list = &(room->userList);
    }

    auto found = find_if(list->begin(), list->end(),
                         [&name](User *user){
        return !(user->name.compare(name));
    });

    if(found == list->end())
        return nullptr;
    else
        return *found;
}

Room *Server::findRoom(int id, User *user)
{
    QList<Room*> *list = roomList;
    if(user != nullptr)
    {
        list = &(user->roomList);
    }

    auto room = find_if(list->begin(), list->end(),
                        [id](Room *room){
        return room->id == id;
    });

    if(room == list->end())
        return nullptr;
    else
        return *room;
}

bool Server::checkPassword(User *user, const QString &password)
{
    return !(password.compare(user->password));
}
