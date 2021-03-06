#include "XFFmpeg.h"


XFFmpeg::XFFmpeg()
{

}

// 打开视频文件（本质是解封装）
bool XFFmpeg::openVideo(const char *filepath)
{
    // 打开视频文件，先上锁
    m_mutex.lock();
    // 分配一个AVFormatContext（解封装上下文），FFmpeg所有的操作都要通过这个AVFormatContext对象来进行
    m_pFormatCtx = avformat_alloc_context();
    // 打开视频文件，并获得解封装上下文，即pFormatCtx
    if (avformat_open_input(&m_pFormatCtx, filepath, nullptr, nullptr) != 0)
    {
        printf("Couldn't open input stream.\n");
        m_mutex.unlock();
        return false;
    }
    // 进一步获取视频流信息存储在pFormatCtx中
    if (avformat_find_stream_info(m_pFormatCtx, nullptr) < 0)
    {
        printf("Couldn't find stream information.\n");
        m_mutex.unlock();
        return false;
    }
    m_mutex.unlock();

    return true;
}

// 查找视频流
bool XFFmpeg::findVideoStream()
{
    m_mutex.lock();
    for (int i = 0; i < (int)m_pFormatCtx->nb_streams; i++)
    {
        // 判断是否为视频流，是则退出循环
        if (m_pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            m_nVideoStream = i;
            break;
        }
    }
    // 如果videoStream为-1，说明没有找到视频流
    if (m_nVideoStream == -1)
    {
        printf("Didn't find a video stream.\n");
        m_mutex.unlock();
        return false;
    }
    m_mutex.unlock();

    return true;
}

// 打开视频解码器
bool XFFmpeg::openVCodec()
{
    m_mutex.lock();
    // 获取视频解码上下文
    m_pCodecCtx = m_pFormatCtx->streams[m_nVideoStream]->codec;
    // 进一步查找视频解码器
    AVCodec *pCodec = avcodec_find_decoder(m_pCodecCtx->codec_id);
    if (pCodec == nullptr)
    {
        printf("Codec not found.\n");
        m_mutex.unlock();
        return false;
    }
    // 打开解码器（具体采用什么解码器ffmpeg经过封装，无需知道）
    if (avcodec_open2(m_pCodecCtx, pCodec, nullptr) < 0)
    {
        printf("Could not open codec.\n");
        m_mutex.unlock();
        return false;
    }
    m_mutex.unlock();

    return true;
}

// 分配一帧图像和AVFrame等所需的空间
bool XFFmpeg::allocFrame()
{
    m_mutex.lock();
    // 获取一帧图像需要的大小
    int numBytes = (size_t)av_image_get_buffer_size(AV_PIX_FMT_RGB32, m_pCodecCtx->width, m_pCodecCtx->height, 1);
    // 创建存储一帧图像数据的空间
    unsigned char *out_buffer = (unsigned char *)av_malloc(numBytes * sizeof(unsigned char));

    // 创建帧结构，此函数仅分配基本结构空间，图像数据空间需通过av_malloc分配
    m_pFrameYUV = av_frame_alloc();
    m_pFrameRGB = av_frame_alloc();
    // 会将pFrameRGB的数据按RGB格式自动"关联"到out_buffer，即pFrameRGB中的数据改变了则out_buffer中的数据也会相应的改变
    av_image_fill_arrays(m_pFrameRGB->data, m_pFrameRGB->linesize, out_buffer,
                         AV_PIX_FMT_RGB32, m_pCodecCtx->width, m_pCodecCtx->height, 1);

    // 设置YUV转RGB的数据转换参数，作为sws_scale的第一个参数
    m_img_convert_ctx = sws_getContext(m_pCodecCtx->width, m_pCodecCtx->height, m_pCodecCtx->pix_fmt, // 源地址长宽以及数据格式
                                                        m_pCodecCtx->width, m_pCodecCtx->height, AV_PIX_FMT_RGB32, // 目的地址长宽以及数据格式
                                                        SWS_BICUBIC, nullptr, nullptr, nullptr); // 算法类型：AV_PIX_FMT_YUVJ420P 转 AV_PIX_FMT_RGB32
    m_mutex.unlock();

    return true;
}

// 解码
int XFFmpeg::decode(AVPacket *packet, QImage &tmpImage)
{
    m_mutex.lock();

    // 读取一帧解封装后的数据，存入到packet中
    int ret = av_read_frame(m_pFormatCtx, packet);
    if (ret != 0) {
        m_mutex.unlock();
        return -1;
    }

    // 是视频数据才继续解码
    if (packet->stream_index != m_nVideoStream) {
       m_mutex.unlock();
       return -2;
    }

    // 视频解码，解码之后的数据存储在pFrameYUV中（YUV数据）
    int got_picture = 0;
    ret = avcodec_decode_video2(m_pCodecCtx, m_pFrameYUV, &got_picture, packet);
    if (ret < 0)
    {
        printf("Decode Error.\n");
        m_mutex.unlock();
        return -1;
    }
    // 返回标志位，如果解码有数据则got_picture=1
    if (got_picture)
    {
        sws_scale(m_img_convert_ctx, (const unsigned char* const*)m_pFrameYUV->data, m_pFrameYUV->linesize, 0, m_pCodecCtx->height,
                  m_pFrameRGB->data, m_pFrameRGB->linesize);

        // 将RGB转换成QPixmap，每40ms显示一帧图片
        QImage image((uchar*)m_pFrameRGB->data[0], m_pCodecCtx->width, m_pCodecCtx->height, QImage::Format_RGB32);
        tmpImage = image.copy();
    }
    m_mutex.unlock();

    return 0;
}

void XFFmpeg::Close()
{
    m_mutex.lock(); // 需要上锁，以防多线程中你这里在close，另一个线程中在读取，
    sws_freeContext(m_img_convert_ctx);
    av_frame_free(&m_pFrameRGB);
    av_frame_free(&m_pFrameYUV);
    avcodec_close(m_pCodecCtx);
    avformat_close_input(&m_pFormatCtx);
    m_mutex.unlock();
}
