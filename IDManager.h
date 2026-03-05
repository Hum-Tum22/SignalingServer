#pragma once
#include <set>
#include <iostream>
#include <mutex>
#include <map>
template <class T>
class CHnaldeMrg
{
public:
	CHnaldeMrg(T ui_start,T uimax):m_uistart(ui_start),m_uiMax(uimax),CurIncrHandle(ui_start){};
	virtual ~CHnaldeMrg(){};
	T GetHandle(){
		CurIncrHandle++;
		if(CurIncrHandle>=m_uiMax)
		{
			CurIncrHandle = m_uistart;
		}
		return CurIncrHandle;
	};
	T GetHandle_R(){
		while(m_handlemap.count(CurIncrHandle) != 0)
		{
				if(CurIncrHandle>=m_uiMax)
				{
					CurIncrHandle = m_uistart;
				}
				CurIncrHandle++;
		}
		m_handlemap.insert(CurIncrHandle);
		std::cout << "GetHandle_R " << " CurIncrHandle:" << CurIncrHandle << std::endl;
		return CurIncrHandle;
	}
	T GetGBCodeSeq_R(T h){
		T nistat = h;
		while(m_handlemap.count(nistat) != 0)
		{
            if(nistat > (0x98967E + h))		//98967E == 9999999
            {
                return 0;
            }
            nistat++;
		}
		m_handlemap.insert(nistat);
		return nistat;
	}
	void AddHandle_R(T h)
	{
		// std::cout << "AddHandle_R h:" << h << " CurIncrHandle:" << CurIncrHandle << std::endl;
		m_handlemap.insert(h);
	}
	bool ReleasHandle(T h){
		m_handlemap.erase(h);
		if(CurIncrHandle > h)
		{
			CurIncrHandle = h;
		}
		return true;
	}
	int Size()
	{
		return m_handlemap.size();
	}
protected:
	T m_uistart;
	T m_uiMax;
	T CurIncrHandle;
	std::set<T> m_handlemap;
};
#define DEVCODE_DOMAIN_LEN 10
enum EM_Node_Type
{
	em_node_unknown = 0,//未知类型, 暂不使用
    DVR_CODE = 111,//DVR
	NVR_CODE = 118,//NVR	
	IPC_CODE = 132,//IPC
	CAMERA_CODE = 131,//摄像机		没有使用到
    CMS_CODE = 200,
	HARDWARE_MEDIASERVER = 214,//硬媒体
	UNIT_CODE = 216,//组织机构	
    MEDIADECODESVR_CODE = 114,//解码器
	MEDIAENCODESVR_CODE = 113,//编码器
	GROUP_CODE = 215,//分组
	ALARMCONTROL_CODE = 117,//报警主机
    EXT_CODE = 217,//扩展的平台设备类型 cms中未知设备类型 
	em_node_type_max,	// 节点类型数量
};
class CDevCodeMng
{
	using DLLINT = unsigned long long;
	std::mutex m_DevCodeMutex;
	std::set<std::string> m_GBIDList;
	std::string strLocalGBID;
	std::mutex m_CodeSeqMutex;
	std::map<int, CHnaldeMrg<DLLINT>*> mDevCodeSeq;
public:
	CDevCodeMng(std::string localGbId);
	~CDevCodeMng();
    static CDevCodeMng& Instance();
	void AddDevCodeID(const std::string &gbid);
	void DelDevCodeID(const std::string &gbid, int reson);
	bool IsDevCodeExist(const std::string &gbid);
	std::string CreateDevCode(const int &DevType);
	void SetLocalGBID(const std::string &localgbid);
	int GetDeviceType(const std::string gbId);
};