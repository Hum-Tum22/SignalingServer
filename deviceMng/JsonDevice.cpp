#include "JsonDevice.h"
#include "../media/mediaIn/JsonStream.h"
#include "../tools/CodeConversion.h"

JsonNvrDevic::~JsonNvrDevic()
{
	DevDisConnect();
}
bool JsonNvrDevic::DevConnect()
{
	int err = 0;
	JsonNvrLogIn(err);
	return err ? false : true;
}
bool JsonNvrDevic::DevDisConnect()
{
	int err = 0;
	JsonNvrLogOut(err);
	return err ? false : true;
}

JsonNvrDevic::PTZCONTROL_COMMAND JsonNvrDevic::switchFromGB28181(PTZCMDType::GB28181PtzCmd cmd)
{
	switch (cmd)
	{
	case PTZCMDType::PTZ_NO:
	case PTZCMDType::PTZ_STOP:
	{
		return JsonNvrDevic::JSON_PTZCOMMAND_UNKNOWN;
	}
	case PTZCMDType::PTZ_UP:
	{
		return JsonNvrDevic::JSON_PTZCOMMAND_UP;
	}
	case PTZCMDType::PTZ_DOWN:
	{
		return JsonNvrDevic::JSON_PTZCOMMAND_DOWN;
	}
	case PTZCMDType::PTZ_LEFT:
	{
		return JsonNvrDevic::JSON_PTZCOMMAND_LEFT;
	}
	case PTZCMDType::PTZ_RIGHT:
	{
		return JsonNvrDevic::JSON_PTZCOMMAND_RIGHT;
	}
	case PTZCMDType::PTZ_UP_LEFT:
	{
		return JsonNvrDevic::JSON_PTZCOMMAND_UP_LEFT;
	}
	case PTZCMDType::PTZ_UP_RIGHT:
	{
		return JsonNvrDevic::JSON_PTZCOMMAND_UP_RIGHT;
	}
	case PTZCMDType::PTZ_DOWN_LEFT:
	{
		return JsonNvrDevic::JSON_PTZCOMMAND_DOWN_LEFT;
	}
	case PTZCMDType::PTZ_DOWN_RIGHT:
	{
		return JsonNvrDevic::JSON_PTZCOMMAND_DOWN_RIGHT;
	}
	case PTZCMDType::PTZ_ZOOM_IN:
	{
		return JsonNvrDevic::JSON_PTZCOMMAND_ZOOM_IN;
	}
	case PTZCMDType::PTZ_ZOOM_OUT:
	{
		return JsonNvrDevic::JSON_PTZCOMMAND_ZOOM_OUT;
	}
	case PTZCMDType::FOCUS_IN:
	{
		return JsonNvrDevic::JSON_PTZCOMMAND_FOCUS_NEAR;
	}
	case PTZCMDType::FOCUS_OUT:
	{
		return JsonNvrDevic::JSON_PTZCOMMAND_FOCUS_FAR;
	}
	case PTZCMDType::IRIS_IN:
	{
		return JsonNvrDevic::JSON_PTZCOMMAND_IRIS_OPEN;
	}
	case PTZCMDType::IRIS_OUT:
	{
		return JsonNvrDevic::JSON_PTZCOMMAND_IRIS_CLOSE;
	}
	case PTZCMDType::PTZ_SETPRESET:
	{
		return JsonNvrDevic::JSON_PTZCOMMAND_SAVE_PRESET;
	}
	case PTZCMDType::PTZ_GTOPRESET:
	case PTZCMDType::PTZ_DELPRESET:
	{
		break;
	}
	case PTZCMDType::PTZ_MOVPRESET:
	{
		return JsonNvrDevic::JSON_PTZCOMMAND_CALL_PRESET;
	}
	default:
		break;
	}
	return JsonNvrDevic::JSON_PTZCOMMAND_UNKNOWN;
}

