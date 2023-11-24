#pragma once
#include <iostream>
#include "BaseDevice.h"
#include "../Plugin/vsk/x86Nvr/jsonSdkInterface.h"
#include "UserAgent/GB28181Msg.h"


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

	enum PTZCONTROL_COMMAND
	{
		JSON_PTZCOMMAND_UNKNOWN = 0,
		JSON_PTZCOMMAND_UP = 1,
		JSON_PTZCOMMAND_DOWN = 2,
		JSON_PTZCOMMAND_LEFT = 3,
		JSON_PTZCOMMAND_RIGHT = 4,
		JSON_PTZCOMMAND_UP_LEFT = 5,
		JSON_PTZCOMMAND_UP_RIGHT = 6,
		JSON_PTZCOMMAND_DOWN_LEFT = 7,
		JSON_PTZCOMMAND_DOWN_RIGHT = 8,

		JSON_PTZCOMMAND_ZOOM_IN = 9,
		JSON_PTZCOMMAND_ZOOM_OUT = 10,
		JSON_PTZCOMMAND_FOCUS_NEAR = 11,
		JSON_PTZCOMMAND_FOCUS_FAR = 12,
		JSON_PTZCOMMAND_IRIS_OPEN = 13,
		JSON_PTZCOMMAND_IRIS_CLOSE = 14,
		JSON_PTZCOMMAND_PAN_AUTO = 15,

		JSON_PTZCOMMAND_AUX_ON = 16,
		JSON_PTZCOMMAND_AUX_OFF = 17,

		JSON_PTZCOMMAND_LIGHT_OPEN = 18,
		JSON_PTZCOMMAND_LIGHT_CLOSE = 19,
		JSON_PTZCOMMAND_WIPER_OPEN = 20,
		JSON_PTZCOMMAND_WIPER_CLOSE = 21,
		JSON_PTZCOMMAND_CALL_PRESET = 22,
		JSON_PTZCOMMAND_SAVE_PRESET = 23,
		JSON_PTZCOMMAND_BUTT,
	};
	void Dev_PTZCtrl(int channelId, uint32_t PTZCommand, uint32_t Argument, int& err);

	void Dev_ListIPC(char* Buffer, uint32_t BufSize, int& err);

	static void __stdcall VskX86NvrRtPreDataCb(uint32_t PlayHandle, uint8_t* pBuffer, uint32_t BufferSize, uint32_t DateType, time_t systime, uint32_t TimeSpace, void* pUser);
	
	static PTZCONTROL_COMMAND switchFromGB28181(PTZCMDType::GB28181PtzCmd cmd);
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

	void setChannel(int chl);
	const int getChannel();
private:
	std::string name;
	int status;
	std::string parentId;
	std::string ChildIp;
	int channel;
};