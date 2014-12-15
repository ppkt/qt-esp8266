#ifndef ACCESSPOINT_H
#define ACCESSPOINT_H
#include <QString>

enum Encryption {
    OPEN = 0,
    WEP,
    WPA_PSK,
    WPA_PSK2,
    WPA_WPA2_PSK
};

struct AccessPoint
{
    Encryption encryption;
    QString essid;
    int signal;
    QString mac;
    int channel;
};

#endif // ACCESSPOINT_H
