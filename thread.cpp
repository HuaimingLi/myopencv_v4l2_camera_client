#include "thread.h"
#include <QByteArray>
#include <QTcpSocket>
#include <QString>

extern Widget *w;
extern int i;
extern QMutex mutex;
extern QWaitCondition waitforReadData;
extern int tmpbuf_refresh;

PreThread::PreThread(QObject *parent):QThread(parent)
{
    stopped = false;
}

void PreThread::stop()
{
    stopped = true;
}

void PreThread::run()
{
    stopped = false;
    w->ImageDisplay();
}




