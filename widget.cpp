#include "widget.h"
#include <QHostAddress>
#include <QByteArray>
#include <QBuffer>
#include <QDebug>
#include "ui_widget.h"
extern Widget *w;
extern Socket *g_socket;
extern struct vdIn *tmpbuf_videoIn;
extern QMutex mutex;
extern QWaitCondition waitforReadData;
extern int tmpbuf_refresh;
Widget::Widget(QWidget *parent) :
   QWidget(parent),
   ui(new Ui::Widget)
{
   ui->setupUi(this);
   v4l2         = new V4L2();
   preview      = new PreView(this, "preview");
   prethread    = new PreThread();
   timer        = new QTimer();
   timer_show   = new QTimer();
   timer_review = new QTimer();
   //QObject::connect(g_socket->tcpsocket, &QTcpSocket::connected, this, &Widget::connSlot);
   QObject::connect(g_socket->tcpsocket, &QTcpSocket::readyRead, this, &Widget::readyRead);
   QObject::connect(g_socket->tcpsocket, &QTcpSocket::disconnected, this, &Widget::disconnSlot);
   thread = new QThread();
   senddata = new SendData();
   emitsig = new EmitSig();
   senddata->moveToThread(thread);
   QObject::connect(emitsig, SIGNAL(emit_sig()), senddata, SLOT(SendDataSlot()));
   tmpbuf_refresh   = 0;
   ui->SetIPtextEdit->setText(g_socket->strIP);
   ui->SetPorttextEdit->setText(g_socket->strPort);
   v4l2->init_video_device();//call init_video_device(),init_videoIn(),init_v4l2()
   get_videoIn = v4l2->videoIn;
   firstopensendthread_flag = true;
   disconnected_flag = true;
   start_preview_thread();
   QObject::connect(senddata,SIGNAL(connectedSignal()),this,SLOT(connSlot()));
   QObject::connect(timer,SIGNAL(timeout()),this,SLOT(enable()));
   QObject::connect(timer_show,SIGNAL(timeout()),this,SLOT(TextShow()));
   QObject::connect(ui->openCamBtn,SIGNAL(clicked()),this,SLOT(openCamSlot()));
   QObject::connect(ui->closeBtn,SIGNAL(clicked()),this,SLOT(closeSlot()));
   QObject::connect(ui->photoBtn,SIGNAL(clicked()),this,SLOT(takingPicturesSlot()));
   QObject::connect(ui->closecamBtn,SIGNAL(clicked()),this,SLOT(closeCameraSlot()));
   QObject::connect(ui->connBtn, SIGNAL(clicked()), this, SLOT(connSlot()));
   QObject::connect(ui->disconnBtn, SIGNAL(clicked()), this, SLOT(disconnSlot()));
   QObject::connect(ui->SetIPBtn, SIGNAL(clicked()), this, SLOT(SetIP()));
   QObject::connect(ui->SetPortBtn, SIGNAL(clicked()), this, SLOT(SetPort()));
   QObject::connect(ui->sendBtn, SIGNAL(clicked()), this, SLOT(sendSlot()));
}


Widget::~Widget()
{
   delete ui;
}


void Widget::ImageDisplay()
{
   if(!v4l2->has_cam)
   {
       exit(1);
   }
   ui->openCamBtn->setDisabled(true);
   v4l2->drop_frame = 3;
   v4l2->req_buffers(get_videoIn);
   while(!prethread->stopped)
   {
       curtime = (unsigned long)times(NULL);
       if((curtime - lasttime) > 0)
       {
           framerate = 100/(curtime - lasttime);
       }
       lasttime = curtime;
       mutex.lock();
       while((tmpbuf_refresh != 0) && !disconnected_flag)
       {
           waitforReadData.wait(&mutex);
       }
       preview->showCamera(get_videoIn);
       tmpbuf_refresh = 1;
       waitforReadData.wakeAll();
       mutex.unlock();
   }
}

