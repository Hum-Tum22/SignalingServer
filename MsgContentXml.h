#ifndef MSG_CONTENT_XML_H_
#define MSG_CONTENT_XML_H_
#include "tinyxml2.h"
#include "uainfo.h"
#include "deviceMng/VirtualOrganization.h"


#include <string>
#include <vector>

using namespace tinyxml2;

//Message Request
//XML_CMD_NAME_CONTROL,//��ʾһ�����ƵĶ���
bool CreatelPTZControCmd(char* deviceId, const char* ptzCmd, int ControlPriority, unsigned int sn, std::string& outstr);


//XML_CMD_NAME_QUERY,//��ʾһ����ѯ�Ķ���
//Subscription
bool CreateCatalogSubscriptionResponseMsg(const char* user, const uint32_t& sn, std::string& outstr);
//Request
bool CreateCatalogQueryRequestMsg(const char* gbid, const uint32_t& sn, std::string& outstr);


//XML_CMD_NAME_NOTIFY,//��ʾһ��֪ͨ�Ķ���
//keeplive
bool CreateKeepAliveMsg(const char* user, const uint32_t& sn, std::string& outstr);



////Ӧ������
//XML_CMD_NAME_RESPONSE,//��ʾһ����������Ӧ��

//Response
//catalog ��Ӧ
bool CreateCatalogResponse(const char* user, const uint32_t& sn, int SumNum, std::vector<CatalogItem> Items, CatalogItemExpandInfo* pExpand, std::string& outstr);	//pExpand�����ʹ��
void AddDeviceItemToCatalog(tinyxml2::XMLDocument& doc, XMLElement* DeviceListElement, const CatalogItem& item);
void AddIPCInfoToCatalog(tinyxml2::XMLDocument& doc, XMLElement* DeviceListElement, const CatalogItemExpandInfo* expand);

//Virtual organization
bool CreateVirtualOrganizationCatalogResponse(const char* user, const uint32_t& sn, int SumNum, std::vector<VirtualOrganization> Items, std::string& outstr);
void AddVirtualOrganizationToCatalog(tinyxml2::XMLDocument& doc, XMLElement* DeviceListElement, const VirtualOrganization& item);
//�豸��Ϣ��Ӧ
bool CreateDeviceInfoResponse(const char* DeviceId, const uint32_t& sn, const DeviceInfoMsg& devInfoMsg, std::string& outstr);


#endif