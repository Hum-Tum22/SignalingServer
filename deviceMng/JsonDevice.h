#pragma once
#include <iostream>
#include "BaseDevice.h"
#include "../Plugin/vsk/x86Nvr/jsonSdkInterface.h"
#include "UserAgent/GB28181Msg.h"
#include "../uainfo.h"


class JsonNvrDevic : public BaseDevice
{
public:
	JsonNvrDevic(const char*Id, const char* Ip, int port, const char* name, const char* pswd) :BaseDevice(Id, JSON_NVR),
				mIP(Ip), mPort(port), mName(name), mPswd(pswd), status(0),mLoginId(0){};
	~JsonNvrDevic();

	bool DevConnect();
	bool DevDisConnect();

	void GetChannelEncoderParam(int channel, char* pBuffer, uint32_t* pInfoSize, int& err);
	ULHandle Dev_Preview(int channelId, int streamType, void* VideoTranCallBack, void* pUser, int& err);
	void Dev_StopPreview(ULHandle, int& err);

	ULHandle Dev_PlayBack(int channelId, long start, long end, void* VideoTranCallBack, void* fun, void* pUser, int& err);
	void Dev_StopPlayBack(ULHandle, int& err);
	void Dev_PlayBackCtrl(ULHandle, int cmd, int param1, int param2, int& err);
	void Dev_PbCtrlTimePos(ULHandle, time_t pos, int& err);

	ULHandle Dev_Download(int channelId, long start, long end, void* VideoTranCallBack, void* fun, void* pUser, int& err);
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

	void Dev_ListIPC(char* Buffer, uint32_t &BufSize, int& err);

	static PTZCONTROL_COMMAND switchFromGB28181(PTZCMDType::GB28181PtzCmd cmd);

	void setStatus(int statu);
	const int getStatus();

	void setIp(const std::string ip);
	const std::string& getIp() const;

	void setPort(const short port);
	const short getPort() const;

	void setUser(const std::string user);
	const std::string& getUser() const;

	void setPswd(const std::string pswd);
	const std::string& getPswd() const;

	enum JsonPbCtrlCode
	{
		JsonPbCtrl_Play = 0,	// 继续播放
		JsonPbCtrl_Pause = 1,	// 暂停
		JsonPbCtrl_Speed = 2,	// 速度控制
		JsonPbCtrl_FilePos = 3,	// 修改按文件回放时的播放位置
		JsonPbCtrl_TimePos = 4,	// 修改按时间回放时的播放位置
		JsonPbCtrl_DropFrame = 5	// 丢帧控制: param1: I帧后面发送的p帧数量(0: 全部丢p帧, 1000: 不丢p帧), param2: 停止发送音频帧(0: 正常发送, 1:停止发送)
	};
	enum JsonPbSpeed
	{
		JsonPbSpeed_0 = 0,	// 1/4
		JsonPbSpeed_1 = 1,	// 1/2
		JsonPbSpeed_2 = 2,	// 1
		JsonPbSpeed_3 = 3,	// 2
		JsonPbSpeed_4 = 4,	// 4
		JsonPbSpeed_5 = 5,	// 8
	};
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

	CatalogItem GetCatalogItem(std::string myId);
private:
	std::string name;
	int status;
	std::string parentId;
	std::string ChildIp;
	int channel;
};