#include "XDemuxThread.h"

XDemuxThread::XDemuxThread()
{

}

XDemuxThread::~XDemuxThread()
{
	m_isExit = true;
	wait();
}

// 创建对象并打开
bool XDemuxThread::open(const char* url, IVideoCall* call)
{
	if (url == 0 || url[0] == '\0')
		return false;

	m_mutex.lock();
	if (!m_demux) m_demux = new XDemux();
	if (!m_vThread) m_vThread = new XVideoThread();
	if (!m_aThread) m_aThread = new XAudioThread();

	// 打开解封装
	bool nRet = m_demux->open(url);
	if (!nRet)
	{
		cout << "demux->Open(url) failed!" << endl;
		return false;
	}
	// 打开视频解码器和处理线程
	if (!m_vThread->open(m_demux->copyVPara(), call, m_demux->getVideoInfo().width, m_demux->getVideoInfo().height))
	{
		nRet = false;
		cout << "vt->Open failed!" << endl;
	}
	// 打开音频解码器和处理线程
	if (!m_aThread->open(m_demux->copyAPara(), m_demux->getVideoInfo().sampleRate, m_demux->getVideoInfo().channels))
	{
		nRet = false;
		cout << "at->Open failed!" << endl;
	}
	m_mutex.unlock();
	cout << "XDemuxThread::Open " << nRet << endl;
	return nRet;
}

void XDemuxThread::run()
{
	while (!m_isExit)
	{
		m_mutex.lock();
		if (!m_demux)
		{
			m_mutex.unlock();
			msleep(5);
			continue;
		}
		AVPacket *pkt = m_demux->read();
		if (!pkt)
		{
			m_mutex.unlock();
			msleep(5);
			continue;
		}
		// 判断数据是音频
		if (m_demux->isAudio(pkt))
		{
			if(m_aThread)m_aThread->push(pkt);
		}
		else // 视频
		{
			if (m_vThread)m_vThread->push(pkt);
		}

		m_mutex.unlock();
	}
}

// 启动所有线程
void XDemuxThread::start()
{
	m_mutex.lock();
	// 启动当前线程
	QThread::start();
	if (m_vThread)m_vThread->start();
	if (m_aThread)m_aThread->start();
	m_mutex.unlock();
}
