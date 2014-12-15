#ifndef WIFI_H
#define WIFI_H

#include <QMainWindow>
#include <QtSerialPort/QSerialPort>


namespace Ui {
    class Wifi;
}

enum Operation {
    AT_CWLAP, // Get AP list
    AT_CIFSR, // Get IP address
    AT_CWJAP,  // Connected AP info
    AT_CWQAP, // Disconnect from AP
    AT_CIPSERVER // TCP Server
};

enum Type {
    SET_EXECUTE,
    INQUIRY
};

class Wifi : public QMainWindow
{
    Q_OBJECT

public:
    explicit Wifi(QWidget *parent = 0);
    ~Wifi();

public slots:
    void readPort();

private slots:
    void on_input_returnPressed();

    void test();
    void reset();
    void apList();
    void getIpAddress();
    void checkApConnection();
    void disconnectFromAp();
    void connectToAp();
    void startTcpServer();


private:
    void send(const QString text);
    void logSend(const QString text);
    void checkBuffer();
    void parseApList();
    void parseIpList();
    void parseAp();
    void pong();

    Ui::Wifi *ui;
    QSerialPort *port;
    QString buffer;
    Operation operation;
    Type type;

    static QMap<int, QString> encryption;
};

#endif // WIFI_H