void JsonNvrDevic::setStatus(int statu)
{
	status = statu;
}
const int JsonNvrDevic::getStatus()
{
	return status;
}
void JsonNvrDevic::JsonNvrLogIn(int& err)
{
	err = 0;
	if (!JsonSdkInterface::Instance().SdkIsInit())
	{
		JsonSdkInterface::Instance().InitSdk(err);
	}
	if (err == 0 && mLoginId == 0)
	{
		mLoginId = JsonSdkInterface::Instance().LogIn(mIP.c_str(), mPort, mName.c_str(), mPswd.c_str(), err);
		if (err != 0)
		{

		}
	}
}
void JsonNvrDevic::JsonNvrLogOut(int& err)
{
	err = 0;
	if (!JsonSdkInterface::Instance().SdkIsInit())
	{
		JsonSdkInterface::Instance().InitSdk(err);
	}
	if (err == 0 && mLoginId == 0)
	{
		JsonSdkInterface::Instance().LogOut(mLoginId, err);
		if (err == 3001 || err == 2007)
		{
			JsonSdkInterface::Instance().LogOut(mLoginId, err);
			mLoginId = 0;
		}
	}
}
void JsonNvrDevic::GetChannelEncoderParam(int channel, char* pBuffer, uint32_t* pInfoSize, int& err)
{
	err = 0;
	if (!JsonSdkInterface::Instance().SdkIsInit())
	{
		JsonSdkInterface::Instance().InitSdk(err);
	}
	if (err == 0 && mLoginId == 0)
	{
		mLoginId = JsonSdkInterface::Instance().LogIn(mIP.c_str(), mPort, mName.c_str(), mPswd.c_str(), err);
		if (err != 0)
		{
			return;
		}
	}
	if (mLoginId > 0)
	{
		JsonSdkInterface::Instance().GetChannelEncoderParam(mLoginId, channel, pBuffer, pInfoSize, err);
		if (err == 3001 || err == 2007)
		{
			JsonSdkInterface::Instance().LogOut(mLoginId, err);
			mLoginId = 0;
		}
	}
}
ULHandle JsonNvrDevic::Dev_Preview(int channelId, int streamType, void* VideoTranCallBack, void* pUser, int& err)
{
	err = 0;
	if (!JsonSdkInterface::Instance().SdkIsInit())
	{
		JsonSdkInterface::Instance().InitSdk(err);
	}
	if (err == 0 && mLoginId == 0)
	{
		mLoginId = JsonSdkInterface::Instance().LogIn(mIP.c_str(), mPort, mName.c_str(), mPswd.c_str(), err);
		if (err != 0)
		{

		}
	}
	if (mLoginId > 0)
	{
		ULHandle ulPreviewHandle = JsonSdkInterface::Instance().VskPreview(mLoginId, channelId, streamType, (DataPlayCallBack)VideoTranCallBack, pUser, err);
		if (err == 3001 || err == 2007)
		{
			JsonSdkInterface::Instance().LogOut(mLoginId, err);
			mLoginId = 0;
		}
		return ulPreviewHandle;
	}
	return 0;
}
void JsonNvrDevic::Dev_StopPreview(ULHandle handle, int& err)
{
	err = 0;
	if (!JsonSdkInterface::Instance().SdkIsInit())
	{
		JsonSdkInterface::Instance().InitSdk(err);
	}
	if (mLoginId > 0)
	{
		JsonSdkInterface::Instance().StopPreview(handle, err);
		if (err == 3001 || err == 2007)
		{
			JsonSdkInterface::Instance().LogOut(mLoginId, err);
			mLoginId = 0;
		}
	}
}

