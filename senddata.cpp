#include "senddata.h"

extern Widget *w;
extern Socket *g_socket;
extern int i;
extern QMutex mutex;
extern QWaitCondition waitforReadData;
extern int tmpbuf_refresh;

EmitSig::EmitSig()
{

}

void EmitSig::emitsigSlot()
{
    emit emit_sig();
    emit emit2ImageShow();
}

SendData::SendData()
{
    stopped = false;
    i = 0;
}

void SendData::stop()
{
    stopped = true;
}

void SendData::SendDataSlot()
{
    stopped = false;
    QImage img;
    while(!stopped)
    {
        mutex.lock();
        while(tmpbuf_refresh == 0)
        {
            waitforReadData.wait(&mutex);
        }
        img.loadFromData(w->get_videoIn->vdIn_tmpbuffer, (w->get_videoIn->vdIn_pre_width) * (w->get_videoIn->vdIn_pre_height) * 2);
        QBuffer buffer;
        img.save(&buffer, "JPEG");
        QByteArray data;
        data.append(buffer.data());
        g_socket->tcpsocket->write(data);
        g_socket->tcpsocket->waitForBytesWritten();
        tmpbuf_refresh = 0;
        waitforReadData.wakeAll();
        mutex.unlock();
        i++;
        printf("i = %d\n", i);
    }
}


