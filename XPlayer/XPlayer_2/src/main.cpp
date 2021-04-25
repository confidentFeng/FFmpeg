#include <iostream>
#include <fstream>
#include "XDemux.h"
#include "XDecode.h"

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
	//=================1°¢Ω‚∑‚◊∞≤‚ ‘====================
	const char* url = "dove_640x360.mp4";
	XDemux demux; // ≤‚ ‘XDemux
	cout << "demux.Open = " << demux.open(url);
	demux.read();
	cout << "CopyVPara = " << demux.copyVPara() << endl;
	cout << "CopyAPara = " << demux.copyAPara() << endl;
	cout << "seek=" << demux.seek(0.95) << endl;

	//=================2°¢Ω‚¬Î≤‚ ‘====================
	XDecode decode; // ≤‚ ‘XDecode
	cout << "vdecode.Open() = " << decode.Open(demux.copyVPara()) << endl;
	XDecode adecode;
	cout << "adecode.Open() = " << adecode.Open(demux.copyAPara()) << endl;

	while(1)
	{
		AVPacket* pkt = demux.read();
		if (demux.isAudio(pkt))
		{
			adecode.Send(pkt);
			AVFrame* frame = adecode.Recv();
			cout << "Audio:" << frame << endl;
		}
		else
		{
			decode.Send(pkt);
			AVFrame* frame = decode.Recv();
			cout << "Video:" << frame << endl;
		}
		if (!pkt) break;
	}
	
	//  Õ∑≈…Í«Îƒ⁄¥Ê
	demux.close();
	decode.Close();

	// µ»¥˝Ω¯≥ÃÕÀ≥ˆ
	system("pause");

	return 0;
}
