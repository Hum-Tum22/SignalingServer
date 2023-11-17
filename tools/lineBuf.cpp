

#include "lineBuf.h"
#include <stdlib.h>
#include <string.h>


LineBuffer::LineBuffer(int size):pBuf(NULL), bufLen(size), wPos(0), rPos(0)
{
	pBuf = (char*)malloc(size * sizeof(char));
}
LineBuffer::~LineBuffer()
{
	if(pBuf)
	{
		free(pBuf);
		pBuf = NULL;
	}
}
char *LineBuffer::Buf()
{
	return pBuf;
}
int LineBuffer::BufSize()
{
	return bufLen;
}
char *LineBuffer::writePos()
{
	if(pBuf && wPos < bufLen)
	{
		return &pBuf[wPos];
	}
	return NULL;
}
char *LineBuffer::data()
{
	if(pBuf && wPos >= rPos)
	{
		return &pBuf[rPos];
	}
	return NULL;
}
int LineBuffer::dataSize()
{
	return wPos - rPos;
}
int LineBuffer::tailSize()
{
	return bufLen - wPos;
}
int LineBuffer::freeSize()
{
	return bufLen - (wPos - rPos);
}
void LineBuffer::DataForward()
{
	if (pBuf)
	{
		memmove(pBuf, (const void*)&pBuf[rPos], wPos - rPos);
		wPos = wPos - rPos;
		rPos = 0;
	}
}
void LineBuffer::resetBuf(int size)
{
	char * tempbuf=NULL;
	if(pBuf)
		tempbuf=(char *)realloc(pBuf, size*sizeof(char));
	if(tempbuf)
	{
		pBuf = tempbuf;
		bufLen = size;
	}
}
void LineBuffer::setReadSize(int size)
{
	if(rPos + size <= wPos)
		rPos += size;
	if (rPos == wPos)
	{
		rPos = wPos = 0;
	}
}
void LineBuffer::setWriteSize(int size)
{
	if(wPos + size <= bufLen)
		wPos += size;
}
bool LineBuffer::writeBuf(char *buf, int size)
{
	if(buf && size > 0)
	{
		if(wPos + size < bufLen)
		{
			memcpy(&pBuf[wPos], buf, size);
			wPos += size;
			return true;
		}
	}
	return false;
}
void LineBuffer::clear()
{
	wPos = rPos = 0;
}