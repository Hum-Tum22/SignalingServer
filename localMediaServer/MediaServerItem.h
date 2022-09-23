#pragma once
#include <iostream>
#include <map>
#include <sstream>

#include "SsrcConfig.h"
#include "ZLMServerConfig.h"

class MediaServerItem {

    std::string id;

    std::string ip;

    std::string hookIp;

    std::string sdpIp;

    std::string streamIp;

    int httpPort;

    int httpSSlPort;

    int rtmpPort;

    int rtmpSSlPort;

    int rtpProxyPort;

    int rtspPort;

    int rtspSSLPort;

    bool autoConfig;

    std::string secret;

    int streamNoneReaderDelayMS;

    int hookAliveInterval;

    bool rtpEnable;

    bool status;

    std::string rtpPortRange;

    std::string sendRtpPortRange;

    int recordAssistPort;

    std::string createTime;

    std::string updateTime;

    std::string lastKeepaliveTime;

    bool defaultServer;

    SsrcConfig ssrcConfig;

    int currentPort;


    /**
     * 每一台ZLM都有一套独立的SSRC列表
     * 在ApplicationCheckRunner里对mediaServerSsrcMap进行初始化
     */
    std::map<std::string, SsrcConfig> mediaServerSsrcMap;

public:
    MediaServerItem()
    {
    }

    MediaServerItem(ZLMServerConfig zlmServerConfig, std::string sipIp) {
        id = "FQ3TF8yT83wh5Wvz";// zlmServerConfig.getGeneralMediaServerId();
        ip = "192.168.1.232";// zlmServerConfig.getIp();
        hookIp = "192.168.1.232";// StringUtils.isEmpty(zlmServerConfig.getHookIp()) ? sipIp : zlmServerConfig.getHookIp();
        sdpIp = "192.168.1.232";// StringUtils.isEmpty(zlmServerConfig.getSdpIp()) ? zlmServerConfig.getIp() : zlmServerConfig.getSdpIp();
        streamIp = "192.168.1.232";// StringUtils.isEmpty(zlmServerConfig.getStreamIp()) ? zlmServerConfig.getIp() : zlmServerConfig.getStreamIp();
        httpPort = 80;// zlmServerConfig.getHttpPort();
        httpSSlPort = 443;// zlmServerConfig.getHttpSSLport();
        rtmpPort = 1935;// zlmServerConfig.getRtmpPort();
        rtmpSSlPort = 0;// zlmServerConfig.getRtmpSslPort();
        rtpProxyPort = 0;// zlmServerConfig.getRtpProxyPort();
        rtspPort = 554;// zlmServerConfig.getRtspPort();
        rtspSSLPort = 0;// zlmServerConfig.getRtspSSlport();
        autoConfig = true; // 默认值true;
        secret = "035c73f7-bb6b-4889-a715-d9eb2d1925cc";// zlmServerConfig.getApiSecret();
        streamNoneReaderDelayMS = 10000;// zlmServerConfig.getGeneralStreamNoneReaderDelayMS();
        hookAliveInterval = 60;// zlmServerConfig.getHookAliveInterval();
        rtpEnable = false; // 默认使用单端口;直到用户自己设置开启多端口
        rtpPortRange = "30000,30500";// zlmServerConfig.getPortRange().replace("_", ","); // 默认使用30000,30500作为级联时发送流的端口号
        sendRtpPortRange = "30000,30500"; // 默认使用30000,30500作为级联时发送流的端口号
        recordAssistPort = 0; // 默认关闭

    }
    std::string getJsonString()
    {
        std::ostringstream ds;
        ds << "{"
            << "\"" << "id" << "\"" << ":" << id
            << "\"" << "ip" << "\"" << ":" << ip
            << "\"" << "hookIp" << "\"" << ":" << hookIp
            << "\"" << "sdpIp" << "\"" << ":" << sdpIp
            << "\"" << "streamIp" << "\"" << ":" << streamIp
            << "\"" << "httpPort" << "\"" << ":" << httpPort
            << "\"" << "httpSSlPort" << "\"" << ":" << httpSSlPort
            << "\"" << "rtmpPort" << "\"" << ":" << rtmpPort
            << "\"" << "rtmpSSlPort" << "\"" << ":" << rtmpSSlPort
            << "\"" << "rtpProxyPort" << "\"" << ":" << rtpProxyPort
            << "\"" << "rtspPort" << "\"" << ":" << rtspPort
            << "\"" << "rtspSSLPort" << "\"" << ":" << rtspSSLPort
            << "\"" << "autoConfig" << "\"" << ":" << autoConfig
            << "\"" << "secret" << "\"" << ":" << secret
            << "\"" << "streamNoneReaderDelayMS" << "\"" << ":" << streamNoneReaderDelayMS
            << "\"" << "hookAliveInterval" << "\"" << ":" << hookAliveInterval
            << "\"" << "rtpEnable" << "\"" << ":" << rtpEnable
            << "\"" << "status" << "\"" << ":" << status
            << "\"" << "rtpPortRange" << "\"" << ":" << rtpPortRange
            << "\"" << "sendRtpPortRange" << "\"" << ":" << sendRtpPortRange
            << "\"" << "recordAssistPort" << "\"" << ":" << recordAssistPort
            << "\"" << "createTime" << "\"" << ":" << createTime
            << "\"" << "updateTime" << "\"" << ":" << updateTime
            << "\"" << "lastKeepaliveTime" << "\"" << ":" << lastKeepaliveTime
            << "\"" << "defaultServer" << "\"" << ":" << defaultServer

            //<< "\"" << "ssrcConfig" << "\"" << ":" << "{" << "}"

            << "\"" << "currentPort" << "\"" << ":" << currentPort
            << "}";
        return ds.str();
    }
    std::string getId() {
        return id;
    }

