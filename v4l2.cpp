#include "v4l2.h"

int debug = 0;
int g_SupportedFormats[] = {V4L2_PIX_FMT_YUYV, V4L2_PIX_FMT_MJPEG, V4L2_PIX_FMT_RGB565};

V4L2::V4L2()
{
    drop_frame  = 3;
    preview_w   = 320;
    preview_h   = 240;
    picture_w   = 640;
    picture_h   = 480;
    fps         = 30;
    format      = V4L2_PIX_FMT_MJPEG;
    PIC_CNT     = 1;

}

int V4L2::isSupportThisFormat(int PixelFormat)
{
    int i;
    for(i = 0; i < sizeof(g_SupportedFormats)/sizeof(g_SupportedFormats[0]); i++)
    {
        if(g_SupportedFormats[i] == PixelFormat)
        {
            return 1;
        }
    }
    return 0;
}

int V4L2::video_enable(struct vdIn *vd)
{
    int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    int ret;
    //VIDIOC_STREAMON:启动设备
    ret = ioctl(vd->vdIn_fd, VIDIOC_STREAMON, &type);
    if(ret < 0)
    {
        perror("unable to start capture");
        return ret;
    }
    vd->vdIn_isStreaming = 1;
    return 0;
}

int V4L2::video_disable(struct vdIn *vd)
{
    int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    int ret, i;
    ret = ioctl(vd->vdIn_fd, VIDIOC_STREAMOFF, &type);
    if(ret < 0)
    {
        perror("unable to stop capture");
        return ret;
    }
    for(i = 0; i < BUFFER_NUM; i++)
    {
        munmap(vd->vdIn_mem[i], vd->vdIn_buffer.length);
    }
    vd->vdIn_isStreaming = 0;
    return 0;
}

int V4L2::init_v4l2(struct vdIn *vd)
{
    int ret = 0;
    if((vd->vdIn_fd = open(vd->vdIn_videodevice, O_RDWR)) < 0)
    {
        perror("Error opening V4L2 interface");
        exit(1);
    }
    memset(&vd->vdIn_cap, 0, sizeof(struct v4l2_capability));
    //VIDIOC_QUERYCAP:确定它是否支持视频捕捉设备，支持哪种接口(streaming/read,write)
    ret = ioctl(vd->vdIn_fd, VIDIOC_QUERYCAP, &vd->vdIn_cap);
    if(ret < 0)
    {
        printf("Error opening device %s:unable to query device!\n", vd->vdIn_videodevice);
        goto fatal;
    }
    if((vd->vdIn_cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) == 0)
    {
        printf("Error opening device %s: video capture not supported!\n", vd->vdIn_videodevice);
        goto fatal;
    }
    if(!(vd->vdIn_cap.capabilities & V4L2_CAP_STREAMING))
    {
        printf("%s do not support streaming i/o\n", vd->vdIn_videodevice);
        goto fatal;
    }
    if(vd->vdIn_cap.capabilities & V4L2_CAP_READWRITE)
    {
        printf("%s support read/write i/o\n", vd->vdIn_videodevice);
    }

    //set pixel format and frame size
    memset(&vd->vdIn_fmt, 0, sizeof(struct v4l2_format));
    vd->vdIn_fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    vd->vdIn_fmt.fmt.pix.width       = vd->vdIn_pre_width;
    vd->vdIn_fmt.fmt.pix.height      = vd->vdIn_pre_height;
    vd->vdIn_fmt.fmt.pix.pixelformat = vd->vdIn_formatIn;
    vd->vdIn_fmt.fmt.pix.field       = V4L2_FIELD_ANY;
    //VIDIOC_S_FMT:设置摄像头使用哪种格式
    ret = ioctl(vd->vdIn_fd, VIDIOC_S_FMT, &vd->vdIn_fmt);
    if(ret < 0)
    {
        perror("Unable to set format");
        goto fatal;
    }
    if((vd->vdIn_fmt.fmt.pix.width != vd->vdIn_pre_width) || (vd->vdIn_fmt.fmt.pix.height != vd->vdIn_pre_height))
    {
        printf("Frame size: %ux%u (requested size %ux%u is not supported by device)\n",
               vd->vdIn_fmt.fmt.pix.width, vd->vdIn_fmt.fmt.pix.height, vd->vdIn_pre_width, vd->vdIn_pre_height);
        vd->vdIn_pre_width   = vd->vdIn_fmt.fmt.pix.width;
        vd->vdIn_pre_height  = vd->vdIn_fmt.fmt.pix.height;
    }
    else
    {
        printf("Frame size: %dx%d\n", vd->vdIn_pre_width, vd->vdIn_pre_height);
    }

    //set fps

    memset(&vd->vdIn_setfps, 0, sizeof(struct v4l2_streamparm));
    vd->vdIn_setfps.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    vd->vdIn_setfps.parm.capture.timeperframe.numerator      = 1;
    vd->vdIn_setfps.parm.capture.timeperframe.denominator    = vd->vdIn_fps;
    //VIDIOC_S_PARM:设置摄像头帧率的属性
    ret = ioctl(vd->vdIn_fd, VIDIOC_S_PARM, &vd->vdIn_setfps);
    if(ret == -1)
    {
        perror("Unable to set frame rate");
        goto fatal;
    }
    //VIDIOC_G_PARM:设置获取摄像头的帧率
    ret = ioctl(vd->vdIn_fd, VIDIOC_G_PARM, &vd->vdIn_setfps);
    if(ret == 0)
    {
        if((vd->vdIn_setfps.parm.capture.timeperframe.numerator != 1) || (vd->vdIn_setfps.parm.capture.timeperframe.denominator != vd->vdIn_fps))
        {
            printf("Frame rate: %u/%u fps (requested frame rate %u fps is not supported by device)\n",
                   vd->vdIn_setfps.parm.capture.timeperframe.denominator,
                   vd->vdIn_setfps.parm.capture.timeperframe.numerator,
                   vd->vdIn_fps);
        }
        else
        {
            printf("Frame rate: %d fps\n", vd->vdIn_fps);
        }
    }
    else
    {
        perror("Unable to read out current frame rate");
        goto fatal;
    }
    return 0;
fatal:
    return -1;
}

