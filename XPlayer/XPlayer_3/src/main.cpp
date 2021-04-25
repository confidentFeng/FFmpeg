#include "XPlayer.h"
#include <QtWidgets/QApplication>
#include <iostream>
#include <QThread>
#include <future>
#include "XDemux.h"
#include "XDecode.h"
#include "XVideoWidget.h"

extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
}
#pragma comment(lib,"avformat.lib")
#pragma comment(lib,"avutil.lib")
#pragma comment(lib,"avcodec.lib")
#pragma comment(lib,"swscale.lib")
#pragma comment(lib,"swresample.lib")

using namespace std;

int main(int argc, char* argv[])
{

	QApplication a(argc, argv);

	// 播放界面
	XPlayer w;
	w.show();

	//=================1、解封装测试====================
	XDemux demux;
	const char* url = "dove_640x360.mp4";
	cout << "demux.Open = " << demux.open(url);
	demux.read();
	cout << "demux.Open = " << demux.open(url); // open一次的话，很久之后才开始播放
	cout << "CopyVPara = " << demux.copyVPara() << endl;
	cout << "CopyAPara = " << demux.copyAPara() << endl;

	// 初始化openGl窗口
	w.video->init(demux.getVideoInfo().width, demux.getVideoInfo().height);

	//=================2、解码测试====================
	XDecode vdecode;
	XDecode adecode;
	cout << "vdecode.Open() = " << vdecode.Open(demux.copyVPara()) << endl;
	cout << "adecode.Open() = " << adecode.Open(demux.copyAPara()) << endl;
	// 开辟异步线程进行解码播放，避免阻塞GUI
	auto futureLambda = std::async([&]() {
		for (;;)
		{
			AVPacket* pkt = demux.read();
			if (!pkt)
			{
				// 异步线程退出后，才清空销毁
				demux.close();
				vdecode.Close();
				break;
			}
			if (demux.isAudio(pkt))
			{
				//adecode.Send(pkt);
				//AVFrame *frame = adecode.Recv();
				//cout << "Audio:" << frame << endl;
			}
			else
			{
				vdecode.Send(pkt);
				AVFrame* frame = vdecode.Recv();
				// OpenGl子界面重绘
				w.video->myRepaint(frame);
				QThread::msleep(40); // 25帧播放
			}
		}
	});

	return a.exec();
}
