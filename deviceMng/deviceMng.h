#pragma once
#include <map>
#include <string>
#include <mutex>
#include <vector>
#include "BaseDevice.h"
#include "VirtualOrganization.h"


class DeviceMng
{
public:
	using GMUTEX = std::lock_guard<std::mutex>;
	~DeviceMng();

	static DeviceMng& Instance();

	void addDevice(BaseDevice::Ptr);
	void removeDevice(std::string);
	BaseDevice::Ptr findDevice(std::string Id);

	void addChildDevice(BaseChildDevice*);
	void removeChildDevice(std::string);
	BaseChildDevice* findChildDevice(std::string Id);
	void getChildDevice(std::string Id, std::vector<BaseChildDevice*> &vcList);

	void addVirtualOrganization(VirtualOrganization);
	void removeVirtualOrganization(std::string);
	VirtualOrganization* findVirtualOrganization(std::string Id);
	void getVirtualOrganization(std::string Id, std::vector<VirtualOrganization> &vcList);
private:
	std::mutex devMtx;
	std::map<std::string, BaseDevice::Ptr> mDeviceMap;
	std::mutex childMtx;
	std::map<std::string, BaseChildDevice*> mChildMap;
	std::mutex voMtx;
	std::map<std::string, VirtualOrganization> mVoMap;
};