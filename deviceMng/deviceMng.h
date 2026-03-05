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


	void setSelfId(const std::string &myId);
	const std::string getSelfId();

	void addDevice(BaseDevice::Ptr);
	void removeDevice(std::string);
	BaseDevice::Ptr findDevice(const std::string Id);

	void addChildDevice(std::shared_ptr<BaseChildDevice>);
	void removeChildDevice(std::string);
    std::shared_ptr<BaseChildDevice> findChildDevice(const std::string Id);
    std::shared_ptr<BaseChildDevice> findChildDeviceByGBID(const std::string gbId);
    void getChildDevice(const std::string& Id, std::vector<std::shared_ptr<BaseChildDevice>>& vcList);
	std::shared_ptr<BaseChildDevice> findChildDeviceByCCTVDeviceId(const std::string Id);

	void addVirtualOrganization(VirtualOrganization);
	void removeVirtualOrganization(std::string);
	VirtualOrganization* findVirtualOrganization(std::string Id);
	void getVirtualOrganization(const std::string &Id, std::vector<VirtualOrganization> &vcList);
private:
	std::string selfId;
	std::mutex devMtx;
	std::map<std::string, BaseDevice::Ptr> mDeviceMap;
	std::mutex childMtx;
	std::map<std::string, std::shared_ptr<BaseChildDevice>> mChildMap;
	std::mutex voMtx;
	std::map<std::string, VirtualOrganization> mVoMap;
};