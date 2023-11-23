#include "deviceMng.h"

DeviceMng& DeviceMng::Instance()
{
	static DeviceMng *g_DeviceMng = new DeviceMng();
	return *g_DeviceMng;
}
DeviceMng::~DeviceMng()
{
	{
		GMUTEX lock(childMtx);
		for (auto& iter : mChildMap)
		{
			delete iter.second;
			iter.second = NULL;
		}
		mChildMap.clear();
	}
	{
		GMUTEX lock(devMtx);
		/*for (auto &iter : mDeviceMap)
		{
			delete iter.second;
			iter.second = NULL;
		}*/
		mDeviceMap.clear();
	}
	
}
void DeviceMng::setSelfId(const std::string& myId)
{
	selfId = myId;
}
void DeviceMng::addDevice(BaseDevice::Ptr dev)
{
	GMUTEX lock(devMtx);
	mDeviceMap[dev->deviceId] = dev;
}
void DeviceMng::removeDevice(std::string Id)
{
	GMUTEX lock(devMtx);
	mDeviceMap.erase(Id);
}
BaseDevice::Ptr DeviceMng::findDevice(std::string Id)
{
	GMUTEX lock(devMtx);
	auto it = mDeviceMap.find(Id);
	if (it != mDeviceMap.end())
	{
		return it->second;
	}
	return NULL;
}

void DeviceMng::addChildDevice(BaseChildDevice* child)
{
	GMUTEX lock(childMtx);
	mChildMap[child->getDeviceId()] = child;
}
void DeviceMng::removeChildDevice(std::string Id)
{
	GMUTEX lock(childMtx);
	mChildMap.erase(Id);
}
BaseChildDevice* DeviceMng::findChildDevice(std::string Id)
{
	GMUTEX lock(childMtx);
	auto it = mChildMap.find(Id);
	if (it != mChildMap.end())
	{
		return it->second;
	}
	return NULL;
}
void DeviceMng::getChildDevice(const std::string &Id, std::vector<BaseChildDevice*> &vcList)
{
	if (Id.empty())
	{
		GMUTEX lock(childMtx);
		for (auto& it : mChildMap)
		{
			vcList.push_back(it.second);
		}
	}
	else
	{
		if (selfId == Id)
		{
			getChildDevice("", vcList);
		}
		else
		{
			std::map<std::string, BaseChildDevice*> ChildMap;
			{
				GMUTEX lock(childMtx);
				ChildMap = mChildMap;
			}
			for (auto& it : ChildMap)
			{
				if (it.second && it.second->getParentDev()->deviceId == Id)
				{
					vcList.push_back(it.second);
					getChildDevice(it.second->getDeviceId(), vcList);
				}
			}
		}
	}
}

void DeviceMng::addVirtualOrganization(VirtualOrganization vo)
{
	GMUTEX lock(childMtx);
	mVoMap[vo.DeviceID] = vo;
}
void DeviceMng::removeVirtualOrganization(std::string Id)
{
	GMUTEX lock(childMtx);
	mVoMap.erase(Id);
}
VirtualOrganization* DeviceMng::findVirtualOrganization(std::string Id)
{
	GMUTEX lock(childMtx);
	auto it = mVoMap.find(Id);
	if (it != mVoMap.end())
	{
		return &it->second;
	}
	return NULL;
}
void DeviceMng::getVirtualOrganization(const std::string &Id, std::vector<VirtualOrganization> &vcList)
{
	if (Id.empty())
	{
		GMUTEX lock(childMtx);
		for (auto& it : mVoMap)
		{
			vcList.push_back(it.second);
		}
	}
	else
	{
		if (selfId == Id)
		{
			getVirtualOrganization("", vcList);
		}
		else
		{
			GMUTEX lock(childMtx);
			for (auto& it : mVoMap)
			{
				if (it.second.ParentID == Id)
				{
					vcList.push_back(it.second);
					getVirtualOrganization(it.second.DeviceID, vcList);
				}
			}
		}
	}
}