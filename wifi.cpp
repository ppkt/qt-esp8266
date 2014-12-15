#include "wifi.h"
#include "ui_wifi.h"
#include "accesspoint.h"

#include <QDebug>
#include <QHostAddress>
#include <QRegularExpression>

//Wifi::encryption[0] = "Open";

QMap<int, QString> Wifi::encryption;

Wifi::Wifi(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Wifi)
{
    ui->setupUi(this);

    this->encryption[0] = "Open";
    this->encryption[1] = "WEP";
    this->encryption[2] = "WPA_PSK";
    this->encryption[3] = "WPA2_PSK";
    this->encryption[4] = "WPA2_WPA_PSK";

    this->port = new QSerialPort(this);
    this->port->setPortName("ttyUSB0");

    connect(this->port, SIGNAL(readyRead()), SLOT(readPort()));
    bool status = this->port->open(QIODevice::ReadWrite);
    if (status) {
        this->port->setBaudRate(QSerialPort::Baud9600, QSerialPort::AllDirections);
        this->port->setDataBits(QSerialPort::Data8);
        this->port->setStopBits(QSerialPort::OneStop);
        this->port->setFlowControl(QSerialPort::NoFlowControl);
        this->port->setParity(QSerialPort::NoParity);
    }
    else {
        qDebug() << "Port closed";
        this->port->close();
    }

    connect(ui->btnTest, SIGNAL(clicked()), SLOT(test()));
    connect(ui->btnReset, SIGNAL(clicked()), SLOT(reset()));
    connect(ui->btnApList, SIGNAL(clicked()), SLOT(apList()));
    connect(ui->btnGetIpAddress, SIGNAL(clicked()), SLOT(getIpAddress()));
    connect(ui->btnCheckConnection, SIGNAL(clicked()), SLOT(checkApConnection()));
    connect(ui->btnDisconnect, SIGNAL(clicked()), SLOT(disconnectFromAp()));
    connect(ui->btnConnect, SIGNAL(clicked()), SLOT(connectToAp()));
    connect(ui->btnStartServer, SIGNAL(clicked()), SLOT(startTcpServer()));
}

void Wifi::readPort() {
    QByteArray data =  this->port->readAll();

    if (data.isNull()) {
        return;
    }

    buffer.append(data);
    checkBuffer();

    ui->output->insertPlainText(data);
    ui->output->ensureCursorVisible();
}

Wifi::~Wifi()
{
    delete ui;
}

void Wifi::on_input_returnPressed()
{
    this->send(ui->input->text());
    ui->input->clear();
}

void Wifi::test()
{
    ui->log->appendPlainText("Testing connection");
    this->send("AT");
}

void Wifi::reset()
{
    ui->log->appendPlainText("Reseting adapter");
    this->send("AT+RST");
}

void Wifi::apList()
{
    ui->log->appendPlainText("Searching available Access Points");
    this->send("AT+CWLAP");
    this->operation = AT_CWLAP;
}

void Wifi::getIpAddress()
{
    ui->log->appendPlainText("Getting IP address");
    this->send("AT+CIFSR");
    this->operation = AT_CIFSR;
}

void Wifi::checkApConnection()
{
    ui->log->appendPlainText("Checking connection");
    this->send("AT+CWJAP?");
    this->operation = AT_CWJAP;
    this->type = INQUIRY;
}

void Wifi::disconnectFromAp()
{
    ui->log->appendPlainText("Disconnecting from AP");
    this->send("AT+CWQAP");
    this->operation = AT_CWQAP;
    this->type = SET_EXECUTE;
}

void Wifi::connectToAp()
{
    ui->log->appendPlainText("Connecting to AP");
    this->send("AT+CWJAP=\"UPC0050947\",\"FZCYYCEG\"");
    this->operation = AT_CWJAP;
    this->type = SET_EXECUTE;
}

void Wifi::send(const QString text)
{
    QString input = text + "\r\n";
    this->port->write(input.toLocal8Bit());
}

