#pragma once
#include "XDecodeThread.h"

struct AVCodecParameters;
class XAudioPlay;
class XResample;


class XAudioThread:public XDecodeThread
{
public:
	//��ǰ��Ƶ���ŵ�pts
	long long pts = 0;
	//�򿪣����ܳɹ��������
	virtual bool Open(AVCodecParameters *para,int sampleRate,int channels);

	//ֹͣ�̣߳�������Դ
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
