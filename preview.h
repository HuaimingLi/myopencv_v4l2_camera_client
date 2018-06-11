#ifndef PREVIEW_H
#define PREVIEW_H

#include<QWidget>
#include<QPixmap>
#include<QLabel>
#include<QPainter>
#include<QTimer>
#include<QImage>
#include "v4l2.h"

class V4L2;
class PreView : public QLabel
{
public:
    PreView(QWidget *parent = 0, const char *name = 0);
    void showPicData(unsigned char *, int);
    int showCamera(struct vdIn *);

private:
    QImage *pixmap;
    QPainter *painter;
    V4L2 *v4l2_handle;

protected:
    void paintEvent(QPaintEvent *event);

};

#endif // PREVIEW_H