int V4L2::init_videoIn(struct vdIn *vd, const char *deviceName, int width, int height, int fps, int format)
{
    if(vd == NULL || deviceName == NULL)
    {
        return -1;
    }
    if(width == 0 || height == 0)
    {
        return -1;
    }
    vd->vdIn_videodevice = NULL;
    vd->vdIn_videodevice = (char *)calloc(1, 16 * sizeof(char));
    snprintf(vd->vdIn_videodevice, 12, "%s", deviceName);
    printf("Device information:\n");
    printf("Device path: %s\n", vd->vdIn_videodevice);
    vd->vdIn_pre_width   = width;
    vd->vdIn_pre_height  = height;
    vd->vdIn_fps         = fps;
    vd->vdIn_formatIn    = format;
    vd->vdIn_bytesWritten    = 0;
    vd->vdIn_framesWritten   = 0;
    vd->vdIn_isStreaming     = 0;
    vd->vdIn_framecount      = 0;

    if(init_v4l2(vd) < 0)
    {
        printf("Init V4L2 failed, exit fatal!\n");
        goto myerror;
    }
    vd->vdIn_framesizeIn = (vd->vdIn_pre_width * vd->vdIn_pre_height << 1);
    //printf("vd->vdIn_framesizeIn = %u\n", vd->vdIn_framesizeIn);
    vd->vdIn_tmpbuffer = (unsigned char *)calloc(1, (size_t)vd->vdIn_framesizeIn);
    vd->vdIn_framebuffer = (unsigned char *)calloc(1, (size_t)vd->vdIn_framesizeIn);
    if(!vd->vdIn_tmpbuffer || !vd->vdIn_framebuffer)
    {
        goto myerror;
    }
    return 0;
 myerror:
    free(vd->vdIn_videodevice);
    ::close(vd->vdIn_fd);
    return -1;
}

void V4L2::init_video_device(void)
{
    const char *videodevice = "/dev/video0";
    if(access(videodevice, F_OK) == 0)
    {
        has_cam = 1;
        videoIn = (struct vdIn *)calloc(1, sizeof(struct vdIn));
        if(init_videoIn(videoIn, videodevice, preview_w, preview_h, fps, format) < 0)
        {
            exit(1);
        }
    }
    else
    {
        has_cam = 0;
    }
}

