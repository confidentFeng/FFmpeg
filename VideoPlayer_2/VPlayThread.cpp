#include "VPlayThread.h"

VPlayThread::VPlayThread()
{

}

void VPlayThread::run()
{
    // 打开视频
    if(!XFFmpeg::GetInstance()->openVideo("../VideoPlayer_2/test.mp4"))
        return;
    // 查找视频流
    if(!XFFmpeg::GetInstance()->findVideoStream())
        return;
    // 打开视频解码器
    if(!XFFmpeg::GetInstance()->openVCodec())
        return;
    // 分配一帧图像和AVFrame等所需的空间
    XFFmpeg::GetInstance()->allocFrame();

    for (;;)
    {
        // 解码视频帧
        AVPacket *packet = (AVPacket *)av_malloc(sizeof(AVPacket));
        QImage image;
        int ret = XFFmpeg::GetInstance()->decode(packet, image);//每次读取视频得一帧
        if (ret == -1)
            break;
        if(ret == -2)
            continue;
        if(ret == 0)
            emit sig_GetOneFrame(image); // 发射更新图片信号，传递image给界面显示

        // 打印它的pts,用来显示的时间戳，后面用来做同步
        //printf("pts = %d", packet->pts);
        // 设置每延时40ms读取一帧图片，达到25帧播放视频的效果（25帧：1000/25=40）
        QTime timer = QTime::currentTime().addMSecs(40);
        while( QTime::currentTime() < timer);

        // 每读取一次视频帧，就要清空一次packet
        av_free_packet(packet);
    }

    // 关闭文件
    XFFmpeg::GetInstance()->Close();
}
