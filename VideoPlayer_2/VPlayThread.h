#ifndef VPLAYTHREAD_H
#define VPLAYTHREAD_H

#include <QThread>
#include <QTime>
#include "VideoPlay.h"

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
