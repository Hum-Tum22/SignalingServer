#include "MsgContentXml.h"
#include "tools/CodeConvert.h"
#include <string>


bool CreateKeepAliveMsg(const char* user, const uint32_t& sn, string &outstr)
{
	try
	{
		XMLDocument doc;
		doc.InsertEndChild(doc.NewDeclaration("xml version=\"1.0\" encoding=\"GB2312\""));
		XMLElement* NotifyElement = doc.NewElement("Notify");
		doc.InsertEndChild(NotifyElement);

			XMLElement* CmdTypeElement = doc.NewElement("CmdType");
			CmdTypeElement->InsertEndChild(doc.NewText("Keepalive"));
			NotifyElement->InsertEndChild(CmdTypeElement);

			XMLElement* SNElement = doc.NewElement("SN");
			SNElement->InsertEndChild(doc.NewText(to_string(sn).c_str()));
			NotifyElement->InsertEndChild(SNElement);

			XMLElement* DeviceIDElement = doc.NewElement("DeviceID");
			DeviceIDElement->InsertEndChild(doc.NewText(user));
			NotifyElement->InsertEndChild(DeviceIDElement);

			XMLElement* StatusElement = doc.NewElement("Status");
			StatusElement->InsertEndChild(doc.NewText("OK"));
			NotifyElement->InsertEndChild(StatusElement);
		XMLPrinter printer;
		doc.Print(&printer);
		outstr = printer.CStr();
		return true;
	}
	catch (...)
	{
	}
	return false;
}
bool CreateCatalogResponse(const char* user, const uint32_t& sn, int SumNum, vector<CatalogItem> Items, CatalogItemExpandInfo* pExpand, string& outstr)
{
	try
	{
		XMLDocument doc;
		doc.InsertEndChild(doc.NewDeclaration("xml version=\"1.0\" encoding=\"GB2312\""));
		XMLElement* ResponseElement = doc.NewElement("Response");
		doc.InsertEndChild(ResponseElement);

		XMLElement* CmdTypeElement = doc.NewElement("CmdType");
		CmdTypeElement->InsertEndChild(doc.NewText("Catalog"));
		ResponseElement->InsertEndChild(CmdTypeElement);

		XMLElement* SNElement = doc.NewElement("SN");
		SNElement->InsertEndChild(doc.NewText(to_string(sn).c_str()));
		ResponseElement->InsertEndChild(SNElement);

		XMLElement* DeviceIDElement = doc.NewElement("DeviceID");	//Ŀ���豸/����/ϵͳ�ı���,ȡֵ��Ŀ¼��ѯ������ͬ(��ѡ)
		DeviceIDElement->InsertEndChild(doc.NewText(user));
		ResponseElement->InsertEndChild(DeviceIDElement);

		XMLElement* SumNumElement = doc.NewElement("SumNum");
		SumNumElement->InsertEndChild(doc.NewText(to_string(SumNum).c_str()));
		ResponseElement->InsertEndChild(SumNumElement);

		XMLElement* DeviceListElement = doc.NewElement("DeviceList");
		DeviceListElement->SetAttribute("Num", Items.size());			//item num
		ResponseElement->InsertEndChild(DeviceListElement);

		//add item
		for (int i = 0; i < Items.size(); i++)
		{
			AddDeviceItemToCatalog(doc, DeviceListElement, Items[i]);
		}
		if (pExpand)
		{
			AddIPCInfoToCatalog(doc, ResponseElement, pExpand);
		}

		XMLPrinter printer;
		doc.Print(&printer);
		outstr = printer.CStr();
		return true;
	}
	catch (...)
	{
	}
	return false;
}
void AddDeviceItemToCatalog(XMLDocument& doc, XMLElement* DeviceListElement, const CatalogItem& item)
{
	try
	{
		XMLElement* ItemElement = doc.NewElement("Item");
		DeviceListElement->InsertEndChild(ItemElement);

		XMLElement* DeviceIDElement = doc.NewElement("DeviceID");
		DeviceIDElement->InsertEndChild(doc.NewText(item.DeviceID.c_str()));
		ItemElement->InsertEndChild(DeviceIDElement);

		XMLElement* NameElement = doc.NewElement("Name");
		NameElement->InsertEndChild(doc.NewText(ownCodeCvt::Utf8ToGbk(item.Name).c_str()));
		ItemElement->InsertEndChild(NameElement);

		XMLElement* ManufacturerElement = doc.NewElement("Manufacturer");	//Ŀ���豸/����/ϵͳ�ı���,ȡֵ��Ŀ¼��ѯ������ͬ(��ѡ)
		ManufacturerElement->InsertEndChild(doc.NewText(item.Manufacturer.c_str()));
		ItemElement->InsertEndChild(ManufacturerElement);

		XMLElement* ModelElement = doc.NewElement("Model");
		ModelElement->InsertEndChild(doc.NewText(item.Model.c_str()));
		ItemElement->InsertEndChild(ModelElement);

		XMLElement* OwnerElement = doc.NewElement("Owner");
		OwnerElement->InsertEndChild(doc.NewText(item.Owner.c_str()));
		ItemElement->InsertEndChild(OwnerElement);

		XMLElement* CivilCodeElement = doc.NewElement("CivilCode");
		CivilCodeElement->InsertEndChild(doc.NewText(item.CivilCode.c_str()));
		ItemElement->InsertEndChild(CivilCodeElement);

		XMLElement* BlockElement = doc.NewElement("Block");
		BlockElement->InsertEndChild(doc.NewText(item.Block.c_str()));
		ItemElement->InsertEndChild(BlockElement);

		XMLElement* AddressElement = doc.NewElement("Address");
		AddressElement->InsertEndChild(doc.NewText(item.Address.c_str()));
		ItemElement->InsertEndChild(AddressElement);

		XMLElement* ParentalElement = doc.NewElement("Parental");
		ParentalElement->InsertEndChild(doc.NewText(to_string(item.Parental).c_str()));
		ItemElement->InsertEndChild(ParentalElement);

		XMLElement* ParentIDElement = doc.NewElement("ParentID");
		ParentIDElement->InsertEndChild(doc.NewText(item.ParentID.c_str()));
		ItemElement->InsertEndChild(ParentIDElement);

		XMLElement* SafetyWayElement = doc.NewElement("SafetyWay");
		SafetyWayElement->InsertEndChild(doc.NewText(to_string(item.SafetyWay).c_str()));
		ItemElement->InsertEndChild(SafetyWayElement);

		XMLElement* RegisterWayElement = doc.NewElement("RegisterWay");
		RegisterWayElement->InsertEndChild(doc.NewText(to_string(item.RegisterWay).c_str()));
		ItemElement->InsertEndChild(RegisterWayElement);

		XMLElement* CertNumElement = doc.NewElement("CertNum");
		CertNumElement->InsertEndChild(doc.NewText(to_string(item.CertNum).c_str()));
		ItemElement->InsertEndChild(CertNumElement);

		XMLElement* CertifiableElement = doc.NewElement("Certifiable");
		CertifiableElement->InsertEndChild(doc.NewText(to_string(item.Certifiable).c_str()));
		ItemElement->InsertEndChild(CertifiableElement);

		XMLElement* ErrCodeElement = doc.NewElement("ErrCode");
		ErrCodeElement->InsertEndChild(doc.NewText(to_string(item.ErrCode).c_str()));
		ItemElement->InsertEndChild(ErrCodeElement);

		XMLElement* EndTimeElement = doc.NewElement("EndTime");
		EndTimeElement->InsertEndChild(doc.NewText(item.EndTime.c_str()));
		ItemElement->InsertEndChild(EndTimeElement);

		XMLElement* SecrecyElement = doc.NewElement("Secrecy");
		SecrecyElement->InsertEndChild(doc.NewText(to_string(item.Secrecy).c_str()));
		ItemElement->InsertEndChild(SecrecyElement);

		XMLElement* IPAddressElement = doc.NewElement("IPAddress");
		IPAddressElement->InsertEndChild(doc.NewText(item.IPAddress.c_str()));
		ItemElement->InsertEndChild(IPAddressElement);

		XMLElement* PortElement = doc.NewElement("Port");
		PortElement->InsertEndChild(doc.NewText(to_string(item.Port).c_str()));
		ItemElement->InsertEndChild(PortElement);

		XMLElement* PasswordElement = doc.NewElement("Password");
		PasswordElement->InsertEndChild(doc.NewText(item.Password.c_str()));
		ItemElement->InsertEndChild(PasswordElement);

		XMLElement* StatusElement = doc.NewElement("Status");
		StatusElement->InsertEndChild(doc.NewText(item.Status.c_str()));
		ItemElement->InsertEndChild(StatusElement);

		XMLElement* LongitudeElement = doc.NewElement("Longitude");
		LongitudeElement->InsertEndChild(doc.NewText(to_string(item.Longitude).c_str()));
		ItemElement->InsertEndChild(LongitudeElement);

		XMLElement* LatitudeElement = doc.NewElement("Latitude");
		LatitudeElement->InsertEndChild(doc.NewText(to_string(item.Latitude).c_str()));
		ItemElement->InsertEndChild(LatitudeElement);
	}
	catch (...)
	{
	}
}
void AddIPCInfoToCatalog(XMLDocument& doc, XMLElement* Element, const CatalogItemExpandInfo* expand)
{
	if (expand == NULL)
		return;
	try
	{
		XMLElement* InfoElement = doc.NewElement("Info");
		Element->InsertEndChild(InfoElement);

		XMLElement* PTZTypeElement = doc.NewElement("PTZType");
		PTZTypeElement->InsertEndChild(doc.NewText(to_string(expand->PTZType).c_str()));
		InfoElement->InsertEndChild(PTZTypeElement);

		XMLElement* PositionTypeElement = doc.NewElement("PositionType");
		PositionTypeElement->InsertEndChild(doc.NewText(to_string(expand->PositionType).c_str()));
		InfoElement->InsertEndChild(PositionTypeElement);

		XMLElement* RoomTypeElement = doc.NewElement("RoomType");
		RoomTypeElement->InsertEndChild(doc.NewText(to_string(expand->RoomType).c_str()));
		InfoElement->InsertEndChild(RoomTypeElement);

		XMLElement* UseTypeElement = doc.NewElement("UseType");
		UseTypeElement->InsertEndChild(doc.NewText(to_string(expand->UseType).c_str()));
		InfoElement->InsertEndChild(UseTypeElement);

		XMLElement* SupplyLightTypeElement = doc.NewElement("SupplyLightType");
		SupplyLightTypeElement->InsertEndChild(doc.NewText(to_string(expand->SupplyLightType).c_str()));
		InfoElement->InsertEndChild(SupplyLightTypeElement);

		XMLElement* DirectionTypeElement = doc.NewElement("DirectionType");
		DirectionTypeElement->InsertEndChild(doc.NewText(to_string(expand->DirectionType).c_str()));
		InfoElement->InsertEndChild(DirectionTypeElement);

		XMLElement* ResolutionElement = doc.NewElement("Resolution");
		ResolutionElement->InsertEndChild(doc.NewText(expand->Resolution.c_str()));
		InfoElement->InsertEndChild(ResolutionElement);

		XMLElement* BusinessGroupIDElement = doc.NewElement("BusinessGroupID");
		BusinessGroupIDElement->InsertEndChild(doc.NewText(expand->BusinessGroupID.c_str()));
		InfoElement->InsertEndChild(BusinessGroupIDElement);

		XMLElement* DownloadSpeedElement = doc.NewElement("DownloadSpeed");
		DownloadSpeedElement->InsertEndChild(doc.NewText(expand->DownloadSpeed.c_str()));
		InfoElement->InsertEndChild(DownloadSpeedElement);

		XMLElement* SVCSpaceSupportModeElement = doc.NewElement("SVCSpaceSupportMode");
		SVCSpaceSupportModeElement->InsertEndChild(doc.NewText(to_string(expand->SVCSpaceSupportMode).c_str()));
		InfoElement->InsertEndChild(SVCSpaceSupportModeElement);

		XMLElement* SVCTimeSupportModeElement = doc.NewElement("SVCTimeSupportMode");
		SVCTimeSupportModeElement->InsertEndChild(doc.NewText(to_string(expand->SVCTimeSupportMode).c_str()));
		InfoElement->InsertEndChild(SVCTimeSupportModeElement);
	}
	catch (...)
	{
	}
}
bool CreateDeviceInfoResponse(const char* DeviceId, const uint32_t& sn, const DeviceInfoMsg& devInfoMsg, string& outstr)
{
	try
	{
		XMLDocument doc;
		doc.InsertEndChild(doc.NewDeclaration("xml version=\"1.0\" encoding=\"GB2312\""));
		XMLElement* ResponseElement = doc.NewElement("Response");
		doc.InsertEndChild(ResponseElement);

		XMLElement* CmdTypeElement = doc.NewElement("CmdType");
		CmdTypeElement->InsertEndChild(doc.NewText("DeviceInfo"));
		ResponseElement->InsertEndChild(CmdTypeElement);

		XMLElement* SNElement = doc.NewElement("SN");
		SNElement->InsertEndChild(doc.NewText(to_string(sn).c_str()));
		ResponseElement->InsertEndChild(SNElement);

		XMLElement* DeviceIDElement = doc.NewElement("DeviceID");
		DeviceIDElement->InsertEndChild(doc.NewText(DeviceId));
		ResponseElement->InsertEndChild(DeviceIDElement);

		XMLElement* DeviceNameElement = doc.NewElement("DeviceName");
		DeviceNameElement->InsertEndChild(doc.NewText(devInfoMsg.DeviceName.c_str()));
		ResponseElement->InsertEndChild(DeviceNameElement);

		XMLElement* ResultElement = doc.NewElement("Result");
		ResultElement->InsertEndChild(doc.NewText(devInfoMsg.Result.c_str()));//ERROR
		ResponseElement->InsertEndChild(ResultElement);

		XMLElement* ManufacturerElement = doc.NewElement("Manufacturer");
		ManufacturerElement->InsertEndChild(doc.NewText(devInfoMsg.Manufacturer.c_str()));
		ResponseElement->InsertEndChild(ManufacturerElement);

		XMLElement* ModelElement = doc.NewElement("Model");
		ModelElement->InsertEndChild(doc.NewText(devInfoMsg.Model.c_str()));
		ResponseElement->InsertEndChild(ModelElement);

		XMLElement* FirmwareElement = doc.NewElement("Firmware");
		FirmwareElement->InsertEndChild(doc.NewText(devInfoMsg.Firmware.c_str()));
		ResponseElement->InsertEndChild(FirmwareElement);

		XMLElement* ChannelElement = doc.NewElement("Channel");
		ChannelElement->InsertEndChild(doc.NewText(to_string(devInfoMsg.Channel).c_str()));
		ResponseElement->InsertEndChild(ChannelElement);

		/*<!--��չ��Ϣ, �ɶ���-->
		<elementname = "Info"minOccurs = "0"maxOccurs = "unbounded">
		<restrictionbase = "string">
		<maxLengthvalue = "1024" / >
		< / restriction>
		< / element>*/

		XMLPrinter printer;
		doc.Print(&printer);
		outstr = printer.CStr();
		return true;
	}
	catch (...)
	{
	}
	return false;
}
bool CreateCatalogSubscriptionResponseMsg(const char* user, const uint32_t& sn, string& outstr)
{
	try
	{
		XMLDocument doc;
		doc.InsertEndChild(doc.NewDeclaration("xml version=\"1.0\" encoding=\"GB2312\""));
		XMLElement* ResponseElement = doc.NewElement("Response");
		doc.InsertEndChild(ResponseElement);

		XMLElement* CmdTypeElement = doc.NewElement("CmdType");
		CmdTypeElement->InsertEndChild(doc.NewText("Catalog"));
		ResponseElement->InsertEndChild(CmdTypeElement);

		XMLElement* SNElement = doc.NewElement("SN");
		SNElement->InsertEndChild(doc.NewText(to_string(sn).c_str()));
		ResponseElement->InsertEndChild(SNElement);

		XMLElement* DeviceIDElement = doc.NewElement("DeviceID");
		DeviceIDElement->InsertEndChild(doc.NewText(user));
		ResponseElement->InsertEndChild(DeviceIDElement);

		XMLElement* ResultElement = doc.NewElement("Result");
		ResultElement->InsertEndChild(doc.NewText("OK"));
		ResponseElement->InsertEndChild(ResultElement);

		XMLPrinter printer;
		doc.Print(&printer);
		outstr = printer.CStr();
		return true;
	}
	catch (...)
	{
	}
	return false;
}
bool CreateCatalogQueryRequestMsg(const char* gbid, const uint32_t& sn, string& outstr)
{
	try
	{
		XMLDocument doc;
		doc.InsertEndChild(doc.NewDeclaration("xml version=\"1.0\" encoding=\"GB2312\""));
		XMLElement* ResponseElement = doc.NewElement("Query");
		doc.InsertEndChild(ResponseElement);

		XMLElement* CmdTypeElement = doc.NewElement("CmdType");
		CmdTypeElement->InsertEndChild(doc.NewText("Catalog"));
		ResponseElement->InsertEndChild(CmdTypeElement);

		XMLElement* SNElement = doc.NewElement("SN");
		SNElement->InsertEndChild(doc.NewText(to_string(sn).c_str()));
		ResponseElement->InsertEndChild(SNElement);

		XMLElement* DeviceIDElement = doc.NewElement("DeviceID");
		DeviceIDElement->InsertEndChild(doc.NewText(gbid));
		ResponseElement->InsertEndChild(DeviceIDElement);

		XMLPrinter printer;
		doc.Print(&printer);
		outstr = printer.CStr();
		return true;
	}
	catch (...)
	{
	}
	return false;
}