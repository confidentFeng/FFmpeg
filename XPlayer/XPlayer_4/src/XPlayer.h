#pragma once

#include <QtWidgets/QWidget>
#include <QOpenGLWidget>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QPainter>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QTimer>
#include <QMouseEvent>
#include <QEvent>
#include <QStyle>
#include <QDebug>
#include "XVideoWidget.h"

class XPlayer : public QWidget
{
    Q_OBJECT

public:
    XPlayer(QWidget *parent = Q_NULLPTR);

	XVideoWidget* video;

private:
	//bool eventFilter(QObject* obj, QEvent* event); // 鼠标点击进行滑动条滑块定位
	//void sliderMouseLPress(QObject* slider, QEvent* event); // 滑动条滑块定位
	//char* getTimeStr(const int& nSecCnt); // 根据总秒数获取"时分秒字符串"

	QPushButton* m_pBtnOpen; // 打开视频按钮
	QPushButton* m_pBtnPlay; // 播放按钮
	QSlider* m_pSliderVideo; // 视频滑动条
	QLabel* m_pLabCurTime; // 当前播放时间
	QLabel* m_pLabTotal; // 当前播放时间

	QImage m_image; // 接受存放FFmpeg转换来的每一帧图片数据
	//VPlayThread* m_vPlayThread; // 视频播放线程
	QTimer* m_pTimerPts;

	bool m_isPressSlider = false; // 是否按下滑动条
	bool m_bIsPlay = false; // 是否在播放
};
