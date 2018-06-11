#ifndef SOCKET_H
#define SOCKET_H

#include <QTcpSocket>
#include <QString>

class Socket : public QTcpSocket
{
public:
    Socket();
    ~Socket();
    QTcpSocket *tcpsocket;
    QString     strIP;
    QString     strPort;

    int conn2Server();
    int disconn2Server();
    int readyRead(QByteArray *);
    int senddata(QByteArray);

private:


private slots:

};

#endif // SOCKET_H
