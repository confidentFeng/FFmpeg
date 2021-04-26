#include "XDemux.h"

using namespace std;
extern "C" {
	#include "libavformat/avformat.h"
}
#pragma comment(lib,"avformat.lib")
#pragma comment(lib,"avutil.lib")
#pragma comment(lib,"avcodec.lib")

static double r2d(AVRational r)
{
	return r.den == 0 ? 0 : (double)r.num / (double)r.den;
}

XDemux::XDemux()
{
	static bool isFirst = true;
	static std::mutex dmux;
	dmux.lock();
	if (isFirst)
	{
		// ��ʼ����װ��
		av_register_all();

		// ��ʼ������� �����Դ�rtsp rtmp http Э�����ý����Ƶ��
		avformat_network_init();
		isFirst = false;
	}
	dmux.unlock();
}

XDemux::~XDemux()
{

}

bool XDemux::open(const char *url)
{
	close();
	// ��������
	AVDictionary *opts = NULL;
	// ����rtsp����tcpЭ���
	av_dict_set(&opts, "rtsp_transport", "tcp", 0);

	// ������ʱʱ��
	av_dict_set(&opts, "max_delay", "500", 0);

	m_mutex.lock();
	int re = avformat_open_input(
		&pFormatCtx,
		url,
		0,  // 0��ʾ�Զ�ѡ������
		&opts // �������ã�����rtsp����ʱʱ��
	);
	if (re != 0)
	{
		m_mutex.unlock();
		char buf[1024] = { 0 };
		av_strerror(re, buf, sizeof(buf) - 1);
		cout << "open " << url << " failed! :" << buf << endl;
		return false;
	}
	cout << "open " << url << " success! " << endl;

	// ��ȡ����Ϣ 
	re = avformat_find_stream_info(pFormatCtx, 0);

	// ��ʱ�� ����
	this->totalMs = pFormatCtx->duration / (AV_TIME_BASE / 1000);
	cout << "totalMs = " << totalMs << endl;

	// ��ӡ��Ƶ����ϸ��Ϣ
	av_dump_format(pFormatCtx, 0, url, 0);

	// ��ȡ��Ƶ��
	nVStreamIndex = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
	AVStream *as = pFormatCtx->streams[nVStreamIndex];
	width = as->codecpar->width;
	height = as->codecpar->height;

	cout << "=======================================================" << endl;
	cout << nVStreamIndex << "��Ƶ��Ϣ" << endl;
	cout << "codec_id = " << as->codecpar->codec_id << endl;
	cout << "format = " << as->codecpar->format << endl;
	cout << "width=" << as->codecpar->width << endl;
	cout << "height=" << as->codecpar->height << endl;
	// ֡�� fps ����ת��
	cout << "video fps = " << r2d(as->avg_frame_rate) << endl;

	cout << "=======================================================" << endl;
	cout << nAStreamIndex << "��Ƶ��Ϣ" << endl;
	// ��ȡ��Ƶ��
	nAStreamIndex = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
	as = pFormatCtx->streams[nAStreamIndex];
	sampleRate = as->codecpar->sample_rate;
	channels = as->codecpar->channels;

	cout << "codec_id = " << as->codecpar->codec_id << endl;
	cout << "format = " << as->codecpar->format << endl;
	cout << "sample_rate = " << as->codecpar->sample_rate << endl;
	// AVSampleFormat;
	cout << "channels = " << as->codecpar->channels << endl;
	// һ֡���ݣ��� ��ͨ�������� 
	cout << "frame_size = " << as->codecpar->frame_size << endl;
	// 1024 * 2 * 2 = 4096  fps = sample_rate/frame_size
	m_mutex.unlock();

	return true;
}

// ֻ����Ƶ����Ƶ�����ռ��ͷ�
AVPacket* XDemux::readVideo()
{
	m_mutex.lock();
	if (!pFormatCtx) // �ݴ�
	{
		m_mutex.unlock();
		return 0;
	}
	m_mutex.unlock();

	AVPacket* pkt = NULL;
	// ��ֹ����
	for (int i = 0; i < 20; i++)
	{
		pkt = read();
		if (!pkt)break;
		if (pkt->stream_index == nVStreamIndex)
		{
			break;
		}
		av_packet_free(&pkt);
	}
	return pkt;
}

// seek λ�� pos 0.0 ~1.0
bool XDemux::seek(double pos)
{
	m_mutex.lock();
	if (!pFormatCtx)
	{
		m_mutex.unlock();
		return false;
	}
	// ������ȡ����
	avformat_flush(pFormatCtx);

	long long seekPos = 0;
	seekPos = pFormatCtx->streams[nVStreamIndex]->duration * pos;
	int re = av_seek_frame(pFormatCtx, nVStreamIndex, seekPos, AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_FRAME);
	m_mutex.unlock();
	if (re < 0) return false;
	return true;
}

// ��ȡ��Ƶ����  ���صĿռ���Ҫ����  avcodec_parameters_free
AVCodecParameters *XDemux::copyVPara()
{
	m_mutex.lock();
	if (!pFormatCtx)
	{
		m_mutex.unlock();
		return NULL;
	}
	AVCodecParameters *pa = avcodec_parameters_alloc();
	avcodec_parameters_copy(pa, pFormatCtx->streams[nVStreamIndex]->codecpar);
	m_mutex.unlock();
	return pa;
}

// ��ȡ��Ƶ����  ���صĿռ���Ҫ���� avcodec_parameters_free
AVCodecParameters *XDemux::copyAPara()
{
	m_mutex.lock();
	if (!pFormatCtx)
	{
		m_mutex.unlock();
		return NULL;
	}
	AVCodecParameters *pa = avcodec_parameters_alloc();
	avcodec_parameters_copy(pa, pFormatCtx->streams[nAStreamIndex]->codecpar);
	m_mutex.unlock();
	return pa;
}

bool XDemux::isAudio(AVPacket *pkt)
{
	if (!pkt) return false;
	if (pkt->stream_index == nVStreamIndex)
		return false;
	return true;
}

// �ռ���Ҫ�������ͷ� ���ͷ�AVPacket����ռ䣬�����ݿռ� av_packet_free
AVPacket *XDemux::read()
{
	m_mutex.lock();
	if (!pFormatCtx) // �ݴ�
	{
		m_mutex.unlock();
		return 0;
	}
	AVPacket *pkt = av_packet_alloc();
	// ��ȡһ֡��������ռ�
	int re = av_read_frame(pFormatCtx, pkt);
	if (re != 0)
	{
		m_mutex.unlock();
		av_packet_free(&pkt);
		return 0;
	}
	// ptsת��Ϊ����
	pkt->pts = pkt->pts*(1000 * (r2d(pFormatCtx->streams[pkt->stream_index]->time_base)));
	pkt->dts = pkt->dts*(1000 * (r2d(pFormatCtx->streams[pkt->stream_index]->time_base)));
	m_mutex.unlock();
	//cout << pkt->pts << " "<<flush;
	return pkt;
}

// ��ն�ȡ����
void XDemux::clear()
{
	m_mutex.lock();
	if (!pFormatCtx)
	{
		m_mutex.unlock();
		return;
	}

	m_mutex.unlock();
}

void XDemux::close()
{
	m_mutex.lock();
	if (!pFormatCtx)
	{
		m_mutex.unlock();
		return;
	}
	// ������ȡ����
	avformat_flush(pFormatCtx);
	avformat_close_input(&pFormatCtx);
	// ý����ʱ�������룩
	totalMs = 0;
	m_mutex.unlock();
}