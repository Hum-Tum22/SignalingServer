#include "XmlMsgAnalysis.h"
#include "tools/CodeConversion.h"

GB28181XmlMsg::GB28181XmlMsg()
{
	cmdname = XML_CMD_NAME_UNKNOWN;
	cmdtype = XML_CMDTYPE_UNKNOWN;
	controlCmd = XML_CONTROLCMD_UNKNOWN;
	sn = 0;
	pPoint = NULL;
}
GB28181XmlMsg::~GB28181XmlMsg()
{
	if (pPoint)
	{
		//控制命令

		switch (cmdtype)
		{
		case XML_CMDTYPE_DEVICE_CONTROL:
		{
			switch (controlCmd)
			{
			case XML_CONTROLCMD_PTZ:
			{
				delete (PtzControlInfo*)pPoint;
				pPoint = NULL;
				break;
			}
			case XML_CONTROLCMD_TELEBOOT:
			{
				delete (BootControlInfo*)pPoint;
				pPoint = NULL;
				break;
			}
			case XML_CONTROLCMD_RECORD:
			{
				delete (RecordControlInfo*)pPoint;
				pPoint = NULL;
				break;
			}
			case XML_CONTROLCMD_GUARD:
			{
				delete (GuardControlInfo*)pPoint;
				pPoint = NULL;
				break;
			}
			case XML_CONTROLCMD_ALARM:
			{
				delete (AlarmControlInfo*)pPoint;
				pPoint = NULL;
				break;
			}
			case XML_CONTROLCMD_IFRAME:
			{
				delete (IFameControlInfo*)pPoint;
				pPoint = NULL;
				break;
			}
			case XML_CONTROLCMD_DRAGZOOMIN:
			case XML_CONTROLCMD_DRAGZOOMOUT:
			{
				delete (DragZoomControlInfo*)pPoint;
				pPoint = NULL;
				break;
			}
			case XML_CONTROLCMD_HOMEPOSITION:
			{
				delete (HomePositionControlInfo*)pPoint;
				pPoint = NULL;
				break;
			}
			default:
				break;
			}
		}
		case XML_CMDTYPE_DEVICE_CONFIG:
		{
			delete (DeviceConfigInfo*)pPoint;
			pPoint = NULL;
			break;
		}
		case XML_CMDTYPE_DEVICE_STATUS:
		{
			break;
		}
		case XML_CMDTYPE_CATALOG:
		{
			delete (CatalogQueryMsg*)pPoint;
			pPoint = NULL;
			break;
		}
		case XML_CMDTYPE_DEVICE_INFO:
		case XML_CMDTYPE_RECORDINFO:
		case XML_CMDTYPE_ALARM:
		case XML_CMDTYPE_CONFIG_DOWNLOAD:
		case XML_CMDTYPE_PRESET_QUERY:
		case XML_CMDTYPE_MOBILE_POSITION:
		{
			break;
		}
		case XML_CMDTYPE_NOTIFY_KEEPALIVE:
		{
			delete (KeepAliveMsg*)pPoint;
			pPoint = NULL;
			break;
		}
		case XML_CMDTYPE_NOTIFY_ALARM:
		case XML_CMDTYPE_NOTIFY_MEDIA_STATUS:
		case XML_CMDTYPE_NOTIFY_BROADCAST:
		case XML_CMDTYPE_NOTIFY_MOBILE_POSITION:
		case XML_CMDTYPE_NOTIFY_CATALOG:
		case XML_CMDTYPE_RESPONSE_DEV_CONTROL:
		case XML_CMDTYPE_RESPONSE_ALARM:
		{
			break;
		}
		case XML_CMDTYPE_RESPONSE_CATALOG_ITEM:
		{
			delete (ResponseCatalogList*)pPoint;
			pPoint = NULL;
			break;
		}
		case XML_CMDTYPE_RESPONSE_CATALOG:
		case XML_CMDTYPE_RESPONSE_CATALOG_RECEIVED:
		case XML_CMDTYPE_RESPONSE_DEV_INFO:
		case XML_CMDTYPE_RESPONSE_DEV_STATUS:
		case XML_CMDTYPE_RESPONSE_RECORD_INFO:
		case XML_CMDTYPE_RESPONSE_DEV_CONFIG:
		case XML_CMDTYPE_RESPONSE_CONFIG_DOWNLOAD:
		case XML_CMDTYPE_RESPONSE_PRESET_QUERY:
		case XML_CMDTYPE_RESPONSE_BROADCAST:
		{
		}

		
		break;
		/*case XML_CMDTYPE_REQUEST_CATALOG_SUBSCRIPTION:
		{
			delete (CatalogSubscriptionMsg*)pPoint;
			pPoint = NULL;
		}*/
		break;
		default:
			break;
		}
	}
}
bool AnalyzeReceivedSipMsg(const char* MsgStr, GB28181XmlMsg& XmlMsg)
{
	XMLDocument mDocument;
	mDocument.Clear();
	XMLError err = mDocument.Parse(MsgStr);
	if (err == XML_SUCCESS)
	{
		XMLElement* pRootNode = mDocument.RootElement();
		if (!pRootNode)
		{
			return false;
		}
		const char* CmdName = pRootNode->Value();
		if (!CmdName)
			return false;

		//Control 表示一个控制的动作
		//Query 表示一个查询的动作
		//Notify 表示一个通知的动作
		//应答命令 Response 表示一个请求动作的应答
		if (strncmp(CmdName, "Control", 7) == 0)
		{
			XmlMsg.cmdname = XML_CMD_NAME_CONTROL;
			XMLElement* CmdTypeElement = pRootNode->FirstChildElement("CmdType");
			if (!CmdTypeElement)
				return false;

			XMLElement* SNElement = pRootNode->FirstChildElement("SN");
			if (!SNElement)
				return false;
			XmlMsg.sn = atoi(SNElement->GetText());

			XMLElement* DeviceIDElement = pRootNode->FirstChildElement("DeviceID");
			if (!DeviceIDElement)
				return false;
			XmlMsg.DeviceID = DeviceIDElement->GetText();

			const char* CmdType = CmdTypeElement->GetText();
			if (!CmdType)
				return false;

			if (strncmp(CmdType, "DeviceControl", 13) == 0)
			{
				XmlMsg.cmdtype = XML_CMDTYPE_DEVICE_CONTROL;

				XMLElement* controlName = pRootNode->FirstChildElement("PTZCmd");
				if (controlName)
				{
					PtzControlInfo* info = new PtzControlInfo();
					XmlMsg.pPoint = info;
					XmlMsg.controlCmd = XML_CONTROLCMD_PTZ;
					info->value = controlName->Value();
					XMLElement* infoElement = pRootNode->FirstChildElement("info");
					if (infoElement)
					{
						XMLElement* PriorityElement = infoElement->FirstChildElement("ControlPriority");
						if (PriorityElement)
						{
							info->ControlPriority = std::stoi(PriorityElement->Value());
						}
					}
					return true;
				}
				controlName = pRootNode->FirstChildElement("TeleBoot");
				if (controlName)
				{
					BootControlInfo* info = new BootControlInfo();
					XmlMsg.pPoint = info;
					XmlMsg.controlCmd = XML_CONTROLCMD_TELEBOOT;
					info->value = controlName->Value();
					return true;
				}
				controlName = pRootNode->FirstChildElement("RecordCmd");
				if (controlName)
				{
					RecordControlInfo* info = new RecordControlInfo();
					XmlMsg.pPoint = info;
					XmlMsg.controlCmd = XML_CONTROLCMD_RECORD;
					info->value = controlName->Value();
					return true;
				}
				controlName = pRootNode->FirstChildElement("GuardCmd");
				if (controlName)
				{
					GuardControlInfo* info = new GuardControlInfo();
					XmlMsg.pPoint = info;
					XmlMsg.controlCmd = XML_CONTROLCMD_GUARD;
					info->value = controlName->Value();
					return true;
				}
				controlName = pRootNode->FirstChildElement("AlarmCmd");
				if (controlName)
				{
					AlarmControlInfo* info = new AlarmControlInfo();
					XmlMsg.pPoint = info;
					XmlMsg.controlCmd = XML_CONTROLCMD_ALARM;
					info->value = controlName->Value();
					return true;
				}
				controlName = pRootNode->FirstChildElement("IFameCmd");
				if (controlName)
				{
					IFameControlInfo* info = new IFameControlInfo();
					XmlMsg.pPoint = info;
					XmlMsg.controlCmd = XML_CONTROLCMD_IFRAME;
					info->value = controlName->Value();
					return true;
				}
				controlName = pRootNode->FirstChildElement("DragZoomIn");
				if (controlName)
				{
					DragZoomControlInfo* info = new DragZoomControlInfo();
					XmlMsg.pPoint = info;
					XmlMsg.controlCmd = XML_CONTROLCMD_DRAGZOOMIN;
					info->value = controlName->Value();
					return true;
				}
				controlName = pRootNode->FirstChildElement("DragZoomOut");
				if (controlName)
				{
					DragZoomControlInfo* info = new DragZoomControlInfo();
					XmlMsg.pPoint = info;
					XmlMsg.controlCmd = XML_CONTROLCMD_DRAGZOOMOUT;
					info->value = controlName->Value();
					return true;
				}
				controlName = pRootNode->FirstChildElement("HomePosition");
				if (controlName)
				{
					HomePositionControlInfo* info = new HomePositionControlInfo();
					XmlMsg.pPoint = info;
					XmlMsg.controlCmd = XML_CONTROLCMD_HOMEPOSITION;
					info->value = controlName->Value();
					return true;
				}
				return false;
			}
			else if (strncmp(CmdType, "DeviceConfig", 12) == 0)
			{
				XmlMsg.cmdtype = XML_CMDTYPE_DEVICE_CONFIG;

				DeviceConfigInfo* info = new DeviceConfigInfo();
				XmlMsg.pPoint = info;
				//info->value = controlName->Value();
				return true;
			}
			else
			{
				XmlMsg.cmdtype = XML_CMDTYPE_UNKNOWN;
				return false;
			}

		}
		else if (strncmp(CmdName, "Query", 5) == 0)
		{
			XmlMsg.cmdname = XML_CMD_NAME_CONTROL;
			XMLElement* CmdTypeElement = pRootNode->FirstChildElement("CmdType");
			if (!CmdTypeElement)
				return false;
			const char* CmdType = CmdTypeElement->GetText();
			if (!CmdType)
				return false;
			if (strncmp(CmdType, "DeviceStatus", 12) == 0)
			{
				XmlMsg.cmdtype = XML_CMDTYPE_DEVICE_CONTROL;
				return true;
			}
			else if (strncmp(CmdType, "Catalog", 7) == 0)
			{
				XmlMsg.cmdtype = XML_CMDTYPE_DEVICE_CONFIG;
				return true;
			}
			else if (strncmp(CmdType, "DeviceInfo", 10) == 0)
			{
				XmlMsg.cmdtype = XML_CMDTYPE_DEVICE_CONFIG;
				return true;
			}
			else if (strncmp(CmdType, "RecordInfo", 10) == 0)
			{
				XmlMsg.cmdtype = XML_CMDTYPE_DEVICE_CONFIG;
				return true;
			}
			else if (strncmp(CmdType, "Alarm", 5) == 0)
			{
				XmlMsg.cmdtype = XML_CMDTYPE_DEVICE_CONFIG;
				return true;
			}
			else if (strncmp(CmdType, "ConfigDownload", 14) == 0)
			{
				XmlMsg.cmdtype = XML_CMDTYPE_DEVICE_CONFIG;
				return true;
			}
			else if (strncmp(CmdType, "PresetQuery", 11) == 0)
			{
				XmlMsg.cmdtype = XML_CMDTYPE_DEVICE_CONFIG;
				return true;
			}
			else if (strncmp(CmdType, "MobilePosition", 14) == 0)
			{
				XmlMsg.cmdtype = XML_CMDTYPE_DEVICE_CONFIG;
				return true;
			}
			else
			{
				XmlMsg.cmdtype = XML_CMDTYPE_UNKNOWN;
				return false;
			}
		}
		else if (strncmp(CmdName, "Notify", 6) == 0)
		{
			XmlMsg.cmdname = XML_CMD_NAME_CONTROL;
			XMLElement* CmdTypeElement = pRootNode->FirstChildElement("CmdType");
			if (!CmdTypeElement)
				return false;
			const char* CmdType = CmdTypeElement->GetText();
			if (!CmdType)
				return false;
			if (strncmp(CmdType, "Keepalive", 9) == 0)
			{
				XmlMsg.cmdtype = XML_CMDTYPE_DEVICE_CONTROL;
				return true;
			}
			else if (strncmp(CmdType, "Alarm", 5) == 0)
			{
				XmlMsg.cmdtype = XML_CMDTYPE_DEVICE_CONFIG;
				return true;
			}
			else if (strncmp(CmdType, "MediaStatus", 11) == 0)
			{
				XmlMsg.cmdtype = XML_CMDTYPE_DEVICE_CONFIG;
				return true;
			}
			else if (strncmp(CmdType, "Broadcast", 9) == 0)
			{
				XmlMsg.cmdtype = XML_CMDTYPE_DEVICE_CONFIG;
				return true;
			}
			else if (strncmp(CmdType, "MobilePosition", 14) == 0)
			{
				XmlMsg.cmdtype = XML_CMDTYPE_DEVICE_CONFIG;
				return true;
			}
			else if (strncmp(CmdType, "Catalog", 7) == 0)
			{
				XmlMsg.cmdtype = XML_CMDTYPE_DEVICE_CONFIG;
				return true;
			}
			else
			{
				XmlMsg.cmdtype = XML_CMDTYPE_UNKNOWN;
				return false;
			}
		}
		else if (strncmp(CmdName, "Response", 8) == 0)
		{
			XmlMsg.cmdname = XML_CMD_NAME_CONTROL;
			XMLElement* CmdTypeElement = pRootNode->FirstChildElement("CmdType");
			if (!CmdTypeElement)
				return false;
			const char* CmdType = CmdTypeElement->GetText();
			if (!CmdType)
				return false;
			if (strncmp(CmdType, "DeviceControl", 13) == 0)
			{
				XmlMsg.cmdtype = XML_CMDTYPE_DEVICE_CONTROL;
				return true;
			}
			else if (strncmp(CmdType, "Alarm", 5) == 0)
			{
				XmlMsg.cmdtype = XML_CMDTYPE_DEVICE_CONFIG;
				return true;
			}
			else if (strncmp(CmdType, "Catalog", 7) == 0)
			{
				XmlMsg.cmdtype = XML_CMDTYPE_DEVICE_CONFIG;
				return true;
			}
			else if (strncmp(CmdType, "DeviceInfo", 10) == 0)
			{
				XmlMsg.cmdtype = XML_CMDTYPE_DEVICE_CONFIG;
				return true;
			}
			else if (strncmp(CmdType, "DeviceStatus", 12) == 0)
			{
				XmlMsg.cmdtype = XML_CMDTYPE_DEVICE_CONFIG;
				return true;
			}
			else if (strncmp(CmdType, "RecordInfo", 10) == 0)
			{
				XmlMsg.cmdtype = XML_CMDTYPE_DEVICE_CONFIG;
				return true;
			}
			else if (strncmp(CmdType, "DeviceConfig", 12) == 0)
			{
				XmlMsg.cmdtype = XML_CMDTYPE_DEVICE_CONFIG;
				return true;
			}
			else if (strncmp(CmdType, "ConfigDownload", 14) == 0)
			{
				XmlMsg.cmdtype = XML_CMDTYPE_DEVICE_CONFIG;
				return true;
			}
			else if (strncmp(CmdType, "PresetQuery", 11) == 0)
			{
				XmlMsg.cmdtype = XML_CMDTYPE_DEVICE_CONFIG;
				return true;
			}
			else if (strncmp(CmdType, "Broadcast", 9) == 0)
			{
				XmlMsg.cmdtype = XML_CMDTYPE_DEVICE_CONFIG;
				return true;
			}
			else
			{
				XmlMsg.cmdtype = XML_CMDTYPE_UNKNOWN;
				return false;
			}
		}
		else
		{
			XmlMsg.cmdname = XML_CMD_NAME_UNKNOWN;
			return false;
		}

		if (strcmp(CmdName, "Query") == 0)
		{
			XmlMsg.cmdname = XML_CMD_NAME_QUERY;
			XMLElement* CmdTypeElement = pRootNode->FirstChildElement("CmdType");
			if (!CmdTypeElement)
				return false;
			const char* CmdType = CmdTypeElement->GetText();
			if (!CmdType)
				return false;
			XMLElement* SNElement = pRootNode->FirstChildElement("SN");
			if (!SNElement)
				return false;
			XmlMsg.sn = atoi(SNElement->GetText());
			XMLElement* DeviceIDElement = pRootNode->FirstChildElement("DeviceID");
			if (!DeviceIDElement)
				return false;
			XmlMsg.DeviceID = DeviceIDElement->GetText();
			////DeviceStatus 设备状态查询
			//XML_CMDTYPE_DEVICE_STATUS,
			////Catalog 设备目录查询
			//XML_CMDTYPE_CATALOG,
			////DeviceInfo 设备信息查询
			//XML_CMDTYPE_DEVICE_INFO,
			////RecordInfo 文件目录检索
			//XML_CMDTYPE_RECORDINFO,
			////Alarm 报警查询
			//XML_CMDTYPE_ALARM,
			////ConfigDownload 设备配置查询
			//XML_CMDTYPE_CONFIG_DOWNLOAD,
			////PresetQuery 预置位查询
			//XML_CMDTYPE_PRESET_QUERY,
			////MobilePosition 移动设备位置数据查询
			//XML_CMDTYPE_MOBILE_POSITION,
			if (strcmp(CmdType, "Catalog") == 0)
			{
				XmlMsg.cmdtype = XML_CMDTYPE_CATALOG;
				return true;
			}
			else if (strcmp(CmdType, "DeviceInfo") == 0)
			{
				XmlMsg.cmdtype = XML_CMDTYPE_DEVICE_INFO;
				return true;
			}
			
			return true;
		}
		else if (strcmp(CmdName, "Notify") == 0)
		{
			XmlMsg.cmdname = XML_CMD_NAME_NOTIFY;
			XMLElement* CmdTypeElement = pRootNode->FirstChildElement("CmdType");
			if (!CmdTypeElement)
				return false;
			const char* CmdType = CmdTypeElement->GetText();
			if (!CmdType)
				return false;
			XMLElement* SNElement = pRootNode->FirstChildElement("SN");
			if (!SNElement)
				return false;
			XmlMsg.sn = atoi(SNElement->GetText());
			if (strcmp(CmdType, "Keepalive") == 0)
			{
				XmlMsg.cmdtype = XML_CMDTYPE_NOTIFY_KEEPALIVE;
				XMLElement* DeviceIDElement = pRootNode->FirstChildElement("DeviceID");
				if (!DeviceIDElement)
					return false;
				KeepAliveMsg* pKeepAliveMsg = new KeepAliveMsg;
				pKeepAliveMsg->DeviceID = DeviceIDElement->GetText();
				XMLElement* StatusElement = pRootNode->FirstChildElement("Status");
				if (!StatusElement)
					return false;
				pKeepAliveMsg->status = StatusElement->GetText();
				XmlMsg.pPoint = pKeepAliveMsg;
				return true;
			}
			else if (strcmp(CmdType, "Catalog") == 0)
			{
				XmlMsg.cmdtype = XML_CMDTYPE_NOTIFY_CATALOG;
				XMLElement* DeviceIDElement = pRootNode->FirstChildElement("DeviceID");
				if (!DeviceIDElement)
					return false;
				KeepAliveMsg* pKeepAliveMsg = new KeepAliveMsg;
				pKeepAliveMsg->DeviceID = DeviceIDElement->GetText();
				XMLElement* StatusElement = pRootNode->FirstChildElement("Status");
				if (!StatusElement)
					return false;
				pKeepAliveMsg->status = StatusElement->GetText();
				XmlMsg.pPoint = pKeepAliveMsg;
				return true;
			}
		}
		else if (strcmp(CmdName, "Response") == 0)
		{
			XmlMsg.cmdname = XML_CMD_NAME_RESPONSE;
			XMLElement* CmdTypeElement = pRootNode->FirstChildElement("CmdType");
			if (!CmdTypeElement)
				return false;
			const char* CmdType = CmdTypeElement->GetText();
			if (!CmdType)
				return false;
			XMLElement* SNElement = pRootNode->FirstChildElement("SN");
			if (!SNElement)
				return false;
			XmlMsg.sn = atoi(SNElement->GetText());
			XMLElement* DeviceIDElement = pRootNode->FirstChildElement("DeviceID");
			if (!DeviceIDElement)
				return false;
			XmlMsg.DeviceID = DeviceIDElement->GetText();

			if (strcmp(CmdType, "Catalog") == 0)
			{
				XmlMsg.cmdtype = XML_CMDTYPE_RESPONSE_CATALOG;

				ResponseCatalogList* pResponseCatalog = new ResponseCatalogList;
				XMLElement* SumNumElement = pRootNode->FirstChildElement("SumNum");
				if (!SumNumElement)
					return false;
				pResponseCatalog->allnum = atoi(SumNumElement->GetText());
				//<DeviceList Num="1">\n
				XMLElement* DeviceListElement = pRootNode->FirstChildElement("DeviceList");
				if (!DeviceListElement)
					return false;
				const XMLAttribute* NumAttribute = DeviceListElement->FindAttribute("Num");
				if (!NumAttribute)
					return false;
				int itemNum = NumAttribute->IntValue();
				XMLElement* ItemElement = DeviceListElement->FirstChildElement("Item");
				while (ItemElement && itemNum-- > 0)
				{
					std::string DeviceID;
					DeviceIDElement = ItemElement->FirstChildElement("DeviceID");
					if (!DeviceIDElement)
						return false;
					const char *pText = DeviceIDElement->GetText();
					DeviceID = pText;
					int catalogtype = -1;
					if (DeviceID.size() > 13)
					{
						if (DeviceID.substr(10, 3) == "216")
						{
							catalogtype = 0;
						}
						else if (DeviceID.substr(10, 3) == "118")
						{
							catalogtype = 1;
						}
						else if (DeviceID.substr(10, 3) == "132")
						{
							catalogtype = 2;
						}
						else if (DeviceID.substr(10, 3) == "215")
						{
							catalogtype = 3;
						}
						else if (DeviceID.substr(10, 3) == "200")
						{
							catalogtype = 4;
						}
						else if (DeviceID.substr(10, 3) == "131")
						{
							catalogtype = 5;
						}
						pResponseCatalog->catalogtype = catalogtype;
					}
					if (catalogtype == 0)
					{
						//Organization orginfo;
						//orginfo.OrgCode = DeviceID;
						//orginfo.qorgcode = devinfo.fatherdevid;

						//devnode = brothersnode->FirstChildElement("Name");
						//if (devnode && devnode->FirstChild() && devnode->FirstChild()->Value())
						//{
						//	orginfo.orgName = devnode->FirstChild()->Value();
						//}

						//devnode = brothersnode->FirstChildElement("ParentID");
						//if (devnode && devnode->FirstChild() && devnode->FirstChild()->Value())
						//{
						//	orginfo.ParentCode = devnode->FirstChild()->Value();
						//}
						//devnode = brothersnode->FirstChildElement("BusinessGroupID");
						//if (devnode && devnode->FirstChild() && devnode->FirstChild()->Value())
						//{
						//	orginfo.BGroupID = devnode->FirstChild()->Value();
						//}
						///*devnode = brothersnode->FirstChildElement("BusinessGroupID");
						//if (devnode&& devnode->FirstChild() && devnode->FirstChild()->Value())
						//{
						//	orginfo.BGroupID = devnode->FirstChild()->Value();
						//}*/
						//devinfo.m_OrgVect.push_back(orginfo);
					}
					else if (catalogtype == 2 || catalogtype == 1 || catalogtype == 5)
					{
						ResponseCatalogMsg cataitem;
						cataitem.DeviceID = DeviceID;

						XMLElement* nodeElement = ItemElement->FirstChildElement("Name");
						if (nodeElement)
						{
							pText = nodeElement->GetText();
							cataitem.Name = GbkToUtf8(pText ? pText : "");
						}
						nodeElement = ItemElement->FirstChildElement("Manufacturer");
						if (nodeElement)
						{
							pText = nodeElement->GetText();
							cataitem.Manufacturer = pText ? pText : "";
						}
						nodeElement = ItemElement->FirstChildElement("Model");
						if (nodeElement)
						{
							pText = nodeElement->GetText();
							cataitem.Model = pText ? pText : "";
						}
						nodeElement = ItemElement->FirstChildElement("Owner");
						if (nodeElement)
						{
							pText = nodeElement->GetText();
							cataitem.Owner = pText ? pText : "";
						}
						nodeElement = ItemElement->FirstChildElement("CivilCode");
						if (nodeElement)
						{
							pText = nodeElement->GetText();
							cataitem.CivilCode = pText ? pText : "";
						}
						nodeElement = ItemElement->FirstChildElement("Block");
						if (nodeElement)
						{
							pText = nodeElement->GetText();
							cataitem.Block = pText ? pText : "";
						}
						nodeElement = ItemElement->FirstChildElement("Address");
						if (nodeElement)
						{
							pText = nodeElement->GetText();
							cataitem.Address = pText ? pText : "";
						}
						nodeElement = ItemElement->FirstChildElement("Parental");
						if (nodeElement)
						{
							pText = nodeElement->GetText();
							cataitem.Parental = pText ? atoi(pText) : 0;
						}
						nodeElement = ItemElement->FirstChildElement("ParentID");
						if (nodeElement)
						{
							pText = nodeElement->GetText();
							cataitem.ParentID = pText ? pText : "";
						}
						//nodeElement = ItemElement->FirstChildElement("ParentID");
						//if (nodeElement)
						//{
						//	std::string parentid = nodeElement->GetText();
						//	std::vector<std::string> element;// = vStringSplit(parentid, "/");
						//	if (element.size() == 1)
						//	{
						//		if (element[0].substr(10, 3) == "216")
						//		{
						//			//cataitem.mOrgID = element[0];
						//		}
						//		else
						//		{
						//			cataitem.ParentID = element[0];
						//		}
						//	}
						//	else if (element.size() == 2)
						//	{
						//		cataitem.ParentID = element[0];
						//		//cataitem.mOrgID = element[1];
						//	}
						//	cataitem.ParentID = parentid;
						//}
						nodeElement = ItemElement->FirstChildElement("SafetyWay");
						if (nodeElement)
						{
							pText = nodeElement->GetText();
							cataitem.SafetyWay = pText ? atoi(pText) : 0;
						}
						nodeElement = ItemElement->FirstChildElement("RegisterWay");
						if (nodeElement)
						{
							pText = nodeElement->GetText();
							cataitem.RegisterWay = pText ? atoi(pText) : 0;
						}
						nodeElement = ItemElement->FirstChildElement("CertNum");
						if (nodeElement)
						{
							pText = nodeElement->GetText();
							cataitem.CertNum = pText ? pText : "";
						}
						nodeElement = ItemElement->FirstChildElement("Certifiable");
						if (nodeElement)
						{
							pText = nodeElement->GetText();
							cataitem.Certifiable = pText ? atoi(pText) : 0;
						}
						nodeElement = ItemElement->FirstChildElement("ErrCode");
						if (nodeElement)
						{
							pText = nodeElement->GetText();
							cataitem.ErrCode = pText ? atoi(pText) : 0;
						}
						nodeElement = ItemElement->FirstChildElement("EndTime");
						if (nodeElement)
						{
							pText = nodeElement->GetText();
							cataitem.EndTime = pText ? pText : "";
						}
						nodeElement = ItemElement->FirstChildElement("Secrecy");
						if (nodeElement)
						{
							pText = nodeElement->GetText();
							cataitem.Secrecy = pText ? atoi(pText) : 0;
						}
						nodeElement = ItemElement->FirstChildElement("IPAddress");
						if (nodeElement)
						{
							pText = nodeElement->GetText();
							cataitem.IPAddress = pText ? pText : "";
						}
						nodeElement = ItemElement->FirstChildElement("Port");
						if (nodeElement)
						{
							pText = nodeElement->GetText();
							cataitem.Port = pText ? atoi(pText) : 0;
						}
						nodeElement = ItemElement->FirstChildElement("Password");
						if (nodeElement)
						{
							pText = nodeElement->GetText();
							cataitem.Password = pText ? pText : "";
						}
						nodeElement = ItemElement->FirstChildElement("Status");
						if (nodeElement)
						{
							pText = nodeElement->GetText();
							if (strcmp(pText, "ON") == 0 || strcmp(pText, "OK") == 0)
							{
								cataitem.Status = 1;
							}
							else
							{
								cataitem.Status = 0;
							}
						}
						nodeElement = ItemElement->FirstChildElement("Longitude");
						if (nodeElement)
						{
							pText = nodeElement->GetText();
							cataitem.Longitude = pText ? atof(pText) : 0;
						}
						nodeElement = ItemElement->FirstChildElement("Latitude");
						if (nodeElement)
						{
							pText = nodeElement->GetText();
							cataitem.Latitude = pText ? atof(pText) : 0;
						}
						pResponseCatalog->m_devVect.push_back(cataitem);
					}
					else if (catalogtype == 3)
					{
						/*BusinessGroupItem groupinfo;
						groupinfo.qgroupcode = devinfo.fatherdevid;
						groupinfo.groupcode = DeviceID;

						devnode = brothersnode->FirstChildElement("Name");
						if (devnode && devnode->FirstChild() && devnode->FirstChild()->Value())
						{
							groupinfo.groupname = devnode->FirstChild()->Value();
						}
						devinfo.m_GroupVect.push_back(groupinfo);*/
					}
					else if (catalogtype == 4)
					{
						/*devinfo.sysinfo.qsystemcode = devinfo.fatherdevid;
						devinfo.sysinfo.systemid = DeviceID;

						devnode = brothersnode->FirstChildElement("Name");
						if (devnode && devnode->FirstChild() && devnode->FirstChild()->Value())
						{
							devinfo.sysinfo.name = devnode->FirstChild()->Value();
						}
						devnode = brothersnode->FirstChildElement("Manufacturer");
						if (devnode && devnode->FirstChild() && devnode->FirstChild()->Value())
						{
							devinfo.sysinfo.manufacturer = devnode->FirstChild()->Value();
						}
						devnode = brothersnode->FirstChildElement("Model");
						if (devnode && devnode->FirstChild() && devnode->FirstChild()->Value())
						{
							devinfo.sysinfo.model = devnode->FirstChild()->Value();
						}
						devnode = brothersnode->FirstChildElement("Owner");
						if (devnode && devnode->FirstChild() && devnode->FirstChild()->Value())
						{
							devinfo.sysinfo.owner = devnode->FirstChild()->Value();
						}
						devnode = brothersnode->FirstChildElement("CivilCode");
						if (devnode && devnode->FirstChild() && devnode->FirstChild()->Value())
						{
							devinfo.sysinfo.civilcode = devnode->FirstChild()->Value();
						}
						devnode = brothersnode->FirstChildElement("Address");
						if (devnode && devnode->FirstChild() && devnode->FirstChild()->Value())
						{
							devinfo.sysinfo.address = devnode->FirstChild()->Value();
						}
						devnode = brothersnode->FirstChildElement("RegisterWay");
						if (devnode && devnode->FirstChild() && devnode->FirstChild()->Value())
						{
							devinfo.sysinfo.registerway = Data(devnode->FirstChild()->Value()).convertInt();
						}
						devnode = brothersnode->FirstChildElement("Secrecy");
						if (devnode && devnode->FirstChild() && devnode->FirstChild()->Value())
						{
							devinfo.sysinfo.secrecy = Data(devnode->FirstChild()->Value()).convertInt();
						}*/
					}
					ItemElement = pRootNode->NextSiblingElement();
				}
				XmlMsg.pPoint = pResponseCatalog;
				return true;
			}
		}
		else if (strcmp(CmdName, "Request") == 0)
		{

		}
		return false;
	}
	return false;
}
bool AnalyzeSubscriptionMsg(const char* MsgStr, GB28181XmlMsg& XmlMsg)
{
	XMLDocument mDocument;
	mDocument.Clear();
	XMLError err = mDocument.Parse(MsgStr);
	if (err == XML_SUCCESS)
	{
		XMLElement* pRootNode = mDocument.RootElement();
		if (!pRootNode)
		{
			return false;
		}
		const char* XmlCmd = pRootNode->Value();
		if (!XmlCmd)
			return false;
		if (strcmp(XmlCmd, "Query") == 0)
		{
			XmlMsg.cmdname = XML_CMD_NAME_QUERY;
			XMLElement* CmdTypeElement = pRootNode->FirstChildElement("CmdType");
			if (!CmdTypeElement)
				return false;
			const char* XmlSubCmd = CmdTypeElement->GetText();
			if (!XmlSubCmd)
				return false;
			XMLElement* SNElement = pRootNode->FirstChildElement("SN");
			if (!SNElement)
				return false;
			XmlMsg.sn = atoi(SNElement->GetText());
			if (strcmp(XmlSubCmd, "Catalog") == 0)
			{
				XmlMsg.cmdtype = XML_CMDTYPE_CATALOG;
				XMLElement* DeviceIDElement = pRootNode->FirstChildElement("DeviceID");
				if (!DeviceIDElement)
					return false;
				CatalogSubscriptionMsg* pCatalogSubMsg = new CatalogSubscriptionMsg;
				pCatalogSubMsg->DeviceID = DeviceIDElement->GetText();

				XMLElement* StartTimeElement = pRootNode->FirstChildElement("StartTime");
				if (!StartTimeElement)
					return false;
				pCatalogSubMsg->StartTime = StartTimeElement->GetText();

				XMLElement* EndTimeElement = pRootNode->FirstChildElement("EndTime");
				if (!EndTimeElement)
					return false;
				pCatalogSubMsg->EndTime = EndTimeElement->GetText();

				XmlMsg.pPoint = pCatalogSubMsg;
				return true;
			}
			else if (strcmp(XmlSubCmd, "...") == 0)
			{

			}

			return true;
		}
		else if (strcmp(XmlCmd, "...") == 0)
		{

		}
		return false;
		XMLElement* QueryElement = mDocument.FirstChildElement("Query");
		if (!QueryElement)
		{
			return false;
		}

		return true;
	}
	return false;
}