ULHandle JsonNvrDevic::Dev_PlayBack(int channelId, long start, long end, void* VideoTranCallBack, void* fun, void* pUser, int& err)
{
	err = 0;
	if (!JsonSdkInterface::Instance().SdkIsInit())
	{
		JsonSdkInterface::Instance().InitSdk(err);
	}
	if (err == 0 && mLoginId == 0)
	{
		mLoginId = JsonSdkInterface::Instance().LogIn(mIP.c_str(), mPort, mName.c_str(), mPswd.c_str(), err);
		if (err != 0)
		{

		}
	}
	if (mLoginId > 0)
	{
		ULHandle ulPreviewHandle = JsonSdkInterface::Instance().PlayBack(mLoginId, channelId, start, end, (DataPlayCallBack)VideoTranCallBack, (PlayBackEndCallBack)fun, pUser, err);
		if (err == 3001 || err == 2007)
		{
			JsonSdkInterface::Instance().LogOut(mLoginId, err);
			mLoginId = 0;
		}
		return ulPreviewHandle;
	}
	return 0;
}
void JsonNvrDevic::Dev_StopPlayBack(ULHandle handle, int& err)
{
	err = 0;
	if (!JsonSdkInterface::Instance().SdkIsInit())
	{
		JsonSdkInterface::Instance().InitSdk(err);
	}
	if (mLoginId > 0)
	{
		JsonSdkInterface::Instance().StopPlayBack(handle, err);
		if (err == 3001 || err == 2007)
		{
			JsonSdkInterface::Instance().LogOut(mLoginId, err);
			mLoginId = 0;
		}
	}
}
void JsonNvrDevic::Dev_PlayBackCtrl(ULHandle handle, int cmd, int param1, int param2, int& err)
{
	err = 0;
	if (!JsonSdkInterface::Instance().SdkIsInit())
	{
		JsonSdkInterface::Instance().InitSdk(err);
	}
	if (mLoginId > 0)
	{
		JsonSdkInterface::Instance().PlayBackCtrl(handle, cmd, param1, param2, err);
		if (err == 3001 || err == 2007)
		{
			JsonSdkInterface::Instance().LogOut(mLoginId, err);
			mLoginId = 0;
		}
	}
}
void JsonNvrDevic::Dev_PbCtrlTimePos(ULHandle handle, time_t pos, int& err)
{
	err = 0;
	if (!JsonSdkInterface::Instance().SdkIsInit())
	{
		JsonSdkInterface::Instance().InitSdk(err);
	}
	if (mLoginId > 0)
	{
		JsonSdkInterface::Instance().setTimePos(handle, pos, err);
		if (err == 3001 || err == 2007)
		{
			JsonSdkInterface::Instance().LogOut(mLoginId, err);
			mLoginId = 0;
		}
	}
}
ULHandle JsonNvrDevic::Dev_Download(int channelId, long start, long end, void* VideoTranCallBack, void* fun, void* pUser, int& err)
{
	return 0;
}
void JsonNvrDevic::Dev_StopDownload(ULHandle, int& err)
{
}

void JsonNvrDevic::Dev_PTZCtrl(int channelId, uint32_t PTZCommand, uint32_t Argument, int& err)
{
}
void JsonNvrDevic::Dev_ListIPC(char* Buffer, uint32_t &BufSize, int& err)
{
	err = 0;
	if (!JsonSdkInterface::Instance().SdkIsInit())
	{
		JsonSdkInterface::Instance().InitSdk(err);
		if (err != 0)
		{
			printf("json sdk init err:%d\n", err);
		}
	}
	if (err == 0 && mLoginId == 0)
	{
		mLoginId = JsonSdkInterface::Instance().LogIn(mIP.c_str(), mPort, mName.c_str(), mPswd.c_str(), err);
		if (err != 0)
		{
			printf("json sdk login err:%d\n", err);
		}
	}
	if (mLoginId > 0)
	{
		uint32_t size = BufSize;
		JsonSdkInterface::Instance().ListIPC(mLoginId, Buffer, &size, err);
		if (err != 0)
		{
			printf("json sdk get ipc list err:%d\n", err);
			if (err == 3001 || err == 2007)
			{
				JsonSdkInterface::Instance().LogOut(mLoginId, err);
				mLoginId = 0;
			}
		}
		if (err == 0)
		{
			BufSize = size;
		}
	}
	return;
}
void JsonNvrDevic::setIp(const std::string ip)
{
	mIP = ip;
}
const std::string& JsonNvrDevic::getIp() const
{
	return mIP;
}

