#include "user.h"

User::User()
{

}

User::User(const QString &_name, const QString &_password,
           QTcpSocket *_socket)
    : name(_name), password(_password), socket(_socket),
      roomList()
{

}

User::~User()
{

}
