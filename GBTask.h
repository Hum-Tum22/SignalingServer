#pragma once

#include "tools/iThreadPool.h"
#include "MsgContentXml.h"
#include "SipServer.h"
#include "UserAgentMgr.h"

class GBDeviceOnlineTask : public ownTask::CTask
{
public:
	GBDeviceOnlineTask(std::string gbid, resip::Uri target) :DeviceId(gbid), toUri(target)
	{}
	bool TaskRun()
	{
		std::string OutStr;
		CreateCatalogQueryRequestMsg(DeviceId.c_str(), 1, OutStr);
		resip::UaMgr* pUaManager = GetServer()->GetUaManager();
		if (pUaManager)
		{
			pUaManager->GetMsgMgr()->SendRequestPageMsg(toUri, OutStr, MsgCmdType_Catalog);
		}
		//GetServer()->GetMsgManager()->SendRequestPageMsg(toUri, OutStr, MsgCmdType_Catalog);
		//this->setResult(NULL);
		std::cout << "query catalog :" << DeviceId << std::endl;
		return true;
	}
	bool TaskClose()
	{
		return true;
	}
private:
	std::string DeviceId;
	resip::Uri toUri;
};