#include "XDemux.h"

// 确保time_base的分母不为0
static double r2d(AVRational r)
{
    return r.den == 0 ? 0 : (double)r.num / (double)r.den;
}

XDemux::XDemux()
{
    std::unique_lock<std::mutex> guard(m_mutex); // 加上锁，避免多线程同时初始化导致错误
    if(m_isFirst) {
		// 注册FFmpeg的所有组件
		av_register_all();
        //初始化网络库 （可以打开rtsp rtmp http 协议的流媒体视频）
        avformat_network_init();
        m_isFirst = false;
    }
}

XDemux::~XDemux()
{

}

// 打开媒体文件或者流媒体（rtsp、rtmp、http）
bool XDemux::open(const char *url)
{
    close();
    // 参数设置
    AVDictionary *opts = NULL;
    av_dict_set(&opts, "rtsp_transport", "tcp", 0); // 设置rtsp流以tcp协议打开
    av_dict_set(&opts, "max_delay", "500", 0); // 设置网络延时时间

    // 1、打开媒体文件
    std::unique_lock<std::mutex> guard(m_mutex);
    int nRet = avformat_open_input(
        &pFormatCtx,
        url,
        nullptr,  // nullptr表示自动选择解封器
        &opts // 参数设置
    );
    if (nRet != 0)
    {
        char errBuf[1024] = { 0 };
        av_strerror(nRet, errBuf, sizeof(errBuf));
        cout << "open " << url << " failed! :" << errBuf << endl;
        return false;
    }
    cout << "open " << url << " success! " << endl;

    // 2、探测获取流信息
    nRet = avformat_find_stream_info(pFormatCtx, 0);
    if (nRet < 0) {
        char errBuf[1024] = { 0 };
        av_strerror(nRet, errBuf, sizeof(errBuf));
        cout << "open " << url << " failed! :" << errBuf << endl;
        return false;
    }

    // 获取媒体总时长，单位为毫秒
    m_stVideoInfo.totalMs = pFormatCtx->duration / (AV_TIME_BASE / 1000);
    cout << "totalMs = " << m_stVideoInfo.totalMs << endl;
    // 打印视频流详细信息
    av_dump_format(pFormatCtx, 0, url, 0);

    // 3、获取视频流索引
    nVStreamIndex = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    if (nVStreamIndex == -1) {
        cout << "find videoStream failed!" << endl;
        return false;
    }
    // 打印视频信息（这个pStream只是指向pFormatCtx的成员，未申请内存，为栈指针无需释放，下面同理）
    AVStream *pVStream = pFormatCtx->streams[nVStreamIndex];
    cout << "=======================================================" << endl;
    cout << "VideoInfo: " << nVStreamIndex << endl;
    cout << "codec_id = " << pVStream->codecpar->codec_id << endl;
    cout << "format = " << pVStream->codecpar->format << endl;
    cout << "width=" << pVStream->codecpar->width << endl;
    cout << "height=" << pVStream->codecpar->height << endl;
    // 帧率 fps 分数转换
    cout << "video fps = " << r2d(pVStream->avg_frame_rate) << endl;
	// 获取输入视频的宽高
    m_stVideoInfo.width = pVStream->codecpar->width;
    m_stVideoInfo.height = pVStream->codecpar->height;

    // 4、获取音频流索引
    nAStreamIndex = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
    if (nVStreamIndex == -1) {
        cout << "find audioStream failed!" << endl;
        return false;
    }
    // 打印音频信息
    AVStream *pAStream = pFormatCtx->streams[nAStreamIndex];
    cout << "=======================================================" << endl;
    cout << "AudioInfo: " << nAStreamIndex  << endl;
    cout << "codec_id = " << pAStream->codecpar->codec_id << endl;
    cout << "format = " << pAStream->codecpar->format << endl;
    cout << "sample_rate = " << pAStream->codecpar->sample_rate << endl;
    // AVSampleFormat;
    cout << "channels = " << pAStream->codecpar->channels << endl;
    // 一帧数据？？ 单通道样本数
    cout << "frame_size = " << pAStream->codecpar->frame_size << endl;

    return true;
}

// 读取一帧AVPacket（由于返回值指针申请了内存，函数内未释放，
// 所以到调用时要记得释放，否则多次调用会造成内存泄漏，下面函数同理）
AVPacket *XDemux::read()
{
    std::unique_lock<std::mutex> guard(m_mutex);

    // 容错处理，确保即使视频未打开也不会崩溃
    if (!pFormatCtx)
        return nullptr;

    // 读取一帧，并分配空间
    AVPacket *pkt = av_packet_alloc();
    int nRet = av_read_frame(pFormatCtx, pkt);
    if (nRet != 0) // 读取错误，或者帧读取完了
    {
        av_packet_free(&pkt);
        return nullptr;
    }
    // pts转换为毫秒
    pkt->pts = pkt->pts*((r2d(pFormatCtx->streams[pkt->stream_index]->time_base) * 1000));
    pkt->dts = pkt->dts*((r2d(pFormatCtx->streams[pkt->stream_index]->time_base) * 1000));
    cout << pkt->pts << " "<<flush;

    return pkt;
}

// 获取视频参数
// 为什么不直接返回AVCodecParameters，而是间接拷贝，是为了避免多线程时一个线程调用open后close，
// 另一个线程再去调用open()中的AVCodecParameters容易出错，获取音频参数同理
AVCodecParameters *XDemux::copyVPara()
{
    std::unique_lock<std::mutex> guard(m_mutex);
    if (!pFormatCtx)
        return nullptr;
    // 拷贝视频参数
    AVCodecParameters *pCodecPara = avcodec_parameters_alloc();
    avcodec_parameters_copy(pCodecPara, pFormatCtx->streams[nVStreamIndex]->codecpar);

    return pCodecPara;
}

// 获取音频参数
AVCodecParameters *XDemux::copyAPara()
{
    std::unique_lock<std::mutex> guard(m_mutex);
    if (!pFormatCtx)
        return nullptr;
    // 拷贝音频参数
    AVCodecParameters *pCodecPara = avcodec_parameters_alloc();
    avcodec_parameters_copy(pCodecPara, pFormatCtx->streams[nAStreamIndex]->codecpar);

    return pCodecPara;
}

// 是否为音频
bool XDemux::isAudio(AVPacket *pkt)
{
    if (!pkt) return false;
    if (pkt->stream_index == nVStreamIndex)
        return false;

    return true;
}

// seek位置（pos 0.0~1.0）
bool XDemux::seek(double pos)
{
    std::unique_lock<std::mutex> guard(m_mutex);
    if (!pFormatCtx)
        return false;
    // 清理先前未滑动时解码到的视频帧
    avformat_flush(pFormatCtx);

    long long seekPos = seekPos = pFormatCtx->streams[nVStreamIndex]->duration * pos; // 计算要移动到的位置
    int nRet = av_seek_frame(pFormatCtx, nVStreamIndex, seekPos, AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_FRAME);
    if (nRet < 0)
        return false;

    return true;
}

// 关闭
void XDemux::close()
{
    std::unique_lock<std::mutex> guard(m_mutex);
    if (!pFormatCtx)
        return;
    // 释放解封装上下文申请空间
    avformat_flush(pFormatCtx);
    // 关闭解封装上下文
    avformat_close_input(&pFormatCtx);
    // 重新初始化媒体总时长（毫秒）
    m_stVideoInfo.totalMs = 0;
}

// 获取视频信息
stVideoInfo XDemux::getVideoInfo()
{
    return m_stVideoInfo;
}
