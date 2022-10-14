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
	 * 数据库自增ID
	 */
	int id;

	/**
	 * 通道id
	 */
	string uuid;
	/**
	 * 通道id
	 */
	string channelId;

	/**
	 * 设备id
	 */
	string deviceId;

	/**
	 * 通道名
	 */
	string name;

	/**
	 * 生产厂商
	 */
	string manufacture;

	/**
	 * 型号
	 */
	string model;

	/**
	 * 设备归属
	 */
	string owner;

	/**
	 * 行政区域
	 */
	string civilCode;

	/**
	 * 警区
	 */
	string block;

	/**
	 * 安装地址
	 */
	string address;

	/**
	 * 是否有子设备 1有, 0没有
	 */
	int parental;

	/**
	 * 父级id
	 */
	string parentId;

	/**
	 * 信令安全模式  缺省为0; 0:不采用; 2: S/MIME签名方式; 3: S/ MIME加密签名同时采用方式; 4:数字摘要方式
	 */
	int safetyWay;

	/**
	 * 注册方式 缺省为1;1:符合IETFRFC3261标准的认证注册模 式; 2:基于口令的双向认证注册模式; 3:基于数字证书的双向认证注册模式
	 */
	int registerWay;

	/**
	 * 证书序列号
	 */
	string certNum;

	/**
	 * 证书有效标识 缺省为0;证书有效标识:0:无效1: 有效
	 */
	int certifiable;

	/**
	 * 证书无效原因码
	 */
	int errCode;

	/**
	 * 证书终止有效期
	 */
	string endTime;

	/**
	 * 保密属性 缺省为0; 0:不涉密, 1:涉密
	 */
	string secrecy;

	/**
	 * IP地址
	 */
	string ipAddress;

	/**
	 * 端口号
	 */
	int port;

	/**
	 * 密码
	 */
	string password;

	/**
	 * 云台类型
	 */
	int PTZType;

	/**
	 * 云台类型描述字符串
	 */
	string PTZTypeText;

	/**
	 * 创建时间
	 */
	string createTime;

	/**
	 * 更新时间
	 */
	string updateTime;

	/**
	 * 在线/离线
	 * 1在线,0离线
	 * 默认在线
	 * 信令:
	 * <Status>ON</Status>
	 * <Status>OFF</Status>
	 * 遇到过NVR下的IPC下发信令可以推流， 但是 Status 响应 OFF
	 */
	int status;

	/**
	 * 经度
	 */
	double longitude;

	/**
	 * 纬度
	 */
	double latitude;

	/**
	 * 子设备数
	 */
	int subCount;

	/**
	 * 流唯一编号，存在表示正在直播
	 */
	string  streamId;

	/**
	 *  是否含有音频
	 */
	bool hasAudio;

	/**
	 * 标记通道的类型，0->国标通道 1->直播流通道 2->业务分组/虚拟组织/行政区划
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
			PTZTypeText = "未知";
			break;
		case 1:
			PTZTypeText = "球机";
			break;
		case 2:
			PTZTypeText = "半球";
			break;
		case 3:
			PTZTypeText = "固定枪机";
			break;
		case 4:
			PTZTypeText = "遥控枪机";
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