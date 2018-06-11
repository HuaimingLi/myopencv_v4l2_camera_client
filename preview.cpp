#include "preview.h"

PreView::PreView(QWidget *parent, const char *name):
        QLabel(name, parent)
{
    int width = 229;
    int height = 220;
    //int width = 480;
    //int height = 272;
    setFixedSize(width, height);
    pixmap = new QImage(width, height, QImage::Format_RGB32);
    v4l2_handle = new V4L2();
}

void PreView::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    //painter.drawImage(0,0,*pixmap);
    painter.drawImage(1,33,*pixmap);
}

int PreView::showCamera(struct vdIn *vd)
{
    v4l2_handle->v4l2_grab(vd);
    pixmap->loadFromData(vd->vdIn_tmpbuffer, (vd->vdIn_pre_width) * (vd->vdIn_pre_height) * 2);
    //pixmap->loadFromData(vd->vdIn_tmpbuffer, (vd->vdIn_pre_width) * (vd->vdIn_pre_height));
    update();
    return 0;
}
