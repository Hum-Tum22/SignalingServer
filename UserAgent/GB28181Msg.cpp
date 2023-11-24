#include "GB28181Msg.h"
#include "GB28181Msg.h"
#include <string.h>
#include <stdio.h>
#include <math.h>

PTZCMDType::PTZCMDType()
{
	memset(mptzcmd, '\0', sizeof(mptzcmd));
	snprintf(mptzcmd, 20, "%s", "A50F010000000000");
}
PTZCMDType::PTZCMDType(const char* ptzcmd)
{
	strncpy(mptzcmd, ptzcmd, 16);
}
int PTZCMDType::HexStrToTen(char HexStr)
{
	if (HexStr >= '0' && HexStr <= '9')
	{
		return HexStr - '0';
	}
	else
	{
		switch (HexStr)
		{
		case 'A':
		case 'a':
			return 10;
		case 'B':
		case 'b':
			return 11;
		case 'C':
		case 'c':
			return 12;
		case 'D':
		case 'd':
			return 13;
		case 'E':
		case 'e':
			return 14;
		case 'F':
		case 'f':
			return 15;

		}
	}
	return 0;
}
int PTZCMDType::HexStrToTen(char* HexStr, int len)
{
	int gint = 0;
	double j = 0;
	for (int i = len - 1; i >= 0; i--)
	{
		gint += HexStrToTen(HexStr[i]) * pow(16, j);
		j++;
	}
	return gint;
}
void PTZCMDType::getCheckCode(char* sCmd)
{
}
void PTZCMDType::GetCmdParityBit()
{
	char pBuf[2] = { 0 };
	int nSum = 0;
	for (int i = 0; i < 14; i++)
	{
		pBuf[0] = mptzcmd[i++];
		pBuf[1] = mptzcmd[i];
		nSum += HexStrToTen(pBuf, 2);
	}
	int nCheckCode = nSum % 256;
	TenIntToChar2(nCheckCode, pBuf);
	memcpy(mptzcmd + 14, pBuf, 2);
}
char  PTZCMDType::TenIntToChar(int ii)
{
	if (ii > 16)
	{
		return 0;
	}
	char temp;
	switch (ii)
	{
	case 0:
		return '0';
	case 1:
		return '1';
	case 2:
		return '2';
	case 3:
		return '3';
	case 4:
		return '4';
	case 5:
		return '5';
	case 6:
		return '6';
	case 7:
		return '7';
	case 8:
		return '8';
	case 9:
		return '9';
	case 10:
		return 'A';
	case 11:
		return 'B';
	case 12:
		return 'C';
	case 13:
		return 'D';
	case 14:
		return 'E';
	case 15:
		return 'F';
	}
	return '0';
}
int  PTZCMDType::TenIntToChar2(int ii, char* p)
{
	if (ii > 16)
	{
		p[0] = TenIntToChar(ii / 16);
		p[1] = TenIntToChar(ii % 16);
	}
	else
	{
		p[0] = '0';
		p[1] = TenIntToChar(ii);
	}
	return 0;
}
bool PTZCMDType::isHavePtzCmd()
{
	int ntempCmd = HexStrToTen(mptzcmd[6]);
	if (ntempCmd & (1 << 2))
	{
		return false;
	}
	return true;
}
bool PTZCMDType::isHaveFICmd()
{
	int ntempCmd = HexStrToTen(mptzcmd[6]);
	if (ntempCmd & (1 << 2))
	{
		return true;
	}
	return false;
}
uint32_t PTZCMDType::GetPTZZoomCmd(int& arg)
{
	int ntempCmd = HexStrToTen(mptzcmd[6]);

	arg = 0;
	if (ntempCmd & 1)//ZOOM da
	{
		arg = HexStrToTen(&mptzcmd[12], 2);
		return PTZ_ZOOM_IN;
	}
	else if (ntempCmd & 1 << 1)
	{
		arg = HexStrToTen(&mptzcmd[12], 2);
		return PTZ_ZOOM_OUT;
	}
	return PTZ_NO;
}

