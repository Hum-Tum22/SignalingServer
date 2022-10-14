#pragma once

#include <iostream>

using namespace std;
class IDeviceChannel
{
public:
	virtual void updateDeviceChannel(IDeviceChannel *devchl);
};
class GBDeviceChannel: public IDeviceChannel
{
	/**
	 * ���ݿ�����ID
	 */
	int id;

	/**
	 * ͨ��id
	 */
	string uuid;
	/**
	 * ͨ��id
	 */
	string channelId;

	/**
	 * �豸id
	 */
	string deviceId;

	/**
	 * ͨ����
	 */
	string name;

	/**
	 * ��������
	 */
	string manufacture;

	/**
	 * �ͺ�
	 */
	string model;

	/**
	 * �豸����
	 */
	string owner;

	/**
	 * ��������
	 */
	string civilCode;

	/**
	 * ����
	 */
	string block;

	/**
	 * ��װ��ַ
	 */
	string address;

	/**
	 * �Ƿ������豸 1��, 0û��
	 */
	int parental;

	/**
	 * ����id
	 */
	string parentId;

	/**
	 * ���ȫģʽ  ȱʡΪ0; 0:������; 2: S/MIMEǩ����ʽ; 3: S/ MIME����ǩ��ͬʱ���÷�ʽ; 4:����ժҪ��ʽ
	 */
	int safetyWay;

	/**
	 * ע�᷽ʽ ȱʡΪ1;1:����IETFRFC3261��׼����֤ע��ģ ʽ; 2:���ڿ����˫����֤ע��ģʽ; 3:��������֤���˫����֤ע��ģʽ
	 */
	int registerWay;

	/**
	 * ֤�����к�
	 */
	string certNum;

	/**
	 * ֤����Ч��ʶ ȱʡΪ0;֤����Ч��ʶ:0:��Ч1: ��Ч
	 */
	int certifiable;

	/**
	 * ֤����Чԭ����
	 */
	int errCode;

	/**
	 * ֤����ֹ��Ч��
	 */
	string endTime;

	/**
	 * �������� ȱʡΪ0; 0:������, 1:����
	 */
	string secrecy;

	/**
	 * IP��ַ
	 */
	string ipAddress;

	/**
	 * �˿ں�
	 */
	int port;

	/**
	 * ����
	 */
	string password;

	/**
	 * ��̨����
	 */
	int PTZType;

	/**
	 * ��̨���������ַ���
	 */
	string PTZTypeText;

	/**
	 * ����ʱ��
	 */
	string createTime;

	/**
	 * ����ʱ��
	 */
	string updateTime;

	/**
	 * ����/����
	 * 1����,0����
	 * Ĭ������
	 * ����:
	 * <Status>ON</Status>
	 * <Status>OFF</Status>
	 * ������NVR�µ�IPC�·�������������� ���� Status ��Ӧ OFF
	 */
	int status;

	/**
	 * ����
	 */
	double longitude;

	/**
	 * γ��
	 */
	double latitude;

	/**
	 * ���豸��
	 */
	int subCount;

	/**
	 * ��Ψһ��ţ����ڱ�ʾ����ֱ��
	 */
	string  streamId;

	/**
	 *  �Ƿ�����Ƶ
	 */
	bool hasAudio;

	/**
	 * ���ͨ�������ͣ�0->����ͨ�� 1->ֱ����ͨ�� 2->ҵ�����/������֯/��������
	 */
	int channelType;
public:
	GBDeviceChannel() {};
	GBDeviceChannel(const GBDeviceChannel& dChl):
		id(dChl.id)
		, uuid(dChl.uuid)
		, channelId(dChl.channelId)
		, deviceId(dChl.deviceId)
		, name(dChl.name)
		, manufacture(dChl.manufacture)
		, model(dChl.model)
		, owner(dChl.owner)
		, civilCode(dChl.civilCode)
		, block(dChl.block)
		, address(dChl.address)
		, parental(dChl.parental)
		, parentId(dChl.parentId)
		, safetyWay(dChl.safetyWay)
		, registerWay(dChl.registerWay)
		, certNum(dChl.certNum)
		, certifiable(dChl.certifiable)
		, errCode(dChl.errCode)
		, endTime(dChl.endTime)
		, secrecy(dChl.secrecy)
		, ipAddress(dChl.ipAddress)
		, port(dChl.port)
		, password(dChl.password)
		, PTZType(dChl.PTZType)
		, PTZTypeText(dChl.PTZTypeText)
		, createTime(dChl.createTime)
		, updateTime(dChl.updateTime)
		, status(dChl.status)
		, longitude(dChl.longitude)
		, latitude(dChl.latitude)
		, subCount(dChl.subCount)
		, streamId(dChl.streamId)
		, hasAudio(dChl.hasAudio)
		, channelType(dChl.channelType)

