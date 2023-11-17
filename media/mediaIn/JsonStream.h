#pragma once
#include "../MediaStream.h"
#include "../../deviceMng/JsonDevice.h"


class JsonStream : public MediaStream
{
public:
	JsonStream(const char* streamId);
	virtual ~JsonStream();

	void setDevice(JsonNvrDevic::Ptr);

	int getvalue() { return 0; };
	static void CALLBACK NvrRtPreDataCb(uint32_t handle, const uint8_t* pBuffer, unsigned int BufferSize, void* pUser);

	//static void CALLBACK VskX86NvrRtPreDataCb(uint32_t PlayHandle, uint8_t* pBuffer, uint32_t BufferSize, uint32_t DateType, time_t systime, uint32_t TimeSpace, void* pUser);
	//unsigned int PlayHandle,unsigned int DateType,BYTE *pBuffer,unsigned int BufferSize,LPVOID pUser
	static void CALLBACK VskX86NvrRtPreDataCb(unsigned int PlayHandle, unsigned int DateType, unsigned char *pBuffer,unsigned int BufferSize, void* pUser);
	

private:
	JsonNvrDevic::Ptr mJsonNvr;
};