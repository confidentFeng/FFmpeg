#ifndef VPLAYTHREAD_H
#define VPLAYTHREAD_H

#include <QThread>
#include <QTime>
#include <QImage>

// 添加FFmpeg的头文件
extern "C"{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}

class VPlayThread : public QThread
{
    Q_OBJECT

public:
    VPlayThread();
    void run();

signals:
    void sig_GetOneFrame(QImage); // 获取到一帧图像 就发送此信号
};

#endif // VPLAYTHREAD_H
