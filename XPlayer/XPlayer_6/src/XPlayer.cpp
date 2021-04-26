#pragma execution_character_set("utf-8")
#include "XPlayer.h"
#include <QFileDialog>
#include <QDebug>
#include "XDemuxThread.h"
#include <QMessageBox>
static XDemuxThread dt;

XPlayer::XPlayer(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	dt.Start();
	startTimer(40);
}

XPlayer::~XPlayer()
{
	dt.Close(); 
}	

void XPlayer::SliderPress()
{
	isSliderPress = true;
}

void XPlayer::SliderRelease()
{
	isSliderPress = false;
	double pos = 0.0;
	pos = (double)ui.playPos->value() / (double)ui.playPos->maximum();
	dt.Seek(pos);
}

// 定时器 滑动条显示
void XPlayer::timerEvent(QTimerEvent *e)
{
	if (isSliderPress)return;
	long long total = dt.totalMs;
	if (total > 0)
	{
		double pos = (double)dt.pts / (double)total;
		int v = ui.playPos->maximum() * pos;
		ui.playPos->setValue(v);
	}
}

// 双击全屏
void XPlayer::mouseDoubleClickEvent(QMouseEvent *e)
{
	if (isFullScreen())
		this->showNormal();
	else
		this->showFullScreen();
}

// 窗口尺寸变化
void XPlayer::resizeEvent(QResizeEvent *e)
{
	ui.playPos->move(50, this->height() - 100);
	ui.playPos->resize(this->width() - 100, ui.playPos->height());
	ui.openFile->move(100,this->height() - 150);
	ui.isplay->move(ui.openFile->x() + ui.openFile->width() + 10, ui.openFile->y());
	ui.video->resize(this->size());
}

void XPlayer::PlayOrPause()
{
	bool isPause = !dt.isPause;
	SetPause(isPause);
	dt.SetPause(isPause);
}

void XPlayer::SetPause(bool isPause)
{
	if (isPause)
		ui.isplay->setText("播 放");
	else
		ui.isplay->setText("暂 停");
}

void XPlayer::OpenFile()
{
	// 选择文件
	QString name = QFileDialog::getOpenFileName(this, "选择视频文件");
	if (name.isEmpty())return;
	this->setWindowTitle(name);
	if (!dt.Open(name.toLocal8Bit(), ui.video))
	{
		QMessageBox::information(0, "error", "open file failed!");
		return;
	}
	SetPause(dt.isPause);
}