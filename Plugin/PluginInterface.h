#pragma once

#include <string>
#include "PluginMacro.h"

class PluginInter
{
public:
    typedef enum
    {
        JSON_SDK,
    }InterProtocal;
    PluginInter(InterProtocal type) :pluginType(type) {};
    InterProtocal pluginType;
};

class IPlugin
{
public:
    // IPlugin() :m_hDll(nullptr) {};
    virtual int Init() = 0;
    virtual int UnInit() = 0;
    virtual int addDeviceInfo(std::string ip, ushort port, std::string user, std::string pswd) = 0;
    virtual int deleteDeviceInfo(std::string ip) = 0;
    virtual int getChannelInfo(std::string ip, std::string& channels) = 0;
    virtual int registDataCallBack(void* cb, void* cbData) = 0;
    virtual int startLiveStream(std::string ip, int channel, int streamType) = 0;
    virtual int stopLiveStream(int streamId) = 0;
    virtual int getRecordInfoList(std::string ip, int chl, uint64_t st, uint64_t et, std::string& recordInfos) = 0;
protected:
    PlHandle m_hDll;
};



/*==========================================================================
 *  FUNCTION		 : VSK_Plugin_Initate
 *  IN              : NONE
 *  OUT             : NONE
 *  RETURN          : 成功返回true,失败返回false;
 *  INFO            : 初始化插件;其他接口调用的前提。
 *=========================================================================*/
VSK_PLUGIN_API bool CALL_METHOD VSK_Plugin_Initate();

/*==========================================================================
 *  FUNCTION		: VSK_Plugin_Clear
 *  IN              : NONE
 *  OUT             : NONE
 *  RETURN          : 成功返回true,失败返回false;
 *  INFO            : 释放插件资源
 *=========================================================================*/
VSK_PLUGIN_API bool CALL_METHOD VSK_Plugin_Clear();
/*==========================================================================
 *  FUNCTION		: VSK_Plugin_addDeviceInfo
 *  IN              : ip;                               : 设备IP
 *  IN              : port;                             : 设备私有协议port
 *  IN              : user;                             : 设备登录用户名
 *  IN              : pswd;                             : 设备登录密码
 *  OUT             : NONE
 *  RETURN          : 成功返回true,失败返回false;
 *  INFO            : 添加设备到插件
 *=========================================================================*/
VSK_PLUGIN_API bool CALL_METHOD VSK_Plugin_addDeviceInfo(const char* ip, ushort port, const char* user, const char* pswd);
/*==========================================================================
 *  FUNCTION		: VSK_Plugin_removeDeviceInfo
 *  IN              : ip;                               : 设备IP
 *  OUT             : NONE
 *  RETURN          : 成功返回true,失败返回false;
 *  INFO            : 从插件中移除设备
 *=========================================================================*/
VSK_PLUGIN_API bool CALL_METHOD VSK_Plugin_removeDeviceInfo(const char* ip);
/*==========================================================================
 *  FUNCTION		: VSK_Plugin_free
 *  IN              : buffer;                           : 需要释放的指针
 *  OUT             : NONE
 *  RETURN          : NONE
 *  INFO            : 释放接口中申请的资源
 *=========================================================================*/
VSK_PLUGIN_API void CALL_METHOD VSK_Plugin_free(void** buffer);
/*==========================================================================
 *  FUNCTION		: VSK_Plugin_getChannelInfo
 *  IN              : ip;                               : 设备IP
 *  OUT             : pOutBuffer;                       : 返回结果buffer由接口内部申请, 由VSK_Plugin_free释放
 *  OUT             : nOutSize;                         : 返回结果长度
 *  RETURN          : 成功返回true,失败返回false;
 *  INFO            : 获取设备的通道列表
 *=========================================================================*/
VSK_PLUGIN_API bool CALL_METHOD VSK_Plugin_getChannelInfo(const char* ip, char** pOutBuffer, int* nOutSize);
/*==========================================================================
 *  FUNCTION		: VSK_Plugin_getChannelInfo
 *  IN              : ip;                               : 设备IP
 *  OUT             : pOutBuffer;                       : 返回结果buffer由接口内部申请, 由VSK_Plugin_free释放
 *  OUT             : nOutSize;                         : 返回结果长度
 *  RETURN          : 成功返回true,失败返回false;
 *  INFO            : 获取设备的通道列表
 *=========================================================================*/
VSK_PLUGIN_API bool CALL_METHOD VSK_Plugin_LiveStream(const char* ip, int channel, int streamType);