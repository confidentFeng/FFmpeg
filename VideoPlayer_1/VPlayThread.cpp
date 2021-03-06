#include "VPlayThread.h"

VPlayThread::VPlayThread()
{

}

void VPlayThread::run()
{
    // 注册FFmpeg的所有组件（4.0版本后被弃用）
    //av_register_all();

    /*** 1.打开视频文件（本质是解封装） ***/
    char filepath[] = "../VideoPlayer_1/test.mp4";
    // 分配一个AVFormatContext（解封装上下文），FFmpeg所有的操作都要通过这个AVFormatContext对象来进行
    AVFormatContext *pFormatCtx = avformat_alloc_context();
    // 打开视频文件，并获得解封装上下文，即pFormatCtx
    if (avformat_open_input(&pFormatCtx, filepath, nullptr, nullptr) != 0)
    {
        printf("Couldn't open input stream.\n");
        return;
    }
    // 进一步获取视频流信息存储在pFormatCtx中
    if (avformat_find_stream_info(pFormatCtx, nullptr) < 0)
    {
        printf("Couldn't find stream information.\n");
        return;
    }

    /*** 2.查找视频流 ***/
    int videoStream = -1; // 视频流索引，初始化为-1
    for (int i = 0; i < (int)pFormatCtx->nb_streams; i++)
    {
        // 判断是否为视频流，是则退出循环
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) //
        {
            videoStream = i;
            break;
        }
    }
    // 如果videoStream为-1，说明没有找到视频流
    if (videoStream == -1)
    {
        printf("Didn't find a video stream.\n");
        return ;
    }

    /*** 3.查找视频解码器并打开 ***/
    // 获取视频流解码上下文
    AVCodecContext *pCodecCtx = pFormatCtx->streams[videoStream]->codec;
    // 进一步查找视频解码器
    AVCodec *pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
    if (pCodec == nullptr)
    {
        printf("Codec not found.\n");
        return ;
    }
    // 打开解码器（具体采用什么解码器ffmpeg经过封装，无需知道）
    if (avcodec_open2(pCodecCtx, pCodec, nullptr) < 0)
    {
        printf("Could not open codec.\n");
        return ;
    }

    /*** 4.分配一帧图像和AVFrame等所需的空间 ***/
    // 获取一帧图像需要的大小
    int numBytes = (size_t)av_image_get_buffer_size(AV_PIX_FMT_RGB32, pCodecCtx->width, pCodecCtx->height, 1);
    // 创建存储一帧图像数据的空间
    unsigned char *out_buffer = (unsigned char *)av_malloc(numBytes * sizeof(unsigned char));

    // 创建帧结构，此函数仅分配基本结构空间，图像数据空间需通过av_malloc分配
    AVFrame *pFrameYUV = av_frame_alloc();
    AVFrame *pFrameRGB = av_frame_alloc();
    // 会将pFrameRGB的数据按RGB格式自动"关联"到out_buffer，即pFrameRGB中的数据改变了则out_buffer中的数据也会相应的改变
    av_image_fill_arrays(pFrameRGB->data, pFrameRGB->linesize, out_buffer,
                         AV_PIX_FMT_RGB32, pCodecCtx->width, pCodecCtx->height, 1);

    /*** 5.解码 ***/
    // 设置YUV转RGB的数据转换参数，作为sws_scale的第一个参数
    struct SwsContext *img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, // 源地址长宽以及数据格式
                                                        pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_RGB32, // 目的地址长宽以及数据格式
                                                        SWS_BICUBIC, nullptr, nullptr, nullptr); // 算法类型：AV_PIX_FMT_YUVJ420P 转 AV_PIX_FMT_RGB32
    // 分配AVPacket结构体
    AVPacket *packet = (AVPacket *)av_malloc(sizeof(AVPacket));
    while (1)
    {
        // 读取一帧解封装后的数据，存入到packet中
        int ret = av_read_frame(pFormatCtx, packet);
        if (ret != 0) break;

        // 是视频数据才继续解码
        if (packet->stream_index != videoStream)
            continue;

        // 视频解码，解码之后的数据存储在pFrameYUV中（YUV数据）
        int got_picture = 0;
        ret = avcodec_decode_video2(pCodecCtx, pFrameYUV, &got_picture, packet);
        if (ret < 0)
        {
            printf("Decode Error.\n");
            return ;
        }
        // 返回标志位，如果解码有数据则got_picture=1
        if (got_picture)
        {
            /*** 6.YUV->RGB ***/
            sws_scale(img_convert_ctx, (const unsigned char* const*)pFrameYUV->data, pFrameYUV->linesize, 0, pCodecCtx->height,
                      pFrameRGB->data, pFrameRGB->linesize);

            // 将RGB转换成QPixmap，每40ms在Label显示一帧图片
            QImage image((uchar*)pFrameRGB->data[0],pCodecCtx->width,pCodecCtx->height,QImage::Format_RGB32);
            // 发射更新图片信号，传递image给界面显示
            emit sig_GetOneFrame(image);

            // 打印它的pts,用来显示的时间戳，后面用来做同步
            //printf("pts = %d", packet->pts);
            // 设置每延时40ms读取一帧图片，达到25帧播放视频的效果（25帧：1000/25=40）
            QTime timer = QTime::currentTime().addMSecs(40);
            while( QTime::currentTime() < timer);
        }

        // 每读取一次视频帧，就要清空一次packet
        av_free_packet(packet);
    }

    // 打印视频的总时长
    int totalSec = pFormatCtx->duration / (AV_TIME_BASE);
    printf("file totalSec is %d:%d\n", totalSec / 60, totalSec % 60); // 以分秒计时
    // 打印视频文件信息：长度 比特率 流格式等
    printf("--------------- File Information ----------------\n");
    av_dump_format(pFormatCtx, 0, filepath, 0);
    printf("-------------------------------------------------\n");

    /*** 7.最后要释放申请的内存空间 ***/
    avformat_close_input(&pFormatCtx);
    avcodec_close(pCodecCtx);
    sws_freeContext(img_convert_ctx);
    av_frame_free(&pFrameYUV);
    av_frame_free(&pFrameRGB);
}
