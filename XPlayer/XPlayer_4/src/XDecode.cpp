#include "XDecode.h"

XDecode::XDecode()
{

}

XDecode::~XDecode()
{

}

// 打开解码器
bool XDecode::Open(AVCodecParameters *codecPara)
{
    if (!codecPara) return false;
    Close();

    // 根据传入的para->codec_id找到解码器
    AVCodec *vcodec = avcodec_find_decoder(codecPara->codec_id);
    if (!vcodec)
    {
        avcodec_parameters_free(&codecPara);
        cout << "can't find the codec id " << codecPara->codec_id << endl;
        return false;
    }
    cout << "find the AVCodec " << codecPara->codec_id << endl;

    std::unique_lock<std::mutex> guard(m_mutex);
    // 创建解码器上下文
    m_VCodecCtx = avcodec_alloc_context3(vcodec);
    // 配置解码器上下文参数
    avcodec_parameters_to_context(m_VCodecCtx, codecPara);
    // 清空编码器参数，避免内存泄漏（很重要）
    avcodec_parameters_free(&codecPara);
    // 八线程解码
    m_VCodecCtx->thread_count = 8;

    // 打开解码器上下文
    int nRet = avcodec_open2(m_VCodecCtx, 0, 0);
    if (nRet != 0)
    {
        avcodec_free_context(&m_VCodecCtx); // 失败这里就释放申请内存，否则留到实际使用那里再释放
        char buf[1024] = { 0 };
        av_strerror(nRet, buf, sizeof(buf) - 1);
        cout << "avcodec_open2  failed! :" << buf << endl;
        return false;
    }
    cout << "avcodec_open2 success!" << endl;

    return true;
}

// 发送到解码线程（不管成功与否都释放pkt空间 对象和媒体内容）
bool XDecode::Send(AVPacket *pkt)
{
    // 容错处理
    if (!pkt || pkt->size <= 0 || !pkt->data)return false;
    std::unique_lock<std::mutex> guard(m_mutex);
    if (!m_VCodecCtx)
        return false;
    int nRet = avcodec_send_packet(m_VCodecCtx, pkt);

    // 无论成功与否，都清空AVPacket，避免内存泄漏（很重要）
    av_packet_free(&pkt);
    if (nRet != 0)
        return false;

    return true;
}

// 获取解码数据，一次send可能需要多次Recv，获取缓冲中的数据Send NULL在Recv多次
// 每次复制一份，由调用者释放 av_frame_free（如果是视频，接受的是YUV数据）
AVFrame* XDecode::Recv()
{
    std::unique_lock<std::mutex> guard(m_mutex);
    if (!m_VCodecCtx)
        return NULL;
    AVFrame *frame = av_frame_alloc();
    int nRet = avcodec_receive_frame(m_VCodecCtx, frame);
    if (nRet != 0)
    {
        av_frame_free(&frame); // 失败这里就释放申请内存，否则留到实际使用那里再释放
        return NULL;
    }
    cout << "["<<frame->linesize[0] << "] " << flush;
    return frame;
}

// 关闭
void XDecode::Close()
{
    std::unique_lock<std::mutex> guard(m_mutex);
    if (m_VCodecCtx)
    {
        avcodec_flush_buffers(m_VCodecCtx); // 清理解码器申请内存
        avcodec_close(m_VCodecCtx);
        avcodec_free_context(&m_VCodecCtx); // 关闭也要清理解码器申请内存
    }
}