void Widget::sendSlot()
{
    disconnected_flag = false;
    g_socket->moveToThread(thread);
    start_senddata_thread();
    ui->sendBtn->setDisabled(true);
}

void Widget::openCamSlot()
{
    v4l2->init_video_device();
    get_videoIn = v4l2->videoIn;
    ui->closecamBtn->setEnabled(true);
    ui->openCamBtn->setDisabled(true);
    ui->camerashowstatu->setText("图像显示区域");
    start_preview_thread();
    if(!disconnected_flag)
    {
        start_senddata_thread();
    }
}

void Widget::start_preview_thread(void)
{
    if(prethread->isRunning())
    {
        prethread->stopped = true;
        prethread->stop();
    }
    else
    {
        prethread->stopped = false;
        prethread->start();
    }
}

void Widget::start_senddata_thread(void)
{
    thread->start();
    emitsig->emitsigSlot();
}


void Widget::enable(void)
{
    if(timer->isActive())
    {
        timer->stop();
    }
}

void Widget::closeCameraSlot()
{
   prethread->stop();
   prethread->wait();
   senddata->stop();
   thread->wait(2);
   thread->quit();
   if(!disconnected_flag)
   {
       g_socket->disconn2Server();
       ui->textEdit->setText("连接断开");
       disconnected_flag = true;
   }
   ui->closecamBtn->setDisabled(true);
   ui->camerashowstatu->setText("Camera Closed");
   v4l2->release_camera(get_videoIn);
   ui->openCamBtn->setEnabled(true);
   ui->connBtn->setEnabled(true);
}

void Widget::takingPicturesSlot()
{
    int ret = 0;
    prethread->stop();
    prethread->quit();
    senddata->stop();
    thread->quit();
    v4l2->video_disable(get_videoIn);
    ret = v4l2->capture(get_videoIn);
    if(!ret)
    {
        ui->camerashowstatu->setText("Capture OK!");
        timer_show->start(1000);
    }
    else
    {
        ui->camerashowstatu->setText("Capture Failed!");
        timer_show->start(1000);
    }
    start_preview_thread();
    start_senddata_thread();
}

void Widget::TextShow()
{
    ui->camerashowstatu->setText("图像显示区域");
}

void Widget::closeSlot()
{
    senddata->stop();
    prethread->stop();
    thread->wait(2);
    thread->quit();
    prethread->wait(2);
    prethread->quit();
    g_socket->disconn2Server();
    v4l2->release_camera(get_videoIn);
    this->close();
}

void Widget::connSlot()
{
    if((!thread->isRunning()) && !firstopensendthread_flag)
    {
        start_senddata_thread();
        ui->disconnBtn->setEnabled(true);
    }
    if(!firstopensendthread_flag)
    {
        disconnected_flag = false;
    }
    firstopensendthread_flag = 0;
    g_socket->conn2Server();
    if(g_socket->tcpsocket->waitForConnected())
    {
        ui->textEdit->setText("和服务器建立连接成功");
        ui->connBtn->setDisabled(true);
    }
}

void Widget::disconnSlot()
{
    ui->disconnBtn->setDisabled(true);
    ui->textEdit->setText("连接断开");
    ui->connBtn->setEnabled(true);
    g_socket->disconn2Server();
    senddata->stop();
    thread->wait(2);
    thread->quit();
    disconnected_flag = true;
}


void Widget::readyRead()
{
    QByteArray array;
    g_socket->readyRead(&array);
    if(!array.isEmpty())
    {
        ui->textEdit->append(array);
    }
}

void Widget::SetIP()
{
    ui->ServerIPlineEdit->setText(g_socket->strIP);
    ui->SetIPBtn->setDisabled(true);
}
void Widget::SetPort()
{
    ui->ServerPortlineEdit->setText(g_socket->strPort);
    ui->SetPortBtn->setDisabled(true);
}

