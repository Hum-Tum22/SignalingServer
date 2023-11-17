#pragma once
#include <iostream>
#include "BaseDevice.h"
#include "../Plugin/vsk/x86Nvr/jsonSdkInterface.h"


class JsonNvrDevic : public BaseDevice
{
public:
	JsonNvrDevic(const char*Id, const char* Ip, int port, const char* name, const char* pswd) :BaseDevice(Id, JSON_NVR),
				mIP(Ip), mPort(port), mName(name), mPswd(pswd), status(0),mLoginId(0){};
	~JsonNvrDevic();

	bool DevConnect();
	bool DevDisConnect();

	ULHandle Dev_Preview(int channelId, int streamType, void* VideoTranCallBack, void* pUser, int& err);
	void Dev_StopPreview(ULHandle, int& err);

	ULHandle Dev_PlayBack(int channelId, long start, long end, BaseDevice::PbCbData VideoTranCallBack, PbCbEnd fun, void* pUser, int& err);
	void Dev_StopPlayBack(ULHandle, int& err);

	ULHandle Dev_Download(int channelId, long start, long end, DownloadData VideoTranCallBack, DownloadEnd fun, void* pUser, int& err);
	void Dev_StopDownload(ULHandle, int& err);

	void Dev_PTZCtrl(int channelId, uint32_t PTZCommand, uint32_t Argument, int& err);

	void Dev_ListIPC(char* Buffer, uint32_t BufSize, int& err);

	static void __stdcall VskX86NvrRtPreDataCb(uint32_t PlayHandle, uint8_t* pBuffer, uint32_t BufferSize, uint32_t DateType, time_t systime, uint32_t TimeSpace, void* pUser);
	
	void setStatus(int statu);
	const int getStatus();
private:
	void JsonNvrLogIn(int& err);
	void JsonNvrLogOut(int& err);

	std::string mIP;
	short mPort;
	std::string mName;
	std::string mPswd;
	int status;
	ULHandle mLoginId;
};

class JsonChildDevic : public BaseChildDevice
{
public:
	JsonChildDevic(const char*Id);
	~JsonChildDevic();

	void setName(std::string nm);
	const std::string getName();
	void setStatus(int statu);
	const int getStatus();
	void setParentId(std::string nm);
	const std::string getParentId();

	void setChildIp(std::string Ip);
	const std::string getChildIp();
private:
	std::string name;
	int status;
	std::string parentId;
	std::string ChildIp;
};