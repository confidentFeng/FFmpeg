#include "XSlider.h"
void XSlider::mousePressEvent(QMouseEvent *e)
{
	double pos = (double)e->pos().x() / (double)width();
	setValue(pos * this->maximum());
	//原有事件处理
	//QSlider::mousePressEvent(e);
	QSlider::sliderReleased();
}
XSlider::XSlider(QWidget *parent)
	: QSlider(parent)
{
}

XSlider::~XSlider()
{
}
