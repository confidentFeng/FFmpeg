#pragma once
struct AVFrame;
class IVideoCall
{
public:
	virtual void init(int width, int height) = 0;
	virtual void myRepaint(AVFrame *frame) = 0;
};

