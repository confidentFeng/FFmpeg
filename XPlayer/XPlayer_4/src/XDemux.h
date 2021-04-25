#ifndef XDEMUX_H
#define XDEMUX_H

#include <iostream>
#include <mutex>

// 调用FFmpeg的头文件
extern "C"{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}

using namespace std;

// 视频信息结构体
typedef struct _stVideoInfo
{
	int totalMs = 0; // 媒体总时长（毫秒）
	int width = 0; // 视频宽
	int height = 0; // 视频高
	int sampleRate = 0;
	int channels = 0;
}stVideoInfo;

// 解封装类
class XDemux
{
public:
    XDemux();
    virtual ~XDemux();

    bool open(const char *url); // 打开媒体文件或者流媒体（rtsp、rtmp、http）
    AVPacket *read(); // 读取一帧AVPacket
    AVCodecParameters *copyVPara(); // 获取视频参数
    AVCodecParameters *copyAPara(); // 获取音频参数
    bool isAudio(AVPacket *pkt); // 是否为音频
    bool seek(double pos); // seek位置（pos 0.0~1.0）
    void close(); // 关闭
    stVideoInfo getVideoInfo(); // 获取视频信息

private:
    std::mutex m_mutex; // 互斥锁
    bool m_isFirst = true; // 是否第一次初始化，避免重复初始化

    AVFormatContext *pFormatCtx = NULL; // 解封装上下文
    int nVStreamIndex = -1; // 视频流索引
    int nAStreamIndex = -1; // 音频流索引

    stVideoInfo m_stVideoInfo;
};

#endif // XDEMUX_H
