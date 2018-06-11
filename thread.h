#ifndef THREAD_H
#define THREAD_H
#include <QThread>
#include <QImage>
#include <QObject>
#include "widget.h"
#include "socket.h"
#include "v4l2.h"

class PreThread : public QThread
{
    Q_OBJECT
public:
    explicit PreThread(QObject *parent = 0);
    void stop();
    bool stopped;

protected:
    void run();
};

#endif // THREAD_H
