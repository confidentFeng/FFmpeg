#include <QtWidgets/QApplication>
#include "XDemux.h"
#include "XDecode.h"
#include "XResample.h"
#include <QThread>
#include "XAudioPlay.h"
#include "XAudioThread.h"
#include "XVideoThread.h"
#include "XDemuxThread.h"
#include "XPlayer.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	XPlayer w;
	w.show();

	return a.exec();
}
