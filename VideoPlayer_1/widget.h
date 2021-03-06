#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QPainter>
#include "VPlayThread.h"

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget();

    // 绘制FFmpeg转换来的每一帧图片
    void paintEvent(QPaintEvent *event);

public slots:
    // 接受从FFmpeg传输来的一帧图像
    void slotGetOneFrame(QImage img);

private slots:
    void on_pushButton_clicked();

private:
    Ui::Widget *ui;

    QImage m_image; // 接受存放FFmpeg转换来的每一帧图片数据
    VPlayThread* m_vPlayThread; // 视频播放线程
};

#endif // WIDGET_H
