
//#include <windows.h>
#include <string>
#include <memory>
#include <string.h>

#include "jsonSdkInterface.h"


int main()
{
	int err = 0;
	std::string ip("192.168.1.176");
	int port = 7000;
	std::string user("admin");
	std::string pswd("12345");
	if (!JsonSdkInterface::Instance().SdkIsInit())
	{
		JsonSdkInterface::Instance().InitSdk(err);
		if (err != 0)
		{
			printf("json sdk init err:%d\n", err);
		}
	}
	JSONLONG mLoginId = 0;
	if (err == 0 && mLoginId == 0)
	{
		mLoginId = JsonSdkInterface::Instance().JsonSdkLogIn(ip.c_str(), port, user.c_str(), pswd.c_str(), err);
		if (err != 0)
		{
			printf("json sdk login err:%d\n", err);
		}
	}
	if (mLoginId > 0)
	{
		uint32_t msgSize = 1024 * 8 * 3;
		char* Buffer = new char[msgSize];
		memset(Buffer, 0, msgSize);
		JsonSdkInterface::Instance().ListIPC(mLoginId, Buffer, &msgSize, err);
		if (err != 0)
		{
			printf("json sdk get ipc list err:%d\n", err);
			if (err == 3001)
			{
				mLoginId = 0;
			}
		}
		printf("%s\n", Buffer);
	}
	JsonSdkInterface::Instance().JsonSdkLogOut(mLoginId, err);
	JsonSdkInterface::Instance().SdkClear(err);
	return 0;
}