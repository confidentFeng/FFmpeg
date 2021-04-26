#ifndef XDECODE_H
#define XDECODE_H

#include <iostream>
#include <mutex>

extern "C"
{
#include<libavcodec/avcodec.h>
}
struct AVCodecParameters;
struct AVCodecContext;
struct AVFrame;
struct AVPacket;

using namespace std;

// 解码类（视频和音频）
class XDecode
{
public:
    XDecode();
    virtual ~XDecode();

    bool open(AVCodecParameters *codecPara); // 打开解码器
    bool send(AVPacket *pkt); // 发送到解码线程
    AVFrame* recv(); // 获取解码数据
    void close(); // 关闭

    bool m_isAudio = false; // 是否为音频的标志位

private:
    AVCodecContext * m_VCodecCtx = 0; // 解码器
    std::mutex m_mutex; // 互斥锁
};

#endif // XDECODE_H
