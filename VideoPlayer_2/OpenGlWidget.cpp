#include "OpenGlWidget.h"

OpenGlWidget::OpenGlWidget(QOpenGLWidget *parent) :
    QOpenGLWidget(parent)
{
    // 初始化窗口大小
    this->setFixedSize(533, 300);

    // 初始化播放按钮
    m_pBtnPlay = new QPushButton;
    m_pBtnPlay->setFixedSize(53, 23);
    m_pBtnPlay->setText("播放");

    // 布局
    QVBoxLayout *m_pVLayout = new QVBoxLayout(this);
    m_pVLayout->addStretch();
    m_pVLayout->addWidget(m_pBtnPlay, 0, Qt::AlignHCenter);
    m_pVLayout->addSpacing(12);

    m_vPlayThread = new VPlayThread();
    // 连接更新图片信号槽
    connect(m_vPlayThread, SIGNAL(sig_GetOneFrame(QImage)),this,SLOT(slotGetOneFrame(QImage)));
    // 连接播放按钮的信号槽
    connect(m_pBtnPlay, &QPushButton::clicked, [=] {
        // 开启线程，解码播放视频
        m_vPlayThread->start();
    });
}

OpenGlWidget::~OpenGlWidget()
{

}

// 绘制FFmpeg转换来的每一帧图片
void OpenGlWidget::paintEvent(QPaintEvent *event)
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
void OpenGlWidget::slotGetOneFrame(QImage img)
{
    m_image = img;
    this->update();
}