int V4L2::req_buffers(struct vdIn *vd)
{
    int i;
    int ret = 0;
    memset(&vd->vdIn_request_buf, 0, sizeof(struct v4l2_requestbuffers));
    vd->vdIn_request_buf.count   = BUFFER_NUM;
    vd->vdIn_request_buf.type    = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    vd->vdIn_request_buf.memory  = V4L2_MEMORY_MMAP;
    //VIDIOC_REQBUFS:申请buffer
    ret = ioctl(vd->vdIn_fd, VIDIOC_REQBUFS, &vd->vdIn_request_buf);
    if(ret < 0)
    {
        perror("Unable to allocate buffer");
        goto fatal;
    }
    for(i = 0; i < BUFFER_NUM; i++)
    {
        memset(&vd->vdIn_buffer, 0, sizeof(struct v4l2_buffer));
        vd->vdIn_buffer.index    = i;
        vd->vdIn_buffer.type     = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        vd->vdIn_buffer.memory   = V4L2_MEMORY_MMAP;
        //VIDIOC_QUERYBUF:确定每一个buffer的信息，并且mmap
        ret = ioctl(vd->vdIn_fd, VIDIOC_QUERYBUF, &vd->vdIn_buffer);
        if(ret < 0)
        {
            perror("Unable to query buffer");
            goto fatal;
        }
        if(debug)
        {
            printf("length: %u offset: %u\n", vd->vdIn_buffer.length, vd->vdIn_buffer.m.offset);
        }
        vd->vdIn_mem[i] = mmap(0, vd->vdIn_buffer.length, PROT_READ, MAP_SHARED, vd->vdIn_fd, vd->vdIn_buffer.m.offset);
        if(vd->vdIn_mem[i] == MAP_FAILED)
        {
            perror("Unable to map buffer");
            goto fatal;
        }
        if(debug)
        {
            printf("Buffer mapped at address %p\n", vd->vdIn_mem[i]);
        }
    }
    //Queue the buffer
    for(i = 0; i < BUFFER_NUM; i++)
    {
        memset(&vd->vdIn_buffer, 0, sizeof(struct v4l2_buffer));
        vd->vdIn_buffer.index    = i;
        vd->vdIn_buffer.type     = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        vd->vdIn_buffer.memory   = V4L2_MEMORY_MMAP;
        //VIDIOC_QBUF:放入队列
        ret = ioctl(vd->vdIn_fd, VIDIOC_QBUF, &vd->vdIn_buffer);
        if(ret < 0)
        {
            perror("Unable to queue buffer");
            goto fatal;
        }
    }
    return 0;
 fatal:
    return -1;
}

int V4L2::capture(struct vdIn *vd)
{
    int device_num;
    int ret = 0;
    char capturedevice[16];
    strcpy(capturedevice, vd->vdIn_videodevice);
    sscanf(capturedevice, "/dev/video%d", &device_num);
    release_camera(vd);
    captureIn = (struct vdIn *)calloc(1, sizeof(struct vdIn));
    if(init_videoIn(captureIn, (const char *)capturedevice, picture_w, picture_h, fps, format) < 0)
    {
        exit(1);
    }
    req_buffers(captureIn);
    ret = do_capture(captureIn);
    printf("do capture\n");
    if(device_num == 0)
    {
        if(init_videoIn(videoIn, (const char *)capturedevice, preview_w, preview_h, fps, format) < 0)
        {
            exit(1);
        }
    }
    printf("6xxxxx!!!\n");
    if(captureIn)
    {
        free(captureIn);
        captureIn = NULL;
    }
    return ret;
}

int V4L2::do_capture(struct vdIn *vd)
{
    int loop = 10;
    unsigned int i;

 #define HEADER_FRAME 0xaf
    int ret;
    if(!vd->vdIn_isStreaming)
    {
        if(video_enable(vd))
        {
            goto myerror;
        }
    }
    memset(&vd->vdIn_buffer, 0, sizeof(struct v4l2_buffer));
    vd->vdIn_buffer.type     = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    vd->vdIn_buffer.memory   = V4L2_MEMORY_MMAP;

    while(loop--)
    {
        ret = ioctl(vd->vdIn_fd, VIDIOC_DQBUF, &vd->vdIn_buffer);
        if(ret < 0)
        {
            perror("Unable to dequeue buffer");
            goto myerror;
        }
        if(vd->vdIn_buffer.bytesused <= HEADER_FRAME)
        {
            printf("Ignoring empty buffer...\n");
            return 0;
        }
        if(loop == 1)
        {
            memcpy(vd->vdIn_tmpbuffer, vd->vdIn_mem[vd->vdIn_buffer.index], vd->vdIn_buffer.bytesused);
            get_picture(vd->vdIn_tmpbuffer, vd->vdIn_buffer.bytesused, NULL);
        }
        ret = ioctl(vd->vdIn_fd, VIDIOC_QBUF, &vd->vdIn_buffer);
        if(ret < 0)
        {
            perror("Unable to requeue buffer");
            goto myerror;
        }
    }
    if(vd->vdIn_isStreaming)
    {
        video_disable(vd);
    }
    if(vd->vdIn_tmpbuffer)
    {
        free(vd->vdIn_tmpbuffer);
    }
    vd->vdIn_tmpbuffer = NULL;
    for(i = 0; i < BUFFER_NUM; i++)
    {
        munmap(vd->vdIn_mem[i], vd->vdIn_buffer.length);
    }
    free(vd->vdIn_videodevice);
    vd->vdIn_videodevice = NULL;
    ::close(vd->vdIn_fd);
    return 0;
 myerror:
    return -1;
}

