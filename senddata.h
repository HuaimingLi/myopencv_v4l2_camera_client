#ifndef SENDDATA_H
#define SENDDATA_H
#include "widget.h"
#include "socket.h"
#include <QApplication>
#include <QtCore/QTextCodec>
#include <QTcpSocket>
#include <QObject>
#include <QThread>
#include <QtCore/QObject>
#include <QtCore/QThread>
#include <QByteArray>

class Socket;

class EmitSig : public QObject
{
    Q_OBJECT
public:
    EmitSig();

public slots:
    void emitsigSlot();

signals:
    void emit_sig();
    void emit2ImageShow();
};

class SendData : public QObject
{
    Q_OBJECT
public:
    SendData();
    void stop();
    bool stopped;

public slots:
    void SendDataSlot();

signals:
    void connectedSignal();

private:
    bool start2disconn;

};

#endif // SENDDATA_H
