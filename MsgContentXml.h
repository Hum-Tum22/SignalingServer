#ifndef MSG_CONTENT_XML_H_
#define MSG_CONTENT_XML_H_
#include "tinyxml2.h"
#include "uainfo.h"
#include "deviceMng/VirtualOrganization.h"


#include <string>
#include <vector>

using namespace tinyxml2;

//Message Request
//XML_CMD_NAME_CONTROL,//表示一个控制的动作
bool CreatelPTZControCmd(char* deviceId, const char* ptzCmd, int ControlPriority, unsigned int sn, std::string& outstr);


//XML_CMD_NAME_QUERY,//表示一个查询的动作
//Subscription
bool CreateCatalogSubscriptionResponseMsg(const char* user, const uint32_t& sn, std::string& outstr);
//Request
bool CreateCatalogQueryRequestMsg(const char* gbid, const uint32_t& sn, std::string& outstr);


//XML_CMD_NAME_NOTIFY,//表示一个通知的动作
//keeplive
bool CreateKeepAliveMsg(const char* user, const uint32_t& sn, std::string& outstr);



////应答命令
//XML_CMD_NAME_RESPONSE,//表示一个请求动作的应答

//Response
//catalog 响应
bool CreateCatalogResponse(const char* user, const uint32_t& sn, int SumNum, std::vector<CatalogItem> Items, CatalogItemExpandInfo* pExpand, std::string& outstr);	//pExpand摄相机使用
void AddDeviceItemToCatalog(tinyxml2::XMLDocument& doc, XMLElement* DeviceListElement, const CatalogItem& item);
void AddIPCInfoToCatalog(tinyxml2::XMLDocument& doc, XMLElement* DeviceListElement, const CatalogItemExpandInfo* expand);

//Virtual organization
bool CreateVirtualOrganizationCatalogResponse(const char* user, const uint32_t& sn, int SumNum, std::vector<VirtualOrganization> Items, std::string& outstr);
void AddVirtualOrganizationToCatalog(tinyxml2::XMLDocument& doc, XMLElement* DeviceListElement, const VirtualOrganization& item);
//设备信息响应
bool CreateDeviceInfoResponse(const char* DeviceId, const uint32_t& sn, const DeviceInfoMsg& devInfoMsg, std::string& outstr);


#endif