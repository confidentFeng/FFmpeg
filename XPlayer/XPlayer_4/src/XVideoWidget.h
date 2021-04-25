#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QGLShaderProgram>
#include <mutex>

extern "C" {
#include <libavutil/frame.h>
}

struct AVFrame;

class XVideoWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
	Q_OBJECT

public:
	XVideoWidget();
	XVideoWidget(QWidget* parent);
	~XVideoWidget();

	void init(int width, int height); // 初始化	
	void myRepaint(AVFrame *frame); // 重绘AVFrame(不能与Qt的repaint函数名冲突)

protected:	
	void initializeGL(); // 初始化gl
	void paintGL(); // 刷新显示	
	void resizeGL(int width, int height); // 窗口尺寸变化

private:
	QGLShaderProgram program; // shader程序	
	GLuint unis[3] = { 0 }; // shader中yuv变量地址	
	GLuint texs[3] = { 0 }; // openg的 texture地址

	// 材质内存空间
	unsigned char *datas[3] = { 0 };
	int width = 240;
	int height = 128;

	std::mutex mux;
};