void Wifi::logSend(const QString text)
{
    ui->log->appendPlainText(text);
}

void Wifi::checkBuffer()
{
    if (this->buffer.endsWith("OK\r\n") or this->buffer.endsWith("ERROR\r\n")) {

        if (operation == AT_CWLAP) {
            this->parseApList();
        }
        else if (operation == AT_CIFSR) {
            this->parseIpList();
        }
        else if (operation == AT_CWJAP) {
            if (type == INQUIRY) {
                this->parseAp();
            }
        }
        else if (operation == AT_CIPSERVER) {

        }
        if (this->buffer.contains("+IPD")) {
            this->pong();
        }

        if (this->buffer.endsWith("OK\r\n"))
            this->logSend("Operation successful");
        else
            this->logSend("Operation NOT successful");
        this->buffer.clear();
    } else if (this->buffer.endsWith("ready\r\n")) {
        this->logSend("Adapter is ready");
        this->buffer.clear();
    }
}

void Wifi::parseApList()
{
    QRegularExpression re("\\+CWLAP:\\((.*)\\)");
    QRegularExpressionMatch match;
    QString ap;
    QStringList elements;
    QStringList entries = buffer.split("\r\n");
    foreach (QString element, entries) {
        match = re.match(element);
        if (!match.hasMatch())
            continue;

        ap = match.captured(1);
        elements = ap.split(",");

        // Encryption
        AccessPoint ap;
        ap.encryption = static_cast<Encryption>(elements.at(0).toInt());
        ap.essid = elements[1].remove(elements.at(1).length()-1, 1).remove(0, 1);
        ap.signal = elements[2].toInt();
        ap.mac = elements[3].remove(elements[3].length()-1, 1).remove(0, 1);
        ap.channel = elements[4].toInt();

        qDebug() << encryption[ap.encryption] << ap.essid << ap.signal << ap.mac << ap.channel;

        QString log = QString("%1 [%2]:\nsignal: %3dB\nchannel: %4\nencryption: %5\n")
                .arg(ap.essid)
                .arg(ap.mac)
                .arg(ap.signal)
                .arg(ap.channel)
                .arg(encryption[ap.encryption]);
        logSend(log);
    }
}

void Wifi::parseIpList()
{
    QStringList entries = buffer.split("\r\n");
    QHostAddress host;
    foreach (QString element, entries) {
        if (host.setAddress(element)) {
            logSend(element);
        }
    }
}

void Wifi::parseAp()
{
    bool connected = false;
    QRegularExpression re("\\+CWJAP:\"(.*)\"");
    QRegularExpressionMatch match;
    QStringList entries = buffer.split("\r\n");
    QHostAddress host;
    foreach (QString element, entries) {
        match = re.match(element);
        if (!match.hasMatch())
            continue;
        logSend("Connected to AP: " + match.captured(1));
        connected = true;
    }

    if (!connected) {
        logSend("NOT CONNECTED");
    }
}

void Wifi::pong()
{
    QStringList entries = buffer.split("\r\n");
    foreach (QString element, entries) {
        if (!element.startsWith("+IPD"))
            continue;

        // Extract ID of connection, size of data and content:
        QRegularExpression re("^\\+IPD,(?<id>\\d+),(?<length>\\d+):(?<data>.*)");
        QRegularExpressionMatch match = re.match(element);
        if (!match.hasMatch()) {
            qDebug() << "No match";
            return;
        }
        qDebug() << match.captured("data");

        QString reply = QString("AT+CIPSEND=%1,%2")
                .arg(match.captured("id"))
                .arg(match.captured("length"));
        this->send(reply);
        this->send(match.captured("data"));
    }
}

void Wifi::startTcpServer()
{
    ui->log->appendPlainText("Stating TCP Server on port 9999");
    this->send("AT+CIPMUX=1");
    this->send("AT+CIPSERVER=1,9999");
    this->operation = AT_CIPSERVER;
    this->type = SET_EXECUTE;
}