void JsonNvrDevic::setPort(const short port)
{
	mPort = port;
}
const short JsonNvrDevic::getPort() const
{
	return mPort;
}

void JsonNvrDevic::setUser(const std::string user)
{
	mName = user;
}
const std::string& JsonNvrDevic::getUser() const
{
	return mName;
}

void JsonNvrDevic::setPswd(const std::string pswd)
{
	mPswd = pswd;
}
const std::string& JsonNvrDevic::getPswd() const
{
	return mPswd;
}










JsonChildDevic::JsonChildDevic(const char* Id) :BaseChildDevice(Id), channel(-1)
{
}
JsonChildDevic::~JsonChildDevic()
{
}
//void JsonChildDevic::setParentDev(JsonNvrDevic::Ptr pdev)
//{
//	mParentDev = pdev;
//}
//const JsonNvrDevic::Ptr JsonChildDevic::getParentDev()
//{
//	return mParentDev;
//}
void JsonChildDevic::setName(std::string nm)
{
	name = nm;
}
const std::string JsonChildDevic::getName()
{
	return name;
}
void JsonChildDevic::setStatus(int statu)
{
	status = statu;
}
const int JsonChildDevic::getStatus()
{
	return status;
}
void JsonChildDevic::setParentId(std::string pId)
{
	parentId = pId;
}
const std::string JsonChildDevic::getParentId()
{
	return parentId;
}
void JsonChildDevic::setChildIp(std::string Ip)
{
	ChildIp = Ip;
}
const std::string JsonChildDevic::getChildIp()
{
	return ChildIp;
}
void JsonChildDevic::setChannel(int chl)
{
	channel = chl;
}
const int JsonChildDevic::getChannel()
{
	return channel;
}
CatalogItem JsonChildDevic::GetCatalogItem(std::string myId)
{
	CatalogItem item;
	item.DeviceID = getDeviceId();
	if (parentId.empty())
	{
		item.ParentID = myId;
	}
	else
	{
		item.ParentID = parentId;
	}
	item.Name = Utf8ToGbk(name);

	item.Manufacturer = "VSK";//当为设备时,设备厂商(必选)
	item.Model = "";//当为设备时,设备型号(必选)
	item.Owner = "";//当为设备时,设备归属(必选)
	item.CivilCode = item.DeviceID.substr(0, 6);//行政区域(必选)
	item.Block = "";//警区(可选)
	item.Address = "";//当为设备时,安装地址(必选)
	item.Parental = 0;//当为设备时,是否有子设备(必选)1有,0没有
	item.SafetyWay = 0;//信令安全模式(可选)缺省为0; 0:不采用;2:S/MIME 签名方式;3:S/MIME加密签名同时采用方式; 4:数字摘要方式
	item.RegisterWay = 1;//注册方式(必选)缺省为1;1:符合IETFRFC3261标准的认证注册模式; 2:基于口令的双向认证注册模式; 3:基于数字证书的双向认证注册模式
	item.CertNum = 0;//证书序列号(有证书的设备必选)
	item.Certifiable = 0;//证书有效标识(有证书的设备必选)缺省为0;证书有效标识:0:无效 1:有效
	item.ErrCode = 0;//无效原因码(有证书且证书无效的设备必选)
	item.EndTime = "";//证书终止有效期(有证书的设备必选)
	item.Secrecy = 0;//保密属性(必选)缺省为0;0:不涉密,1:涉密
	item.IPAddress = ChildIp;//设备/区域/系统IP地址(可选)
	item.Port = 0;//设备/区域/系统端口(可选)
	item.Password = "";
	item.Status = status ? "ON" : "OFF";
	item.Longitude = 0;//经度(可选)
	item.Latitude = 0;//纬度(可选)
	return item;
}