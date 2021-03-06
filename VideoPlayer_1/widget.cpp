#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);

    m_vPlayThread = new VPlayThread();
    // 连接更新图片信号槽
    connect(m_vPlayThread, SIGNAL(sig_GetOneFrame(QImage)),this,SLOT(slotGetOneFrame(QImage)));
}

Widget::~Widget()
{
    delete ui;
}

// 绘制FFmpeg转换来的每一帧图片
void Widget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    // 设置painter
    QPainter painter(this);
    painter.setBrush(Qt::black);
    painter.drawRect(0, 0, this->width(), this->height()); // 先画成黑色
    if (m_image.size().width() <= 0) return;

    // 将图像按比例缩放成和窗口一样大小
    QImage img = m_image.scaled(this->size(),Qt::KeepAspectRatio);
    int x = this->width()  - img.width();
    int y = this->height() - img.height();
    x /= 2;
    y /= 2;

    // 绘制图像
    painter.drawImage(QPoint(x,y),img);
}

// 接受从FFmpeg传输来的一帧图像
void Widget::slotGetOneFrame(QImage img)
{
    m_image = img;
    this->update();
}

void Widget::on_pushButton_clicked()
{
    // 开启线程，解码播放视频
    m_vPlayThread->start();
}
