#include <iostream>
#include <fstream>

extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
}
// 传统安装方法需要
#pragma comment(lib,"avformat.lib")
#pragma comment(lib,"avutil.lib")
#pragma comment(lib,"avcodec.lib")
#pragma comment(lib,"swscale.lib")
#pragma comment(lib,"swresample.lib")

using namespace std;

static double r2d(AVRational r)
{
	return r.den == 0 ? 0 : (double)r.num / (double)r.den;
}

int main(int argc, char* argv[])
{
	// 打开rgb文件
	FILE* outFileRgb = fopen("../bin/win64/dove_640x360.rgb", "wb");
	if (outFileRgb == NULL) {
		cout << "file not exist!" << endl;
		return false;
	}
	// 打开pcm文件
	FILE* outFilePcm = fopen("../bin/win64/dove.pcm", "wb");	
	if (outFilePcm == NULL) {
		cout << "file not exist!" << endl;
		return false;
	}

	// 初始化网络库 （可以打开rtsp rtmp http 协议的流媒体视频）
	avformat_network_init();

	//===================1、打开视频文件===================
	const char* path = "dove_640x360.mp4";
	// 参数设置
	AVDictionary* opts = NULL;
	// 设置rtsp流已tcp协议打开
	av_dict_set(&opts, "rtsp_transport", "tcp", 0);
	// 网络延时时间
	av_dict_set(&opts, "max_delay", "500", 0);

	// 解封装上下文
	AVFormatContext* pFormatCtx = NULL;
	int nRet = avformat_open_input(
		&pFormatCtx,
		path,
		0,  // 0表示自动选择解封器
		&opts // 参数设置，比如rtsp的延时时间
	);
	if (nRet != 0)
	{
		char buf[1024] = { 0 };
		av_strerror(nRet, buf, sizeof(buf) - 1);
		cout << "open " << path << " failed! :" << buf << endl;
		return -1;
	}
	cout << "open " << path << " success! " << endl;

	// 探测获取流信息
	nRet = avformat_find_stream_info(pFormatCtx, 0);

	// 获取媒体总时长，单位为毫秒
	int totalMs = static_cast<int>(pFormatCtx->duration / (AV_TIME_BASE / 1000));
	cout << "totalMs = " << totalMs << endl;
	// 打印视频流详细信息
	av_dump_format(pFormatCtx, 0, path, 0);

	//===================2、获取音视频流索引===================
	int nVStreamIndex = -1; // 视频流索引（读取时用来区分音视频）
	int nAStreamIndex = -1; // 音频流索引
	// 获取视频流索引（新版本方法：使用av_find_best_stream函数）	
	nVStreamIndex = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
	if (nVStreamIndex == -1) {
		cout << "find videoStream failed!" << endl;
		return -1;
	}
	// 打印视频信息（这个pStream只是指向pFormatCtx的成员，未申请内存，为栈指针无需释放，下面同理）
	AVStream* pVStream = pFormatCtx->streams[nVStreamIndex];
	cout << "=======================================================" << endl;
	cout << "VideoInfo: " << nVStreamIndex << endl;
	cout << "codec_id = " << pVStream->codecpar->codec_id << endl;
	cout << "format = " << pVStream->codecpar->format << endl;
	cout << "width=" << pVStream->codecpar->width << endl;
	cout << "height=" << pVStream->codecpar->height << endl;
	// 帧率 fps 分数转换
	cout << "video fps = " << r2d(pVStream->avg_frame_rate) << endl;
	// 帧率 fps 分数转换
	cout << "video fps = " << r2d(pFormatCtx->streams[nVStreamIndex]->avg_frame_rate) << endl;

	// 获取音频流索引
	nAStreamIndex = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
	if (nVStreamIndex == -1) {
		cout << "find audioStream failed!" << endl;
		return -1;
	}
	// 打印音频信息
	AVStream* pAStream = pFormatCtx->streams[nAStreamIndex];
	cout << "=======================================================" << endl;
	cout << "AudioInfo: " << nAStreamIndex << endl;
	cout << "codec_id = " << pAStream->codecpar->codec_id << endl;
	cout << "format = " << pAStream->codecpar->format << endl;
	cout << "sample_rate = " << pAStream->codecpar->sample_rate << endl;
	// AVSampleFormat;
	cout << "channels = " << pAStream->codecpar->channels << endl;
	// 一帧数据？？ 单通道样本数
	cout << "frame_size = " << pAStream->codecpar->frame_size << endl;

	//===================3、打开视频解码器===================
	// 根据codec_id找到视频解码器
	AVCodec* pVCodec = avcodec_find_decoder(pVStream->codecpar->codec_id);
	if (!pVCodec)
	{
		cout << "can't find the codec id " << pVStream->codecpar->codec_id;
		return -1;
	}
	cout << "find the AVCodec " << pVStream->codecpar->codec_id << endl;

	// 创建视频解码器上下文
	AVCodecContext* pVCodecCtx = avcodec_alloc_context3(pVCodec);
	// 配置视频解码器上下文参数
	avcodec_parameters_to_context(pVCodecCtx, pVStream->codecpar);
	// 八线程视频解码
	pVCodecCtx->thread_count = 8;

	// 打开视频解码器上下文
	nRet = avcodec_open2(pVCodecCtx, 0, 0);
	if (nRet != 0)
	{
		char buf[1024] = { 0 };
		av_strerror(nRet, buf, sizeof(buf) - 1);
		cout << "avcodec_open2  failed! :" << buf << endl;
		return -1;
	}
	cout << "video avcodec_open2 success!" << endl;

	//===================3、打开音频解码器===================
	// 找到音频解码器
	AVCodec* pACodec = avcodec_find_decoder(pFormatCtx->streams[nAStreamIndex]->codecpar->codec_id);
	if (!pACodec)
	{
		cout << "can't find the codec id " << pFormatCtx->streams[nAStreamIndex]->codecpar->codec_id;
		return -1;
	}
	cout << "find the AVCodec " << pFormatCtx->streams[nAStreamIndex]->codecpar->codec_id << endl;

	// 创建音频解码器上下文
	AVCodecContext* pACodecCtx = avcodec_alloc_context3(pACodec);
	// /配置音频解码器上下文参数
	avcodec_parameters_to_context(pACodecCtx, pFormatCtx->streams[nAStreamIndex]->codecpar);
	// 八线程音频解码
	pACodecCtx->thread_count = 8;

	// 打开音频解码器上下文
	nRet = avcodec_open2(pACodecCtx, 0, 0);
	if (nRet != 0)
	{
		char buf[1024] = { 0 };
		av_strerror(nRet, buf, sizeof(buf) - 1);
		cout << "avcodec_open2  failed! :" << buf << endl;
		return -1;
	}
	cout << "audio avcodec_open2 success!" << endl;

	//===================4、循环解码前初始化各缓冲区===================
	// malloc AVPacket并初始化
	AVPacket* pkt = av_packet_alloc();
	AVFrame* frame = av_frame_alloc();

	// 像素格式和尺寸转换上下文
	SwsContext* vSwsCtx = NULL;
	unsigned char* rgb = NULL;

	// 音频重采样 上下文初始化
	SwrContext* actx = swr_alloc();
	actx = swr_alloc_set_opts(actx,
		av_get_default_channel_layout(2),	// 输出格式
		AV_SAMPLE_FMT_S16,					// 输出样本格式
		pACodecCtx->sample_rate,			// 输出采样率
		av_get_default_channel_layout(pACodecCtx->channels), // 输入格式
		pACodecCtx->sample_fmt,
		pACodecCtx->sample_rate,
		0, 0
	);
	// 初始化音频采样数据上下文
	nRet = swr_init(actx);
	if (nRet != 0)
	{
		char buf[1024] = { 0 };
		av_strerror(nRet, buf, sizeof(buf) - 1);
		cout << "swr_init  failed! :" << buf << endl;
		return -1;
	}
	unsigned char* pcm = NULL;
	// 缓冲区大小 = 采样率(44100HZ) * 采样精度(16位 = 2字节)
	int MAX_AUDIO_SIZE = 44100 * 2;
	uint8_t* out_audio = (uint8_t*)av_malloc(MAX_AUDIO_SIZE);;
	// 获取输出的声道个数
	int out_nb_channels = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);

	//===================5、开始循环解码===================
	while(1)
	{
		int nRet = av_read_frame(pFormatCtx, pkt);
		if (nRet != 0)
		{
#if 0
			// 循环"播放"
			cout << "==============================end==============================" << endl;
			int ms = 3000; // 三秒位置 根据时间基数（分数）转换
			long long pos = (double)ms / (double)1000 * r2d(ic->streams[pkt->stream_index]->time_base);
			av_seek_frame(ic, nVStreamIndex, pos, AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_FRAME);
			continue;
#else
			// "播放"完一次之后退出
			break;
#endif
		}
		cout << "pkt->size = " << pkt->size << endl;
		// 显示的时间
		cout << "pkt->pts = " << pkt->pts << endl;
		// 转换为毫秒，方便做同步
		cout << "pkt->pts ms = " << pkt->pts * (r2d(pFormatCtx->streams[pkt->stream_index]->time_base) * 1000) << endl;
		// 解码时间
		cout << "pkt->dts = " << pkt->dts << endl;

		AVCodecContext* pCodecCtx = 0;
		if (pkt->stream_index == nVStreamIndex)
		{
			cout << "图像" << endl;
			pCodecCtx = pVCodecCtx;
		}
		if (pkt->stream_index == nAStreamIndex)
		{
			cout << "音频" << endl;
			pCodecCtx = pACodecCtx;
		}

		// 解码视频
		// 发送packet到解码线程  send传NULL后调用多次receive取出所有缓冲帧
		nRet = avcodec_send_packet(pCodecCtx, pkt);
		// 释放，引用计数-1 为0释放空间
		av_packet_unref(pkt);

		if (nRet != 0)
		{
			char buf[1024] = { 0 };
			av_strerror(nRet, buf, sizeof(buf) - 1);
			cout << "avcodec_send_packet  failed! :" << buf << endl;
			continue;
		}

		for (;;)
		{
			// 从线程中获取解码接口,一次send可能对应多次receive
			nRet = avcodec_receive_frame(pCodecCtx, frame);
			if (nRet != 0) break;
			cout << "recv frame " << frame->format << " " << frame->linesize[0] << endl;

			// 视频
			if (pCodecCtx == pVCodecCtx)
			{
				vSwsCtx = sws_getCachedContext(
					vSwsCtx,	// 传NULL会新创建
					frame->width, frame->height,		// 输入的宽高
					(AVPixelFormat)frame->format,	// 输入格式 YUV420p
					frame->width, frame->height,	// 输出的宽高
					AV_PIX_FMT_RGBA,				// 输出格式RGBA
					SWS_BILINEAR,					// 尺寸变化的算法
					0, 0, 0);
				if (vSwsCtx)
				{
					// RGB缓冲区分配内存，只第一次分配
					//（当然也可以创建pFrameRGB，用avpicture_fill初始化pFrameRGB来实现）
					if (!rgb) rgb = new unsigned char[static_cast<long long>(frame->width) * frame->height * 4];
					uint8_t* data[2] = { 0 };
					data[0] = rgb;
					int lines[2] = { 0 };
					lines[0] = frame->width * 4;
					// 类型转换：YUV转换成RGB
					nRet = sws_scale(vSwsCtx,
						frame->data,		// 输入数据
						frame->linesize,	// 输入行大小
						0,
						frame->height,		// 输入高度
						data,				// 输出数据和大小
						lines
					);
					cout << "sws_scale = " << nRet << endl;

					// 将数据以二进制的形式写入文件中
					fwrite(data[0], static_cast<long long>(frame->width)* frame->height * 4, 1, outFileRgb);
				}
			}
			else // 音频
			{
				// 创建音频采样缓冲区
				uint8_t* data[2] = { 0 };
				if (!pcm) pcm = new uint8_t[static_cast<long long>(frame->nb_samples) * 2 * 2];
				data[0] = pcm;
				// 类型转换：转换成PCM
				nRet = swr_convert(actx,
					data, frame->nb_samples,		// 输出
					(const uint8_t**)frame->data, frame->nb_samples	// 输入
				);
				cout << "swr_convert = " << nRet << endl;

				// 获取缓冲区实际存储大小
				int out_buffer_size = av_samples_get_buffer_size(NULL, out_nb_channels, frame->nb_samples,
					AV_SAMPLE_FMT_S16, 1);
				// 将数据以二进制的形式写入文件中
				fwrite(data[0], 1, out_buffer_size, outFilePcm);
			}
		}
	}

	//===================6、内存释放===================
	fclose(outFileRgb);
	fclose(outFilePcm);

	// 释放解封装上下文，并且把ic置0
	avformat_close_input(&pFormatCtx);
	// 释放视频解码器上下文
	avcodec_free_context(&pVCodecCtx);
	// 释放音频解码器上下文
	avcodec_free_context(&pVCodecCtx);
	// 释放frame
	av_frame_free(&frame);
    // 释放pkt
	av_packet_free(&pkt);

	return 0;
}
