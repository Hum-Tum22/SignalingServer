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
        {
            delete (RecordInfoQueryMsg*)pPoint;
            pPoint = NULL;
            break;
        }
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
        case XML_CMDTYPE_RESPONSE_SUB_CATALOG:
        case XML_CMDTYPE_RESPONSE_CATALOG_NOTIFY_RECEIVED:
        case XML_CMDTYPE_RESPONSE_DEV_INFO:
        case XML_CMDTYPE_RESPONSE_DEV_STATUS:
        case XML_CMDTYPE_RESPONSE_RECORD_INFO:
        case XML_CMDTYPE_RESPONSE_DEV_CONFIG:
        case XML_CMDTYPE_RESPONSE_CONFIG_DOWNLOAD:
        case XML_CMDTYPE_RESPONSE_PRESET_QUERY:
        case XML_CMDTYPE_RESPONSE_BROADCAST:
        {
            break;
        }
        default:
            break;
        }
    }
}
bool AnalyzeReceivedSipMsg(const char* MsgStr, GB28181XmlMsg& XmlMsg)
{
    if (!MsgStr) return false;
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
            XmlMsg.sn = SNElement->IntText();

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
                    info->value = controlName->GetText();
                    XMLElement* infoElement = pRootNode->FirstChildElement("Info");
                    if (infoElement)
                    {
                        XMLElement* PriorityElement = infoElement->FirstChildElement("ControlPriority");
                        if (PriorityElement)
                        {
                            info->ControlPriority = PriorityElement->IntText();
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
                    info->value = controlName->GetText();
                    return true;
                }
                controlName = pRootNode->FirstChildElement("RecordCmd");
                if (controlName)
                {
                    RecordControlInfo* info = new RecordControlInfo();
                    XmlMsg.pPoint = info;
                    XmlMsg.controlCmd = XML_CONTROLCMD_RECORD;
                    info->value = controlName->GetText();
                    return true;
                }
                controlName = pRootNode->FirstChildElement("GuardCmd");
                if (controlName)
                {
                    GuardControlInfo* info = new GuardControlInfo();
                    XmlMsg.pPoint = info;
                    XmlMsg.controlCmd = XML_CONTROLCMD_GUARD;
                    info->value = controlName->GetText();
                    return true;
                }
                controlName = pRootNode->FirstChildElement("AlarmCmd");
                if (controlName)
                {
                    AlarmControlInfo* info = new AlarmControlInfo();
                    XmlMsg.pPoint = info;
                    XmlMsg.controlCmd = XML_CONTROLCMD_ALARM;
                    info->value = controlName->GetText();
                    return true;
                }
                controlName = pRootNode->FirstChildElement("IFameCmd");
                if (controlName)
                {
                    IFameControlInfo* info = new IFameControlInfo();
                    XmlMsg.pPoint = info;
                    XmlMsg.controlCmd = XML_CONTROLCMD_IFRAME;
                    info->value = controlName->GetText();
                    return true;
                }
                controlName = pRootNode->FirstChildElement("DragZoomIn");
                if (controlName)
                {
                    DragZoomControlInfo* info = new DragZoomControlInfo();
                    XmlMsg.pPoint = info;
                    XmlMsg.controlCmd = XML_CONTROLCMD_DRAGZOOMIN;
                    info->value = controlName->GetText();
                    return true;
                }
                controlName = pRootNode->FirstChildElement("DragZoomOut");
                if (controlName)
                {
                    DragZoomControlInfo* info = new DragZoomControlInfo();
                    XmlMsg.pPoint = info;
                    XmlMsg.controlCmd = XML_CONTROLCMD_DRAGZOOMOUT;
                    info->value = controlName->GetText();
                    return true;
                }
                controlName = pRootNode->FirstChildElement("HomePosition");
                if (controlName)
                {
                    HomePositionControlInfo* info = new HomePositionControlInfo();
                    XmlMsg.pPoint = info;
                    XmlMsg.controlCmd = XML_CONTROLCMD_HOMEPOSITION;
                    info->value = controlName->GetText();
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
            XmlMsg.cmdname = XML_CMD_NAME_QUERY;
            XMLElement* CmdTypeElement = pRootNode->FirstChildElement("CmdType");
            if (!CmdTypeElement)
                return false;

            XMLElement* SNElement = pRootNode->FirstChildElement("SN");
            if (!SNElement)
                return false;
            XmlMsg.sn = SNElement->IntText();

            XMLElement* DeviceIDElement = pRootNode->FirstChildElement("DeviceID");
            if (!DeviceIDElement)
                return false;
            XmlMsg.DeviceID = DeviceIDElement->GetText();

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
                XmlMsg.cmdtype = XML_CMDTYPE_CATALOG;

                CatalogQueryMsg* pCatalogMsg = new CatalogQueryMsg();
                XMLElement* StartTimeElement = pRootNode->FirstChildElement("StartTime");
                if (StartTimeElement)
                {
                    pCatalogMsg->StartTime = StartTimeElement->GetText();
                }

                XMLElement* EndTimeElement = pRootNode->FirstChildElement("EndTime");
                if (EndTimeElement)
                {
                    pCatalogMsg->EndTime = EndTimeElement->GetText();
                }

                XmlMsg.pPoint = pCatalogMsg;
                return true;
            }
            else if (strncmp(CmdType, "DeviceInfo", 10) == 0)
            {
                XmlMsg.cmdtype = XML_CMDTYPE_DEVICE_CONFIG;
                return true;
            }
            else if (strncmp(CmdType, "RecordInfo", 10) == 0)
            {
                XmlMsg.cmdtype = XML_CMDTYPE_RECORDINFO;
                RecordInfoQueryMsg *pQueryMsg = new RecordInfoQueryMsg();
                if(pQueryMsg == NULL)
                {
                    return false;
                }
                XmlMsg.pPoint = pQueryMsg;
                pQueryMsg->DeviceID = XmlMsg.DeviceID;
                XMLElement *StartTimeElement = pRootNode->FirstChildElement("StartTime");
                if(StartTimeElement)
                {
                    pQueryMsg->StartTime = StartTimeElement->GetText();
                }
                XMLElement *EndTimeElement = pRootNode->FirstChildElement("EndTime");
                if (EndTimeElement)
                {
                    pQueryMsg->EndTime = EndTimeElement->GetText();
                }
                XMLElement *FilePathElement = pRootNode->FirstChildElement("FilePath");
                if(FilePathElement)
                {
                    pQueryMsg->FilePath = FilePathElement->GetText();
                }
                XMLElement *AddressElement = pRootNode->FirstChildElement("Address");
                if(AddressElement)
                {
                    pQueryMsg->Address = AddressElement->GetText();
                }
                XMLElement *SecrecyElement = pRootNode->FirstChildElement("Secrecy");
                if (SecrecyElement)
                {
                    try
                    {
                        pQueryMsg->Secrecy = std::atoi(SecrecyElement->GetText());
                    }
                    catch(const std::exception& e)
                    {
                        std::cerr << e.what() << '\n';
                        pQueryMsg->Secrecy = 0;
                    }
                }
                XMLElement *TypeElement = pRootNode->FirstChildElement("Type");
                if(TypeElement)
                {
                    pQueryMsg->RecordType = TypeElement->GetText();
                }
                XMLElement *RecorderIDElement = pRootNode->FirstChildElement("RecorderID");
                if(RecorderIDElement)
                {
                    pQueryMsg->StartTime = StartTimeElement->GetText();
                }
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
        return false;
    }
    return false;
}