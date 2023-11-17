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
void __stdcall JsonStream::VskX86NvrRtPreDataCb(uint32_t PlayHandle, uint8_t* pBuffer, uint32_t BufferSize, uint32_t DateType, time_t systime, uint32_t TimeSpace, void* pUser)
{
	JsonStream* pThis = (JsonStream*)pUser;
	if (pThis)
	{
		pThis->OnMediaStream(DateType, pBuffer, BufferSize);
	}
}