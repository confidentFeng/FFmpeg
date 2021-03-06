#ifndef VIDEOPLAY_H
#define VIDEOPLAY_H

#include "QObject"
#include <QImage>
#include <QMutex>

// 调用FFmpeg的头文件
extern "C"{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}

class VideoPlay
{
public:
    // 单例模式
    static VideoPlay *GetInstance()
    {
        static VideoPlay ff;
        return &ff;
    }

    // 打开视频文件（本质是解封装）
    bool openVideo(const char *filepath);
    // 查找视频流
    bool findVideoStream();
    // 打开视频解码器
    bool openVCodec();
    // 分配一帧图像和AVFrame等所需的空间
    bool allocFrame();
    // 解码
    int decode(AVPacket *packet, QImage &tmpImage);
    // 关闭文件
    void Close();

private:
    VideoPlay(); // 构造函数设置为私有，为了实现单例模式

    QMutex m_mutex; // 互斥变量，多线程时避免同时间的读写
    int m_nVideoStream = -1; // 视频流索引，初始化为-1
    AVFormatContext *m_pFormatCtx = NULL; // 解封装上下文
    AVCodecContext *m_pCodecCtx = NULL; // 视频解码上下文
    AVFrame *m_pFrameYUV = NULL; // YUV格式视频帧
    AVFrame *m_pFrameRGB = NULL; // RGB格式视频帧
    struct SwsContext *m_img_convert_ctx = NULL; // YUV转RGB的数据转换参数
};

#endif // VIDEOPLAY_H
