#ifndef MSG_CONTENT_XML_H_
#define MSG_CONTENT_XML_H_
#include "tinyxml2.h"
#include "uainfo.h"


#include <string>
#include <vector>

using namespace tinyxml2;
using namespace std;
//Message Request


//keeplive
bool CreateKeepAliveMsg(const char* user, const uint32_t &sn, string& outstr);

//Response
//catalog 响应
bool CreateCatalogResponse(const char* user, const uint32_t& sn, int SumNum, vector<CatalogItem> Items, CatalogItemExpandInfo *pExpand, string& outstr);	//pExpand摄相机使用
void AddDeviceItemToCatalog(tinyxml2::XMLDocument& doc, XMLElement* DeviceListElement, const CatalogItem& item);
void AddIPCInfoToCatalog(tinyxml2::XMLDocument& doc, XMLElement* DeviceListElement, const CatalogItemExpandInfo* expand);

//设备信息响应
bool CreateDeviceInfoResponse(const char* DeviceId, const uint32_t& sn, const DeviceInfoMsg& devInfoMsg, string& outstr);



//Subscription
bool CreateCatalogSubscriptionResponseMsg(const char* user, const uint32_t& sn, string& outstr);


//Request
bool CreateCatalogQueryRequestMsg(const char* gbid, const uint32_t& sn, string& outstr);
#endif