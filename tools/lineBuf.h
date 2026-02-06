#pragma once
#include <stdint.h>


class LineBuffer
{
public:
	LineBuffer(int size = 0);
	~LineBuffer();
	char *Buf();
	int BufSize();
	char *writePos();
	char *data();
	int dataSize();
	int tailSize();
	int freeSize();
	void DataForward();
	void resetBuf(int size);
	void setReadSize(int size);
	void setWriteSize(int size);
	bool writeBuf(char *buf, int size);
	void clear();
private:
	char *pBuf;
	int bufLen;
	int wPos;
	int rPos;
};