uint32_t PTZCMDType::GetPTZUDLREx(int& lrArg, int& udarg) //检查 是 ptz 的上下左右命令么 Arg 为 上下左右的数据
{
	//此处 28181 按照 char[2],中 char[0] 记录一字节的高4为（不是存 值 是存 转换过的数）

	int ntempCmd = HexStrToTen(mptzcmd[7]);

	lrArg = 0; udarg = 0;

	int isleft = PTZ_NO, isupstate = PTZ_NO;
	if ((ntempCmd & 1) == 1)//向右
	{
		isleft = PTZ_RIGHT;
		lrArg = HexStrToTen(&mptzcmd[8], 2);
	}
	else if (ntempCmd & (1 << 1))
	{
		isleft = PTZ_LEFT;
		lrArg = HexStrToTen(&mptzcmd[8], 2);
	}
	if ((ntempCmd & (1 << 2)) == (1 << 2))//像下
	{
		//return PTZ_DOWN_RIGHT;
		isupstate = PTZ_DOWN;
		udarg = HexStrToTen(&mptzcmd[10], 2);
	}
	else if (ntempCmd & (1 << 3)) //向上
	{
		isupstate = PTZ_UP;
		udarg = HexStrToTen(&mptzcmd[10], 2);
	}
	if (isleft > 0 && isupstate > 0)
	{
		//组合值
		if (isleft == PTZ_LEFT)
		{
			if (isupstate == PTZ_DOWN)
			{
				return PTZ_DOWN_LEFT;
			}
			else if (isupstate == PTZ_UP)
			{
				return PTZ_UP_LEFT;
			}
		}
		else if (isleft == PTZ_RIGHT)
		{
			if (isupstate == PTZ_DOWN)
			{
				return PTZ_DOWN_RIGHT;
			}
			else if (isupstate == PTZ_UP)
			{
				return PTZ_UP_RIGHT;
			}
		}
	}
	else {
		return isleft > PTZ_NO ? isleft : isupstate;
	}
	return 0;
}

uint32_t PTZCMDType::GetFIIrseCmd(int& arg)
{
	int ntempCmd = HexStrToTen(mptzcmd[7]);
	arg = HexStrToTen(&mptzcmd[8], 2);
	if (ntempCmd & 1)
	{
		return FOCUS_IN;
	}
	else if (ntempCmd & (1 << 1))
	{
		return FOCUS_OUT;
	}
	return PTZ_NO;
}
uint32_t PTZCMDType::GetFIFocusCmd(int& arg)
{
	int ntempCmd = HexStrToTen(mptzcmd[7]);
	arg = HexStrToTen(&mptzcmd[10], 2);
	if (ntempCmd & (1 << 2))
	{
		return IRIS_OUT;
	}
	else if (ntempCmd & (1 << 3))
	{
		return IRIS_IN;
	}
	return PTZ_NO;
}
uint32_t PTZCMDType::GetPresetCmd(int& arg)
{
	int ntempCmd = HexStrToTen(mptzcmd[6]);
	arg = HexStrToTen(&mptzcmd[10], 2);
	if (ntempCmd & (1 << 3))
	{
		int nsubcmd = HexStrToTen(mptzcmd[7]);
		if ((nsubcmd & 3) == 3)
		{
			return PTZ_MOVPRESET;
		}
		else  if ((nsubcmd & 1) == 1)
		{
			return PTZCMDType::PTZ_SETPRESET;
		}
		else if ((nsubcmd & 2) == 2)
		{
			return PTZ_GTOPRESET;
		}
	}
	return PTZ_NO;
	//arg = HexStrToTen(&mptzcmd[10],2);
}

