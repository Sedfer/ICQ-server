#include "server.h"
#include <iostream>

using namespace std;

Server::Server(int port)
{
    tcpServer = new QTcpServer();
    tcpServer->listen(QHostAddress::Any, port);
    connect(tcpServer, SIGNAL(newConnection()), SLOT(newConnection()));
}

Server::~Server()
{

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
        if(socket->bytesAvailable() <= 0)
            break;

        QByteArray data = socket->readLine(1024);

        cerr << QDate::currentDate().toString().toStdString() << " "
            << QTime::currentTime().toString().toStdString() << " "
            << socket->peerPort() << ":" << data.toStdString() << endl;
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
}
