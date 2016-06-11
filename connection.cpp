#include "connection.h"

Connection::Connection(QTcpSocket *_socket, User *_user)
    : socket(_socket), user(_user)
{

}

Connection::~Connection()
{

}
