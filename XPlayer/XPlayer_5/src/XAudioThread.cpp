#include "XAudioThread.h"

XAudioThread::XAudioThread()
{

}

XAudioThread::~XAudioThread()
{
	// 等待线程退出
	m_isExit = true;
	wait();
}

// 打开
bool XAudioThread::open(AVCodecParameters* para, int sampleRate, int channels)
{
	if (!para)return false;
	m_mutex.lock();
	if (!m_decode) m_decode = new XDecode();
	if (!m_resample) m_resample = new XResample();
	if (!m_audioPlay) m_audioPlay = XAudioPlay::Get();
	bool nRet = true;
	if (!m_resample->open(para, false))
	{
		cout << "XResample open failed!" << endl;
		nRet = false;
	}
	m_audioPlay->m_sampleRate = sampleRate;
	m_audioPlay->m_channels = channels;
	if (!m_audioPlay->open())
	{
		nRet = false;
		cout << "XAudioPlay open failed!" << endl;
	}
	if (!m_decode->open(para))
	{
		cout << "audio XDecode open failed!" << endl;
		nRet = false;
	}
	m_mutex.unlock();
	cout << "XAudioThread::Open :" << nRet << endl;
	return nRet;
}

// 将AVPacket加入到队列中等待解码转换
void XAudioThread::push(AVPacket *pkt)
{
	if (!pkt) return; 
	// 阻塞
	while (!m_isExit)
	{
		m_mutex.lock();
		if (m_packs.size() < m_maxList)
		{
			m_packs.push_back(pkt);
			m_mutex.unlock();
			break;
		}
		m_mutex.unlock();
		msleep(1);
	}
}

// 从队列中获取AVPacket进行解码
void XAudioThread::run()
{
	unsigned char *pcm = new unsigned char[1024 * 1024 * 10];
	while (!m_isExit)
	{
		m_mutex.lock();

		//没有数据
		if (m_packs.empty() || !m_decode || !m_resample || !m_audioPlay)
		{
			m_mutex.unlock();
			msleep(1);
			continue;
		}

		AVPacket *pkt = m_packs.front();
		m_packs.pop_front();
		bool nRet = m_decode->send(pkt);
		if (!nRet)
		{
			m_mutex.unlock();
			msleep(1);
			continue;
		}
		//一次send 多次recv
		while (!m_isExit)
		{
			AVFrame * frame = m_decode->recv();
			if (!frame) break;
			//重采样 
			int size = m_resample->resample(frame, pcm);
			//播放音频
			while (!m_isExit)
			{
				if (size <= 0) break;
				//缓冲未播完，空间不够
				if (m_audioPlay->getFree() < size)
				{
					msleep(1);
					continue;
				}
				m_audioPlay->write(pcm, size);
				break;
			}
		}
		m_mutex.unlock();
	}
	delete pcm;
}
