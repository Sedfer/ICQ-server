#include "server.h"
#include <iostream>
#include <sstream>

using namespace std;

Server::Server(int port): roomID(0)
{
    tcpServer = new QTcpServer();
    tcpServer->listen(QHostAddress::Any, port);
    connect(tcpServer, SIGNAL(newConnection()), SLOT(newConnection()));

    userList = new QList<User*>();
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

    while(true){
        if(!(connection->socket) || connection->socket->bytesAvailable() <= 0)
            break;

        QByteArray data = connection->socket->read(1024);
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

    if(!action.compare("logoff"))
    {
        connection->socket->disconnectFromHost();
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
    if(!name.compare("") || !password.compare(""))
    {
        cerr << "Registration error: empty name or password" << endl;
        return;
    }

    User *user = findUser(name);
    if(user != nullptr)
    {
        cerr << "User already exists: " << user->name.toStdString() << ":"
             << user->password.toStdString() << endl;
        return;
    }

    user = new User(name, password);
    userList->append(user);

    cerr << "User registered: " << user->name.toStdString() << ":"
         << user->password.toStdString() << endl;
}

void Server::login(const QString &name, const QString &password,
                   Connection *connection)
{
    if(connection->user != nullptr)
    {
        cerr << "Already logged in as: "
             << connection->user->name.toStdString() << endl;
        return;
    }

    User *user = findUser(name);
    if(user == nullptr)
    {
        cerr << "User does not exsist: " << name.toStdString() << endl;
        return;
    }

    if(!checkPassword(user, password))
    {
        cerr << "Incorrect password: " << name.toStdString() << ":"
             << password.toStdString() << endl;
        return;
    }

    if(user->socket != nullptr)
    {
        cerr << "User already logged in: " << name.toStdString() << endl;
        return;
    }

    // Connect socket with user
    user->socket = connection->socket;
    connection->user = user;

    cerr << "Login: " << user->name.toStdString() << endl;
}

void Server::logoff(Connection *connection)
{
    if(connection == nullptr || connection->user == nullptr)
        return;

    logoff(connection->user);
    connection->user->socket = nullptr;

    cerr << "Logged off: " << connection->user->name.toStdString() << endl;
}

// delete user from each room
void Server::logoff(User *user)
{

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

User* Server::findUser(const QString &name)
{
    auto found = find_if(userList->begin(), userList->end(),
                         [&name](User *user){
        return !(user->name.compare(name));
    });

    if(found == userList->end())
        return nullptr;
    else
        return *found;
}

bool Server::checkPassword(User *user, const QString &password)
{
    return !(password.compare(user->password));
}
