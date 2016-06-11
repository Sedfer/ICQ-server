#include "user.h"

User::User()
{

}

User::User(int _id, const QString &_name, const QString &_password,
           QTcpSocket *_socket):
    id(_id), name(_name), password(_password), socket(_socket)
{

}

User::~User()
{

}
