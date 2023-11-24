#pragma once
#include <stdint.h>

class PTZCMDType
{
	/*28181  ptz 这些 命令是可以组合的*/
public:
	typedef enum {
		PTZ_NO,//没获得命令
		PTZ_STOP = 1,
		PTZ_UP,
		PTZ_DOWN,
		PTZ_LEFT,
		PTZ_RIGHT,
		PTZ_UP_LEFT,
		PTZ_UP_RIGHT,
		PTZ_DOWN_LEFT,
		PTZ_DOWN_RIGHT,

		PTZ_ZOOM_IN,//ZOOM 大
		PTZ_ZOOM_OUT,//

		FOCUS_IN,//近
		FOCUS_OUT,//

		IRIS_IN,//XIAO
		IRIS_OUT,//DA

		PTZ_SETPRESET,
		PTZ_GTOPRESET,
		PTZ_DELPRESET,
		PTZ_MOVPRESET
	}GB28181PtzCmd;
	PTZCMDType();
	PTZCMDType(const char* ptzcmd);//28181 文档那个里面的描述和实际指令不符
	char mptzcmd[20];//理论上是8字节 现在是16字节
	static int HexStrToTen(char HexStr);//28181 比较特殊
	static int HexStrToTen(char* HexStr, int len);
	static void getCheckCode(char* sCmd);
	static char TenIntToChar(int ii);
	static int  TenIntToChar2(int ii, char* p);
	bool isStop();
	bool isHavePtzCmd();
	bool isHaveFICmd();
	uint32_t GetPTZUDLREx(int& lrArg, int& udarg);
	uint32_t GetPTZZoomCmd(int& arg);
	uint32_t GetFIIrseCmd(int& arg);
	uint32_t GetFIFocusCmd(int& arg);
	uint32_t GetPresetCmd(int& arg);
	void GetCmdParityBit();
	bool AddPtzCmd(int pcmd, int arg);
};