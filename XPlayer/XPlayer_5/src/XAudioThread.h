#pragma once
#include <iostream>
#include <mutex>
#include <list>
#include <QThread>
#include "XDecode.h"
#include "XAudioPlay.h"
#include "XResample.h"

using namespace std;

struct AVCodecParameters;
class XDecode;
class XAudioPlay;
class XResample;
class AVPacket;

class XAudioThread:public QThread
{
public:
	XAudioThread();
	virtual ~XAudioThread();
	
	virtual bool open(AVCodecParameters *para,int sampleRate,int channels); // 打开，不管成功与否都清理
	virtual void push(AVPacket *pkt); // 将AVPacket加入到队列中等待解码转换
	void run(); // 从队列中获取AVPacket进行解码
	
	// 最大队列
	int m_maxList = 100;
	bool m_isExit = false;

protected:
	std::list <AVPacket *> m_packs;
	std::mutex m_mutex;
	XDecode *m_decode = 0;
	XAudioPlay *m_audioPlay = 0;
	XResample *m_resample = 0;
};

