#-------------------------------------------------
#
# Project created by QtCreator 2017-11-11T10:34:27
#
#-------------------------------------------------

QT       += core gui network
#CONFIG += C++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = myopencv_v4l2_camera_client
TEMPLATE = app


SOURCES += main.cpp\
        widget.cpp \
    socket.cpp \
    v4l2.cpp \
    preview.cpp \
    thread.cpp \
    senddata.cpp

HEADERS  += widget.h \
    socket.h \
    v4l2.h \
    huffman.h \
    preview.h \
    thread.h \
    senddata.h

FORMS    += widget.ui

INCLUDEPATH += /usr/local/arm/opencv-s3c2440/include/opencv \
            /usr/local/arm/opencv-s3c2440/include/opencv2 \
            /usr/local/arm/opencv-s3c2440/include  \
            /opt/FriendlyARM/toolschain/4.4.3/include \
            /opt/FriendlyARM/toolschain/4.4.3/arm-none-linux-gnueabi/sys-root/usr/include


LIBS += /usr/local/arm/opencv-s3c2440/lib/libopencv_calib3d.so \
     /usr/local/arm/opencv-s3c2440/lib/libopencv_contrib.so \
     /usr/local/arm/opencv-s3c2440/lib/libopencv_core.so \
     /usr/local/arm/opencv-s3c2440/lib/libopencv_features2d.so \
     /usr/local/arm/opencv-s3c2440/lib/libopencv_flann.so \
     /usr/local/arm/opencv-s3c2440/lib/libopencv_gpu.so \
     /usr/local/arm/opencv-s3c2440/lib/libopencv_highgui.so \
     /usr/local/arm/opencv-s3c2440/lib/libopencv_imgproc.so \
     /usr/local/arm/opencv-s3c2440/lib/libopencv_legacy.so \
     /usr/local/arm/opencv-s3c2440/lib/libopencv_ml.so \
     /usr/local/arm/opencv-s3c2440/lib/libopencv_objdetect.so \
     /usr/local/arm/opencv-s3c2440/lib/libopencv_video.so  \

