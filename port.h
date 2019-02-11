#ifndef PORT_H
#define PORT_H

#include <QObject>
#include <QSerialPort>

class Port : public QObject
{
    Q_OBJECT
public:
    explicit Port(QObject *parent = nullptr);
    struct Settings {
        QString name;
        qint32 baudRate;
        QString stringBaudRate;
        QSerialPort::DataBits dataBits;
        QString stringDataBits;
        QSerialPort::Parity parity;
        QString stringParity;
        QSerialPort::StopBits stopBits;
        QString stringStopBits;
        QSerialPort::FlowControl flowControl;
        QString stringFlowControl;
        bool localEchoEnabled;
    };

signals:

public slots:

private:

    QSerialPort thisPort;
    Settings SettingsPort;
};

#endif // PORT_H
