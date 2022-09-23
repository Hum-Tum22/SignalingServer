#pragma once

#include "DeviceInfo.h"
#include "DeviceChannel.h"
#include <list>
#include <vector>


class CatalogDateQuery
{
public:
    enum CatalogDataStatus {
        ready, runIng, end
    };

    CatalogDateQuery():mStatus(ready),mSn(0),mTotal(0), mCurCount(0)
    {
    }

    int getSn()
    {
        return mSn;
    }
    void setSn(int sn)
    {
        mSn = sn;
    }
    int getTotal()
    {
        return mTotal;
    }
    void setTotal(int total)
    {
        mTotal = total;
    }

    list<GBDeviceChannel*> getChannelList()
    {
        return mChannelList;
    }
    void setChannelList(list<GBDeviceChannel*> channelList)
    {
        mChannelList = channelList;
    }
    string getLastTime() {
        return mLastTime;
    }
    void setLastTime(string lastTime)
    {
        mLastTime = lastTime;
    }
    SipServerDeviceInfo getDevice()
    {
        return mDevice;
    }
    void setDevice(SipServerDeviceInfo device)
    {
        mDevice = device;
    }
    string getErrorMsg() {
        return mErrorMsg;
    }
    void setErrorMsg(string errorMsg)
    {
        mErrorMsg = errorMsg;
    }
    CatalogDataStatus getStatus()
    {
        return mStatus;
    }
    void setStatus(CatalogDataStatus status)
    {
        mStatus = status;
    }
    void addResponseCatalog(ResponseCatalogMsg catalog)
    {
        GBDeviceChannel* pGbChannel = new GBDeviceChannel();
        pGbChannel->setChannelId(catalog.DeviceID);
        pGbChannel->setName(catalog.Name);
        pGbChannel->setManufacture(catalog.Manufacturer);
        pGbChannel->setModel(catalog.Model);
        pGbChannel->setOwner(catalog.Owner);
        pGbChannel->setCivilCode(catalog.CivilCode);
        pGbChannel->setBlock(catalog.Block);
        pGbChannel->setAddress(catalog.Address);
        pGbChannel->setParental(catalog.Parental);
        pGbChannel->setParentId(catalog.ParentID);
        pGbChannel->setSafetyWay(catalog.SafetyWay);
        pGbChannel->setRegisterWay(catalog.RegisterWay);
        pGbChannel->setCertNum(catalog.CertNum);
        pGbChannel->setCertifiable(catalog.Certifiable);
        pGbChannel->setErrCode(catalog.ErrCode);
        pGbChannel->setEndTime(catalog.EndTime);
        pGbChannel->setSecrecy(std::to_string(catalog.Secrecy));
        pGbChannel->setIpAddress(catalog.IPAddress);
        pGbChannel->setPort(catalog.Port);
        pGbChannel->setPassword(catalog.Password);
        pGbChannel->setStatus(catalog.Status);
        pGbChannel->setLongitude(catalog.Longitude);
        pGbChannel->setLatitude(catalog.Latitude);
        pGbChannel->setDeviceId(mDevice.getDeviceId());
        mCurCount++;
    }
    int getCatalogNum()
    {
        return mCurCount;
    }
private:
    int mSn; // √¸¡Ó–Ú¡–∫≈
    int mTotal;
    int mCurCount;
    list<GBDeviceChannel*> mChannelList;
    string mLastTime;
    SipServerDeviceInfo mDevice;
    string mErrorMsg;
    CatalogDataStatus mStatus;
};