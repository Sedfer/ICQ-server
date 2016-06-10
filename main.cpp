#include "server.h"
#include <QtCore>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    Server server(2424);

    return a.exec();
}
