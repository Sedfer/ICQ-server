#pragma once

#include "user.h"

class User;

class Room : public QObject
{
    Q_OBJECT
public:
    int id;
    QList<User*> userList;

    Room();
    ~Room();
};