	{};
	int getId() {
		return id;
	}

	void setId(int iid) {
		id = iid;
	}
	string getUuid() {
		return uuid;
	}

	void setUuid(string uid) {
		uuid = uid;
	}
	string getDeviceId() {
		return deviceId;
	}

	void setDeviceId(string sdeviceId) {
		deviceId = sdeviceId;
	}

	void setPTZType(int iPTZType) {
		PTZType = iPTZType;
		switch (PTZType) {
		case 0:
			PTZTypeText = "δ֪";
			break;
		case 1:
			PTZTypeText = "���";
			break;
		case 2:
			PTZTypeText = "����";
			break;
		case 3:
			PTZTypeText = "�̶�ǹ��";
			break;
		case 4:
			PTZTypeText = "ң��ǹ��";
			break;
		}
	}

	string getChannelId() {
		return channelId;
	}

	void setChannelId(string schannelId) {
		channelId = schannelId;
	}

	string getName() {
		return name;
	}

	void setName(string sname) {
		name = sname;
	}

	string getManufacture() {
		return manufacture;
	}

	void setManufacture(string smanufacture) {
		manufacture = smanufacture;
	}

	string getModel() {
		return model;
	}

	void setModel(string smodel) {
		model = smodel;
	}

	string getOwner() {
		return owner;
	}

	void setOwner(string sowner) {
		owner = sowner;
	}

	string getCivilCode() {
		return civilCode;
	}

	void setCivilCode(string scivilCode) {
		civilCode = scivilCode;
	}

	string getBlock() {
		return block;
	}

	void setBlock(string sblock) {
		block = sblock;
	}

	string getAddress() {
		return address;
	}

	void setAddress(string saddress) {
		address = saddress;
	}

	int getParental() {
		return parental;
	}

	void setParental(int iparental) {
		parental = iparental;
	}

	string getParentId() {
		return parentId;
	}

	void setParentId(string sparentId) {
		parentId = sparentId;
	}

	int getSafetyWay() {
		return safetyWay;
	}

	void setSafetyWay(int isafetyWay) {
		safetyWay = isafetyWay;
	}

	int getRegisterWay() {
		return registerWay;
	}

	void setRegisterWay(int iregisterWay) {
		registerWay = iregisterWay;
	}

	string getCertNum() {
		return certNum;
	}

	void setCertNum(string scertNum) {
		certNum = scertNum;
	}

	int getCertifiable() {
		return certifiable;
	}

	void setCertifiable(int icertifiable) {
		certifiable = icertifiable;
	}

	int getErrCode() {
		return errCode;
	}

	void setErrCode(int ierrCode) {
		errCode = ierrCode;
	}

	string getEndTime() {
		return endTime;
	}

	void setEndTime(string sendTime) {
		endTime = sendTime;
	}

	string getSecrecy() {
		return secrecy;
	}

	void setSecrecy(string ssecrecy) {
		secrecy = ssecrecy;
	}

	string getIpAddress() {
		return ipAddress;
	}

	void setIpAddress(string sipAddress) {
		ipAddress = sipAddress;
	}

	int getPort() {
		return port;
	}

	void setPort(int iport) {
		port = iport;
	}

	string getPassword() {
		return password;
	}

	void setPassword(string spassword) {
		password = spassword;
	}

	int getPTZType() {
		return PTZType;
	}

	string getPTZTypeText() {
		return PTZTypeText;
	}

	void setPTZTypeText(string sPTZTypeText) {
		PTZTypeText = sPTZTypeText;
	}

	int getStatus() {
		return status;
	}

	void setStatus(int istatus) {
		status = istatus;
	}

	double getLongitude() {
		return longitude;
	}

	void setLongitude(double dlongitude) {
		longitude = dlongitude;
	}

	double getLatitude() {
		return latitude;
	}

	void setLatitude(double dlatitude) {
		latitude = dlatitude;
	}

	int getSubCount() {
		return subCount;
	}

	void setSubCount(int isubCount) {
		subCount = isubCount;
	}

	bool isHasAudio() {
		return hasAudio;
	}

	void setHasAudio(bool ihasAudio) {
		hasAudio = ihasAudio;
	}

	string getStreamId() {
		return streamId;
	}

	void setStreamId(string sstreamId) {
		streamId = sstreamId;
	}

	string getCreateTime() {
		return createTime;
	}

	void setCreateTime(string screateTime) {
		createTime = screateTime;
	}

	string getUpdateTime() {
		return updateTime;
	}

	void setUpdateTime(string supdateTime) {
		updateTime = supdateTime;
	}

	int getChannelType() {
		return channelType;
	}

	void setChannelType(int ichannelType) {
		channelType = ichannelType;
	}
};
class OnvifDeviceChannel : public IDeviceChannel
{
	
public:
	OnvifDeviceChannel() {};
};