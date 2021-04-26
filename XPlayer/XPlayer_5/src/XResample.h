#pragma once

#include <iostream>
#include <mutex>

extern "C" {
#include <libswresample/swresample.h>
#include <libavcodec/avcodec.h>
}
#pragma comment(lib,"swresample.lib")

using namespace std;

struct AVCodecParameters;
struct AVFrame;
struct SwrContext;

class XResample
{
public:
	XResample();
	~XResample();

	// 输出参数和输入参数一致除了采样格式，输出为S16 ,会释放para
	virtual bool open(AVCodecParameters *para,bool isClearPara = false); // 打开
	virtual void close(); // 关闭

	// 返回重采样后大小,不管成功与否都释放indata空间
	virtual int resample(AVFrame *indata, unsigned char *data);
	
	int outFormat = 1; // 输出格式：AV_SAMPLE_FMT_S16

protected:
	std::mutex m_mutex; // 互斥锁
	SwrContext *actx = 0; // 上下文
};