int V4L2::v4l2_grab(struct vdIn *vd)
{
 #define HEADER_FRAME 0xaf
    int ret;
    if(!vd->vdIn_isStreaming)
    {

        if(video_enable(vd))
        {
            goto myerror;
        }
    }
    memset(&vd->vdIn_buffer, 0, sizeof(v4l2_buffer));
    vd->vdIn_buffer.type     = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    vd->vdIn_buffer.memory   = V4L2_MEMORY_MMAP;
    //VIDIOC_DQBUF:从队列中取出
    ret = ioctl(vd->vdIn_fd, VIDIOC_DQBUF, &vd->vdIn_buffer);
    if(ret < 0)
    {
        perror("Unable to dequeue buffer");
        goto myerror;
    }
    if(vd->vdIn_buffer.bytesused <= HEADER_FRAME)
    {
        printf("Ignoring empty buffer...\n");
        return 0;
    }
    if(drop_frame)
    {
        drop_frame--;
    }
    else
    {
        memcpy(vd->vdIn_tmpbuffer, vd->vdIn_mem[vd->vdIn_buffer.index], vd->vdIn_buffer.bytesused);
        //memcpy(vd->vdIn_framebuffer, vd->vdIn_mem[vd->vdIn_buffer.index], vd->vdIn_buffer.bytesused);
    }
    //VIDIOC_QBUF:再次放入队列
    ret = ioctl(vd->vdIn_fd, VIDIOC_QBUF, &vd->vdIn_buffer);
    if(ret < 0)
    {
        perror("Unable to requeue buffer");
        goto myerror;
    }

    return 0;
 myerror:
    return -1;
}

void V4L2::release_camera(struct vdIn *vd)
{
    unsigned int i;
    if(vd->vdIn_isStreaming)
    {
        video_disable(vd);
    }
    if(vd->vdIn_tmpbuffer)
    {
        free(vd->vdIn_tmpbuffer);
    }
    if(vd->vdIn_framebuffer)
    {
        free(vd->vdIn_framebuffer);
    }
    vd->vdIn_tmpbuffer = NULL;
    vd->vdIn_framebuffer = NULL;
    for(i = 0; i < BUFFER_NUM; i++)
    {
        munmap(vd->vdIn_mem[i], vd->vdIn_buffer.length);
    }
    if(vd->vdIn_videodevice)
    {
        free(vd->vdIn_videodevice);
    }
    vd->vdIn_videodevice = NULL;
    ::close(vd->vdIn_fd);
    vd->vdIn_fd = -1;
    free(videoIn);
    videoIn = NULL;
}

int V4L2::get_picture(unsigned char *buf, int size, char *filename)
{
    FILE *file;
    unsigned char *ptdeb, *ptcur = buf;
    int sizeIn;
    char *name = NULL;

    if(filename == NULL)
    {
        name = (char *)calloc(80, 1);
        get_pictureName(name);
        file = fopen(name, "wb");
    }
    else
    {
        file = fopen(filename, "wb");
    }
    if(file != NULL)
    {
        if(!is_huffman(buf))
        {
            ptdeb = ptcur = buf;
            while(((ptcur[0] << 8) | ptcur[1]) != 0xffc0)
            {
                ptcur++;
            }
            sizeIn = ptcur - ptdeb;
            fwrite(buf, sizeIn, 1, file);
            fwrite(dht_data, DHT_SIZE, 1, file);
            fwrite(ptcur, size - sizeIn, 1, file);
        }
        else
        {
            fwrite(ptcur, size, 1, file);
        }
        fclose(file);
    }
    if(name)
    {
        free(name);
    }
    return 0;
}

void V4L2::get_pictureName(char *picture)
{
    char tmp[30];
    memset(tmp, '\0', sizeof(tmp));
    snprintf(tmp, 30, "/work/nfs_root/my_fs_mdev/camera_pictures/%u.jpg", PIC_CNT);
    PIC_CNT++;
    memcpy(picture, tmp, strlen(tmp));
}

int V4L2::is_huffman(unsigned char *buf)
{
    unsigned char *ptbuf;
    int i = 0;
    ptbuf = buf;
    while(((ptbuf[0] << 8) | ptbuf[1]) != 0xffda)
    {
        if(i++ > 2048)
        {
            return 0;
        }
        if(((ptbuf[0] << 8) | ptbuf[1]) == 0xffc4)
        {
            return 1;
        }
        ptbuf++;
    }
    return 0;
}
