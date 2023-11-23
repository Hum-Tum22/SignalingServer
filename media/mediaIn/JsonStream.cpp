#include "JsonStream.h"


JsonStream::JsonStream(const char* streamId) :MediaStream(streamId), /*streamHandle(0),*/ mJsonNvr(NULL)
{
}
JsonStream::~JsonStream()
{

}
//void JsonStream::setStreamHandle(SHANDLE handle)
//{
//	streamHandle = handle;
//}
//const SHANDLE JsonStream::getStreamHandle()
//{
//	return streamHandle;
//}
void JsonStream::setDevice(JsonNvrDevic::Ptr nvr)
{
	mJsonNvr = nvr;
}

void __stdcall JsonStream::NvrRtPreDataCb(uint32_t handle, const uint8_t* pBuffer, unsigned int BufferSize, void* pUser)
{
}
void __stdcall JsonStream::VskX86NvrRtPreDataCb(unsigned int PlayHandle, uint8_t* pBuffer, unsigned int BufferSize, unsigned int DateType, time_t systime, unsigned int TimeSpace, void *pUser)
{
	JsonStream* pThis = (JsonStream*)pUser;
	if (pThis)
	{
		pThis->OnMediaStream(DateType, pBuffer, (size_t)BufferSize);
	}
}