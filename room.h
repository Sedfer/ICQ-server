#pragma once

#include "user.h"

class Room : QObject
{
    Q_OBJECT
public:
    int id;
    QList<User*> userList;

    Room();
    ~Room();
};
