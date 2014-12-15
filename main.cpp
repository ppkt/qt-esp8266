#include "wifi.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Wifi w;
    w.show();

    return a.exec();
}