bool PTZCMDType::isStop()
{
	if (strncmp(&mptzcmd[6], "00000000", 8) == 0)
	{
		return true;
	}
	return false;
}
bool PTZCMDType::AddPtzCmd(int pcmd, int arg)
{
	switch (pcmd)
	{
	case PTZ_STOP:
		memset(mptzcmd + 6, '0', 8);
		GetCmdParityBit();
		/*mptzcmd[6] = 0;
		mptzcmd[7] = 0;*/
		break;
	case PTZ_UP:
	{
		int ntempCmd = 0;
		mptzcmd[6] = TenIntToChar(ntempCmd);

		int nCmd = 0;
		nCmd = nCmd | (1 << 3);
		mptzcmd[7] = TenIntToChar(nCmd);

		TenIntToChar2(arg, &mptzcmd[10]);
		GetCmdParityBit();
	}
	break;
	case PTZ_DOWN:
	{
		int ntempCmd = 0;
		mptzcmd[6] = TenIntToChar(ntempCmd);

		int nCmd = HexStrToTen(mptzcmd[7]);
		nCmd = nCmd | (1 << 2);
		mptzcmd[7] = TenIntToChar(nCmd);

		TenIntToChar2(arg, &mptzcmd[10]);
		GetCmdParityBit();
	}
	break;
	case PTZ_LEFT:
	{
		int ntempCmd = 0;
		mptzcmd[6] = TenIntToChar(ntempCmd);

		int nCmd = HexStrToTen(mptzcmd[7]);
		nCmd = nCmd | (1 << 1);
		mptzcmd[7] = TenIntToChar(nCmd);

		TenIntToChar2(arg, &mptzcmd[8]);
		GetCmdParityBit();
	}
	break;
	case PTZ_RIGHT:
	{
		int ntempCmd = 0;
		mptzcmd[6] = TenIntToChar(ntempCmd);

		int nCmd = HexStrToTen(mptzcmd[7]);
		nCmd = nCmd | 1;
		mptzcmd[7] = TenIntToChar(nCmd);
		TenIntToChar2(arg, &mptzcmd[8]);
		GetCmdParityBit();
	}
	break;
	case PTZ_UP_LEFT:
	{
		int ntempCmd = 0;
		mptzcmd[6] = TenIntToChar(ntempCmd);

		int nCmd = 0;
		nCmd = nCmd | (1 << 3) | (1 << 1);
		mptzcmd[7] = TenIntToChar(nCmd);
		TenIntToChar2(arg, &mptzcmd[8]);
		TenIntToChar2(arg, &mptzcmd[10]);
		GetCmdParityBit();
	}
	break;
	case PTZ_UP_RIGHT:
	{
		int ntempCmd = 0;
		mptzcmd[6] = TenIntToChar(ntempCmd);

		int nCmd = 0;
		nCmd = nCmd | (1 << 3) | 1;
		mptzcmd[7] = TenIntToChar(nCmd);
		TenIntToChar2(arg, &mptzcmd[8]);
		TenIntToChar2(arg, &mptzcmd[10]);
		GetCmdParityBit();
	}
	break;
	case PTZ_DOWN_LEFT:
	{
		int ntempCmd = 0;
		mptzcmd[6] = TenIntToChar(ntempCmd);

		int nCmd = 0;
		nCmd = nCmd | (1 << 2) | (1 << 1);
		mptzcmd[7] = TenIntToChar(nCmd);
		TenIntToChar2(arg, &mptzcmd[8]);
		TenIntToChar2(arg, &mptzcmd[10]);
		GetCmdParityBit();
	}
	break;
	case PTZ_DOWN_RIGHT:
	{
		int ntempCmd = 0;
		mptzcmd[6] = TenIntToChar(ntempCmd);

		int nCmd = 0;
		nCmd = nCmd | (1 << 2) | 1;
		mptzcmd[7] = TenIntToChar(nCmd);
		TenIntToChar2(arg, &mptzcmd[8]);
		TenIntToChar2(arg, &mptzcmd[10]);
		GetCmdParityBit();
	}
	break;
	case PTZ_ZOOM_IN://ZOOM 大
	{
		int ntempCmd = 0;
		ntempCmd = ntempCmd | 1;

		mptzcmd[6] = TenIntToChar(ntempCmd);
		TenIntToChar2(arg, &mptzcmd[12]);
		GetCmdParityBit();
	}
	break;
	case PTZ_ZOOM_OUT:
	{
		int ntempCmd = 0;
		ntempCmd = ntempCmd | (1 << 1);

		mptzcmd[6] = TenIntToChar(ntempCmd);
		TenIntToChar2(arg, &mptzcmd[12]);
		GetCmdParityBit();
	}
	break;

	case FOCUS_IN://近
	{
		int ntempCmd = 0;
		ntempCmd = ntempCmd | (1 << 2);

		mptzcmd[6] = TenIntToChar(ntempCmd);

		int nCmd = 0;
		nCmd = nCmd | (1 << 1);
		mptzcmd[7] = TenIntToChar(nCmd);

		TenIntToChar2(arg, &mptzcmd[8]);
		GetCmdParityBit();
		break;
	}
	case FOCUS_OUT://
	{
		int ntempCmd = 0;
		ntempCmd = ntempCmd | (1 << 2);

		mptzcmd[6] = TenIntToChar(ntempCmd);

		int nCmd = 0;
		nCmd = nCmd | 1;
		mptzcmd[7] = TenIntToChar(nCmd);

		TenIntToChar2(arg, &mptzcmd[8]);
		GetCmdParityBit();
		break;
	}

	case IRIS_IN://XIAO
	{
		int ntempCmd = 0;
		ntempCmd = ntempCmd | (1 << 2);

		mptzcmd[6] = TenIntToChar(ntempCmd);

		int nCmd = 0;
		nCmd = nCmd | (1 << 3);
		mptzcmd[7] = TenIntToChar(nCmd);

		TenIntToChar2(arg, &mptzcmd[10]);
		GetCmdParityBit();
		break;
	}
	case IRIS_OUT://DA
	{

		int ntempCmd = 0;
		ntempCmd = ntempCmd | (1 << 2);

		mptzcmd[6] = TenIntToChar(ntempCmd);

		int nCmd = 0;
		nCmd = nCmd | (1 << 2);
		mptzcmd[7] = TenIntToChar(nCmd);

		TenIntToChar2(arg, &mptzcmd[10]);
		GetCmdParityBit();
		break;
	}

	case PTZ_SETPRESET:
	{
		int ntempCmd = 0;
		ntempCmd = ntempCmd | (1 << 3);
		mptzcmd[6] = TenIntToChar(ntempCmd);

		int nCmd = 0;
		nCmd = nCmd | 1;
		mptzcmd[7] = TenIntToChar(nCmd);

		TenIntToChar2(arg, &mptzcmd[10]);
		GetCmdParityBit();
		break;
	}
	case PTZ_GTOPRESET:
	{
		int ntempCmd = 0;
		ntempCmd = ntempCmd | (1 << 3);
		mptzcmd[6] = TenIntToChar(ntempCmd);

		int nCmd = 0;
		nCmd = nCmd | 2;
		mptzcmd[7] = TenIntToChar(nCmd);

		TenIntToChar2(arg, &mptzcmd[10]);
		GetCmdParityBit();
		break;
	}
	case PTZ_DELPRESET:
	{
		int ntempCmd = 0;
		ntempCmd = ntempCmd | (1 << 3);
		mptzcmd[6] = TenIntToChar(ntempCmd);

		int nCmd = 0;
		nCmd = nCmd | 3;
		mptzcmd[7] = TenIntToChar(nCmd);

		TenIntToChar2(arg, &mptzcmd[10]);
		GetCmdParityBit();
		break;
	}
	case PTZ_MOVPRESET:
	{
		int ntempCmd = HexStrToTen(mptzcmd[6]);
		ntempCmd = ntempCmd | (1 << 3);

		int nCmd = HexStrToTen(mptzcmd[7]);
		nCmd = nCmd & 3;
		mptzcmd[7] = TenIntToChar(nCmd);

		TenIntToChar2(arg, &mptzcmd[10]);
		break;
	}
	}
	return true;
}