#pragma once
#include <iostream>
#include <list>
#include <mutex>
#include <QThread>
#include "XDecode.h"
#include "IVideoCall.h"

using namespace std;

// 解码和显示视频
struct AVPacket;
struct AVCodecParameters;
class XDecode;

class XVideoThread:public QThread
{
public:	
	virtual bool open(AVCodecParameters *para, IVideoCall *call, int width,int height); // 打开，不管成功与否都清理
	virtual void push(AVPacket *pkt); // 将AVPacket加入到队列中等待解码转换
	void run(); // 从队列中获取AVPacket进行解码

	XVideoThread();
	virtual ~XVideoThread();
	
	int maxList = 100; // 最大队列
	bool isExit = false; // 线程是否存在

protected:
	std::list <AVPacket *> m_pktList; // AVPacket队列
	std::mutex m_mutex;
	XDecode *decode = 0; // 解码器
	IVideoCall *call = 0;
};

