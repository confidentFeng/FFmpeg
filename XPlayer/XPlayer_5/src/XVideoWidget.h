#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QGLShaderProgram>
#include <mutex>
#include "IVideoCall.h"

extern "C" {
#include <libavutil/frame.h>
}

struct AVFrame;

class XVideoWidget : public QOpenGLWidget, protected QOpenGLFunctions, public IVideoCall
{
	Q_OBJECT

public:
	XVideoWidget();
	XVideoWidget(QWidget* parent);
	~XVideoWidget();

	virtual void init(int width, int height); // ��ʼ��	
	virtual void myRepaint(AVFrame *frame); // �ػ�AVFrame(������Qt��repaint��������ͻ)

protected:	
	void initializeGL(); // ��ʼ��gl
	void paintGL(); // ˢ����ʾ	
	void resizeGL(int width, int height); // ���ڳߴ�仯

private:
	QGLShaderProgram program; // shader����	
	GLuint unis[3] = { 0 }; // shader��yuv������ַ	
	GLuint texs[3] = { 0 }; // openg�� texture��ַ

	// �����ڴ�ռ�
	unsigned char *datas[3] = { 0 };
	int width = 240;
	int height = 128;

	std::mutex mux;
};