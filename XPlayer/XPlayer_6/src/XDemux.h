#pragma once
#include <iostream>
#include <mutex>

struct AVFormatContext;
struct AVPacket;
struct AVCodecParameters;

class XDemux
{
public:
	XDemux();
	virtual ~XDemux();

	bool open(const char* url); // 打开媒体文件或者流媒体（rtsp、rtmp、http）
	AVPacket* read(); // 读取一帧AVPacket
	AVCodecParameters* copyVPara(); // 获取视频参数
	AVCodecParameters* copyAPara(); // 获取音频参数
	bool isAudio(AVPacket* pkt); // 是否为音频
	bool seek(double pos); // seek位置（pos 0.0~1.0）
	void clear(); // 清理
	void close(); // 关闭 
	AVPacket* readVideo(); // 只读视频，音频丢弃空间释放

	// 媒体总时长（毫秒）
	int totalMs = 0;
	int width = 0;
	int height = 0;
	int sampleRate = 0;
	int channels = 0;

protected:
	std::mutex m_mutex;
	
	AVFormatContext *pFormatCtx = NULL; // 解封装上下文
	// 音视频索引，读取时区分音视频
	int nVStreamIndex = 0;
	int nAStreamIndex = 1;
};

