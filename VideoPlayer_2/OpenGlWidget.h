#ifndef OPENGLWIDGET_H
#define OPENGLWIDGET_H

#include <QOpenGLWidget>
#include <QPushButton>
#include <QPainter>
#include <QVBoxLayout>
#include "VPlayThread.h"

class OpenGlWidget : public QOpenGLWidget
{
    Q_OBJECT

public:
    explicit OpenGlWidget(QOpenGLWidget *parent = nullptr);
    ~OpenGlWidget();

    // 绘制FFmpeg转换来的每一帧图片
    void paintEvent(QPaintEvent *event);

public slots:
    // 接受从FFmpeg传输来的一帧图像
    void slotGetOneFrame(QImage img);

private:
    QPushButton *m_pBtnPlay; // 播放按钮

    QImage m_image; // 接受存放FFmpeg转换来的每一帧图片数据
    VPlayThread* m_vPlayThread; // 视频播放线程
};

#endif // OPENGlWIDGET_H
