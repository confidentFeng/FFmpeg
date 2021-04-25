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

    virtual bool Open(AVCodecParameters *codecPara); // 打开解码器
    virtual bool Send(AVPacket *pkt); // 发送到解码线程
    virtual AVFrame* Recv(); // 获取解码数据
    virtual void Close(); // 关闭

    bool m_isAudio = false; // 是否为音频的标志位

private:
    AVCodecContext * m_VCodecCtx = 0; // 解码器上下文
    std::mutex m_mutex; // 互斥锁
};

#endif // XDECODE_H
