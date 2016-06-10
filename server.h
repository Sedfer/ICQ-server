#pragma once

#include <QtNetwork/QtNetwork>

class Server : QObject
{
    Q_OBJECT
private:
    QTcpServer *tcpServer;

private slots:
    void newConnection();
    void readyRead();
    void disconnected();

public:
    Server(int port);
    ~Server();
};
