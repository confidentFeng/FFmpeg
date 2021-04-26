#pragma once
struct AVFrame;

class IVideoCall
{
public:
	virtual void Init(int width, int height) = 0;
	virtual void Repaint(AVFrame *frame) = 0;
};

