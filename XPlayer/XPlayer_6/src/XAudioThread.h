#pragma once
#include "XDecodeThread.h"

struct AVCodecParameters;
class XAudioPlay;
class XResample;


class XAudioThread:public XDecodeThread
{
public:
	//当前音频播放的pts
	long long pts = 0;
	//打开，不管成功与否都清理
	virtual bool Open(AVCodecParameters *para,int sampleRate,int channels);

	//停止线程，清理资源
	virtual void Close();

	virtual void Clear();
	void run();
	XAudioThread();
	virtual ~XAudioThread();
	void SetPause(bool isPause);
	bool isPause = false;
protected:
	std::mutex amux;
	XAudioPlay *ap = 0;
	XResample *res = 0;

};

