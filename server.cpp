#include "server.h"
#include <iostream>
#include <sstream>

using namespace std;

Server::Server(int port): userID(0), roomID(0)
{
    tcpServer = new QTcpServer();
    tcpServer->listen(QHostAddress::Any, port);
    connect(tcpServer, SIGNAL(newConnection()), SLOT(newConnection()));

    userList = new QList<User*>();
}

Server::~Server()
{
    delete userList;
}

void Server::newConnection()
{
    QTcpSocket *socket = tcpServer->nextPendingConnection();
    connect(socket, SIGNAL(disconnected()), SLOT(disconnected()));
    connect(socket, SIGNAL(readyRead()), SLOT(readyRead()));

    cerr << QDate::currentDate().toString().toStdString() << " "
         << QTime::currentTime().toString().toStdString() << " "
         << "New connection "
         << socket->peerAddress().toString().toStdString() << ":"
         << socket->peerPort() << endl;
}

void Server::readyRead()
{
    QTcpSocket *socket = static_cast<QTcpSocket*>(sender());

    while(true){
        if(!socket || socket->bytesAvailable() <= 0)
            break;

        QByteArray data = socket->readLine(1024);
        respond(QString(data), socket);
    }
}

void Server::disconnected()
{
    QTcpSocket *socket = static_cast<QTcpSocket*>(sender());

    cerr << QDate::currentDate().toString().toStdString() << " "
         << QTime::currentTime().toString().toStdString() << " "
         << "Disconnected "
         << socket->peerAddress().toString().toStdString() << ":"
         << socket->peerPort() << endl;

    disconnect(socket);
}

void Server::respond(const QString &request, QTcpSocket *socket)
{
    cerr << QDate::currentDate().toString().toStdString() << " "
         << QTime::currentTime().toString().toStdString() << " "
         << socket->peerPort() << ":" << request.toStdString() << endl;

    stringstream in(request.toStdString());
    string action;
    in >> action;

    if(!action.compare("login"))
    {
        string name, password;
        in >> name >> password;
        login(QString(name.c_str()), QString(password.c_str()), socket);
        return;
    }

    if(!action.compare("logoff"))
    {
        socket->disconnectFromHost();
        return;
    }

    if(!action.compare("reg"))
    {
        string name, password;
        in >> name >> password;
        registerUser(QString(name.c_str()), QString(password.c_str()));
        return;
    }

    if(!action.compare("ulist"))
    {
        showUserList();
        return;
    }
}

void Server::registerUser(const QString &name, const QString &password)
{
    User *user = new User(userID++, name, password);
    userList->append(user);

    cerr << "User registered: " << user->id << ":"
         << user->name.toStdString() << ":"
         << user->password.toStdString() << endl;
}

void Server::login(const QString &name, const QString &password,
                   QTcpSocket *socket)
{
    bool valid = false;
    User *userToLogin = nullptr;

    for(auto user = userList->begin(); user != userList->end(); ++user)
    {
        // if already logged in
        if(socket == (*user)->socket){
            valid = false;
            break;
        }

        if(name.compare((*user)->name))
            continue;

        if(password.compare((*user)->password)){
            valid = false;
            break;
        }

        if((*user)->socket != nullptr){
            valid = false;
            break;
        }

        // Success
        valid = true;
        userToLogin = *user;
    }

    if(valid)
    {
        userToLogin->socket = socket;

        cerr << "Login: " << userToLogin->id << ":"
             << userToLogin->name.toStdString() << endl;
    }
    else
    {
        cerr << "Login error: " << name.toStdString() << ":"
             << password.toStdString() << endl;
    }
}

void Server::disconnect(QTcpSocket *socket)
{
    if(socket == nullptr)
        return;

    for(auto user = userList->begin(); user != userList->end(); ++user)
    {
        if(socket != (*user)->socket)
            continue;

        disconnect(*user);
        (*user)->socket = nullptr;

        cerr << "Logged off: " << ((*user)->id) << ":"
             << (*user)->name.toStdString() << ":"
             << (*user)->password.toStdString()
             << ":" << (*user)->socket << endl;

        return;
    }
}

void Server::disconnect(User *user)
{
    // delete user from each room
}

void Server::showUserList()
{
    cerr << "User list:\n";
    for(auto user = userList->begin(); user != userList->end(); ++user)
    {
        cerr << "[" << ((*user)->id) << ":"
             << (*user)->name.toStdString() << ":"
             << (*user)->password.toStdString()
             << ":" << (*user)->socket << "]" << endl;
    }
}
