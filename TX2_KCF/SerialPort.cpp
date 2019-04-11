#include "SerialPort.h"
#include <QDataStream>
#include <QMessageBox>
#include <QDebug>

SerialPort::SerialPort(QObject *parent) : QObject(parent)
{
    init();
}

void SerialPort::init()
{
    m_port.setBaudRate(QSerialPort::Baud115200);
    m_port.setFlowControl(QSerialPort::NoFlowControl);
    m_port.setDataBits(QSerialPort::Data8);
    m_port.setStopBits(QSerialPort::OneStop);
    m_port.setParity(QSerialPort::NoParity);
}

void SerialPort::writeData(int xData, int yData)
{
    if(m_port.open(QIODevice::ReadWrite))
    {
        QDataStream out(&m_port);

        out << xData;
        out << yData;
    }

}


