#pragma once
#include <iostream>
#include <QThread>
#include "IVideoCall.h"
#include <mutex>
#include "XDemux.h"
#include "XVideoThread.h"
#include "XAudioThread.h"

using namespace std;

class XDemux;
class XVideoThread;
class XAudioThread;

class XDemuxThread:public QThread
{
public:
	XDemuxThread();
	virtual ~XDemuxThread();	
	virtual bool open(const char *url, IVideoCall *call); // 创建对象并打开
	void run();
	virtual void start(); // 启动所有线程

	bool m_isExit = false;

protected:
	std::mutex m_mutex;
	XDemux *m_demux = 0;
	XVideoThread *m_vThread = 0;
	XAudioThread *m_aThread = 0;
};

