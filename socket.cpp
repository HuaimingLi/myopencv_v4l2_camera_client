#include "socket.h"
#include "widget.h"
#include <QByteArray>

#define SERVER_IP   "202.204.53.132"
#define SERVER_PORT "8001"

Socket::Socket()
{
    strIP       = SERVER_IP;
    strPort     = SERVER_PORT;
    tcpsocket   = new QTcpSocket();
}

Socket::~Socket()
{
    delete tcpsocket;
}

int Socket::conn2Server()
{
    QString ServerIp    = strIP;
    qint16 ServerPort   = strPort.toInt();
    tcpsocket->connectToHost(QHostAddress(ServerIp),ServerPort);
    return 0;
}

int Socket::disconn2Server()
{
    tcpsocket->disconnectFromHost();
    tcpsocket->close();
    return 0;
}

int Socket::readyRead(QByteArray *arrayOut)
{
    *arrayOut = tcpsocket->readAll();
    return 0;
}

int Socket::senddata(QByteArray dataIn)
{
    tcpsocket->write(dataIn);
    return 0;
}
