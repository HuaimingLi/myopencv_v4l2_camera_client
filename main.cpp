#include "widget.h"
#include <QApplication>
#include <QtCore/QTextCodec>

Widget *w;
Socket *g_socket;
struct vdIn *tmpbuf_videoIn;
int i;
QMutex mutex;
QWaitCondition waitforReadData;
int tmpbuf_refresh;
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    g_socket = new Socket();
    w = new Widget();
    w->show();
    return a.exec();
}







