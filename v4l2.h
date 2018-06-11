#ifndef V4L2_H
#define V4L2_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/mman.h>       //for map_shared
#include <sys/select.h>
#include <fcntl.h>
#include <errno.h>
#include <stdint.h>
#include <time.h>
#include <linux/videodev2.h>
#include <error.h>
#include <unistd.h>
#include "huffman.h"

#define BUFFER_NUM  4

struct vdIn{
    struct v4l2_capability      vdIn_cap;
    struct v4l2_format          vdIn_fmt;
    struct v4l2_buffer          vdIn_buffer;
    struct v4l2_requestbuffers  vdIn_request_buf;
    struct v4l2_streamparm      vdIn_setfps;
    struct v4l2_fmtdesc         vdIn_fmtDesc;

    int     vdIn_fd;
    char    *vdIn_videodevice;
    void    *vdIn_mem[BUFFER_NUM];
    unsigned char *vdIn_tmpbuffer;
    unsigned char *vdIn_framebuffer;
    unsigned int     vdIn_pre_width;
    unsigned int     vdIn_pre_height;
    unsigned int     vdIn_fps;
    int     vdIn_pic_width;
    int     vdIn_pic_height;  
    int     vdIn_formatIn;
    int     vdIn_formatOut;
    int     vdIn_isStreaming;
    int     vdIn_framecount;
    unsigned int vdIn_bytesWritten;
    unsigned int vdIn_framesWritten;
    unsigned int vdIn_framesizeIn;

};

class V4L2
{
public:
    V4L2();
    int     video_enable(struct vdIn *);
    int     video_disable(struct vdIn *);
    int     init_v4l2(struct vdIn *);
    int     init_videoIn(struct vdIn *, const char *, int, int, int, int);
    void    init_video_device(void);
    int     req_buffers(struct vdIn *);
    int     capture(struct vdIn *);
    int     do_capture(struct vdIn *);
    int     v4l2_grab(struct vdIn *);
    void    release_camera(struct vdIn *);
    int     get_picture(unsigned char *, int, char *);
    void    get_pictureName(char *);
    int     is_huffman(unsigned char *);
    int     isSupportThisFormat(int);

public:
    int has_cam;
    struct vdIn *videoIn;
    struct vdIn *captureIn;
    unsigned int drop_frame;
    unsigned int preview_w;
    unsigned int preview_h;
    unsigned int picture_w;
    unsigned int picture_h;
    unsigned int fps;
    unsigned int format;
    unsigned int PIC_CNT;

private:
    char *device_name;
    int pixel_format;
    int fd;

};

#endif // V4L2_H