    void setId(std::string strId) {
        id = strId;
    }

    std::string getIp() {
        return ip;
    }

    void setIp(std::string strIp) {
        ip = strIp;
    }

    std::string getHookIp() {
        return hookIp;
    }

    void setHookIp(std::string strHookIp) {
        hookIp = strHookIp;
    }

    std::string getSdpIp() {
        return sdpIp;
    }

    void setSdpIp(std::string strSdpIp) {
        sdpIp = strSdpIp;
    }

    std::string getStreamIp() {
        return streamIp;
    }

    void setStreamIp(std::string strStreamIp) {
        streamIp = strStreamIp;
    }

    int getHttpPort() {
        return httpPort;
    }

    void setHttpPort(int iHttpPort) {
        httpPort = iHttpPort;
    }

    int getHttpSSlPort() {
        return httpSSlPort;
    }

    void setHttpSSlPort(int iHttpSSlPort) {
        httpSSlPort = iHttpSSlPort;
    }

    int getRtmpPort() {
        return rtmpPort;
    }

    void setRtmpPort(int iRtmpPort) {
        rtmpPort = iRtmpPort;
    }

    int getRtmpSSlPort() {
        return rtmpSSlPort;
    }

    void setRtmpSSlPort(int iRtmpSSlPort) {
        rtmpSSlPort = iRtmpSSlPort;
    }

    int getRtpProxyPort() {
        return rtpProxyPort;
    }

    void setRtpProxyPort(int iRtpProxyPort) {
        rtpProxyPort = iRtpProxyPort;
    }

    int getRtspPort() {
        return rtspPort;
    }

    void setRtspPort(int iRtspPort) {
        rtspPort = rtspPort;
    }

    int getRtspSSLPort() {
        return rtspSSLPort;
    }

    void setRtspSSLPort(int iRtspSSLPort) {
        rtspSSLPort = iRtspSSLPort;
    }

    bool isAutoConfig() {
        return autoConfig;
    }

    void setAutoConfig(bool bAutoConfig) {
        autoConfig = bAutoConfig;
    }

    std::string getSecret() {
        return secret;
    }

    void setSecret(std::string strSecret) {
        secret = strSecret;
    }

    int getStreamNoneReaderDelayMS() {
        return streamNoneReaderDelayMS;
    }

    void setStreamNoneReaderDelayMS(int iStreamNoneReaderDelayMS) {
        streamNoneReaderDelayMS = streamNoneReaderDelayMS;
    }

    bool isRtpEnable() {
        return rtpEnable;
    }

    void setRtpEnable(bool bRtpEnable) {
        rtpEnable = rtpEnable;
    }

    std::string getRtpPortRange() {
        return rtpPortRange;
    }

    void setRtpPortRange(std::string strRtpPortRange) {
        rtpPortRange = strRtpPortRange;
    }

    int getRecordAssistPort() {
        return recordAssistPort;
    }

    void setRecordAssistPort(int iRecordAssistPort) {
        recordAssistPort = iRecordAssistPort;
    }

    bool isDefaultServer() {
        return defaultServer;
    }

    void setDefaultServer(bool bDefaultServer) {
        defaultServer = bDefaultServer;
    }

    std::string getCreateTime() {
        return createTime;
    }

    void setCreateTime(std::string strCreateTime) {
        createTime = createTime;
    }

    std::string getUpdateTime() {
        return updateTime;
    }

    void setUpdateTime(std::string strUpdateTime) {
        updateTime = strUpdateTime;
    }

    std::map<std::string, SsrcConfig> getMediaServerSsrcMap() {
        return mediaServerSsrcMap;
    }

    void setMediaServerSsrcMap(std::map<std::string, SsrcConfig> lMediaServerSsrcMap) {
        mediaServerSsrcMap = lMediaServerSsrcMap;
    }

    SsrcConfig getSsrcConfig() {
        return ssrcConfig;
    }

    void setSsrcConfig(SsrcConfig cSsrcConfig) {
        ssrcConfig = cSsrcConfig;
    }

    int getCurrentPort() {
        return currentPort;
    }

    void setCurrentPort(int iCurrentPort) {
        currentPort = iCurrentPort;
    }

    bool isStatus() {
        return status;
    }

    void setStatus(bool bStatus) {
        status = bStatus;
    }

    std::string getLastKeepaliveTime() {
        return lastKeepaliveTime;
    }

    void setLastKeepaliveTime(std::string strLastKeepaliveTime) {
        lastKeepaliveTime = strLastKeepaliveTime;
    }

    std::string getSendRtpPortRange() {
        return sendRtpPortRange;
    }

    void setSendRtpPortRange(std::string strSendRtpPortRange) {
        sendRtpPortRange = strSendRtpPortRange;
    }

    int getHookAliveInterval() {
        return hookAliveInterval;
    }

    void setHookAliveInterval(int iHookAliveInterval) {
        hookAliveInterval = iHookAliveInterval;
    }
};