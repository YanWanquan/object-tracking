#ifndef SERIALPORT_H
#define SERIALPORT_H

#include <QObject>
#include <QSerialPort>
class SerialPort : public QObject
{
    Q_OBJECT

    QSerialPort m_port;
    void init();

public:
    explicit SerialPort(QObject *parent = nullptr);

public slots:
    void writeData(int xData, int yData);
};

#endif // SERIALPORT_H
