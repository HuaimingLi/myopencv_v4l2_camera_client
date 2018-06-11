#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QDialog>
#include <QDebug>
#include <QTimer>
#include <QImage>
#include <QBuffer>
#include <QTcpServer>
#include <QTcpSocket>
#include <QWaitCondition>
#include <QMutex>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <sys/times.h>
#include "socket.h"
#include "v4l2.h"
#include "preview.h"
#include "thread.h"
#include "senddata.h"

using namespace cv;

//LCD 480x272
class Socket;
class V4L2;
class PreThread;
class PreView;
class SendData;
class EmitSig;

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();
    void ImageDisplay();
    void begin2senddata();
    struct vdIn *get_videoIn;
    SendData *senddata;
    QThread  *thread;
    EmitSig  *emitsig;


private:
    Ui::Widget *ui;
    Mat frame;//video frame
    QImage img;
    VideoCapture capture;
    bool firstopensendthread_flag;
    bool disconnected_flag;
    QTimer      *timer;
    QTimer      *timer_review;
    QTimer      *timer_show;
    QPixmap     *pixmap;
    PreThread   *prethread;//preview thread
    PreView     *preview;
    V4L2        *v4l2;

    unsigned char *tmpbuf;
    unsigned char framerate;
    unsigned long curtime;
    unsigned long lasttime;

    void check_camera(void);
    void start_preview_thread(void);
    void start_senddata_thread(void);

signals:
    void disconnSig();


private slots:

   void openCamSlot();
   void closeCameraSlot();
   void takingPicturesSlot();
   void closeSlot();
   void enable();
   void TextShow();
   void connSlot();
   void disconnSlot();
   void sendSlot();
   void readyRead();
   void SetIP();
   void SetPort();



};

#endif // WIDGET_H
