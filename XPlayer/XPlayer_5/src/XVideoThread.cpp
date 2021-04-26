#include "XVideoThread.h"

XVideoThread::XVideoThread()
{

}


XVideoThread::~XVideoThread()
{
	// 等待线程退出
	isExit = true;
	wait();
}

// 打开，不管成功与否都清理
bool XVideoThread::open(AVCodecParameters *para, IVideoCall *call,int width,int height)
{
	if (!para)return false;
	m_mutex.lock();

	// 初始化显示窗口
	this->call = call;
	if (call)
	{
		call->init(width, height);
	}

	// 打开解码器
	if (!decode) decode = new XDecode();
	int nRet = true;
	if (!decode->open(para))
	{
		cout << "audio XDecode open failed!" << endl;
		nRet = false;
	}
	m_mutex.unlock();
	cout << "XAudioThread::Open :" << nRet << endl;
	return nRet;
}

// 将AVPacket加入到队列中等待解码转换
void XVideoThread::push(AVPacket *pkt)
{
	if (!pkt) return;

	// 阻塞
	while (!isExit)
	{
		m_mutex.lock();
		if (m_pktList.size() < maxList)
		{
			m_pktList.push_back(pkt);
			m_mutex.unlock();
			break;
		}
		m_mutex.unlock();
		msleep(1);
	}
}

// 从队列中获取AVPacket进行解码
void XVideoThread::run()
{
	while (!isExit)
	{
		m_mutex.lock();

		// 没有数据
		if (m_pktList.empty() || !decode)
		{
			m_mutex.unlock();
			msleep(1);
			continue;
		}

		AVPacket *pkt = m_pktList.front();
		m_pktList.pop_front();
		bool nRet = decode->send(pkt);
		if (!nRet)
		{
			m_mutex.unlock();
			msleep(1);
			continue;
		}

		// 一次send 多次recv
		while (!isExit)
		{
			AVFrame * frame = decode->recv();
			if (!frame) break;
			// 显示视频
			if (call)
			{
				call->myRepaint(frame);
			}
		}
		m_mutex.unlock();
	}
}