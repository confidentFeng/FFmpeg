#include "XAudioPlay.h"

class CXAudioPlay : public XAudioPlay
{
public:
	QAudioOutput *output = NULL;
	QIODevice *io = NULL;
	std::mutex mutex;
	virtual void Close()
	{
		mutex.lock();
		if (io)
		{
			io->close();
			io = NULL;
		}
		if (output)
		{
			output->stop();
			delete output;
			output = 0;
		}
		mutex.unlock();
	}
	virtual bool Open()
	{
		Close();
		QAudioFormat fmt;
		fmt.setSampleRate(sampleRate);
		fmt.setSampleSize(sampleSize);
		fmt.setChannelCount(channels);
		fmt.setCodec("audio/pcm");
		fmt.setByteOrder(QAudioFormat::LittleEndian);
		fmt.setSampleType(QAudioFormat::UnSignedInt);
		mutex.lock();
		output = new QAudioOutput(fmt);
		io = output->start(); //¿ªÊ¼²¥·Å
		mutex.unlock();
		if(io)
			return true;
		return false;
	}
	virtual bool Write(const unsigned char *data, int datasize)
	{
		if (!data || datasize <= 0)return false;
		mutex.lock();
		if (!output || !io)
		{
			mutex.unlock();
			return false;
		}
		int size = io->write((char *)data, datasize);
		mutex.unlock();
		if (datasize != size)
			return false;
		return true;
	}

	virtual int GetFree()
	{
		mutex.lock();
		if (!output)
		{
			mutex.unlock();
			return 0;
		}
		int free = output->bytesFree();
		mutex.unlock();
		return free;
	}
};

XAudioPlay::XAudioPlay()
{
}

XAudioPlay::~XAudioPlay()
{
}

XAudioPlay* XAudioPlay::Get()
{
	static CXAudioPlay play;
	return &play;
}
