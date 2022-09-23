#pragma once
#include <iostream>

class SendRtpItem {

    /**
     * 推流ip
     */
    std::string ip;

    /**
     * 推流端口
     */
    int port;

    /**
     * 推流标识
     */
    std::string ssrc;

    /**
     * 平台id
     */
    std::string platformId;

    /**
    * 对应设备id
    */
    std::string deviceId;

    /**
     * 直播流的应用名
     */
    std::string app;

    /**
      * 通道id
      */
    std::string channelId;

    /**
     * 推流状态
     * 0 等待设备推流上来
     * 1 等待上级平台回复ack
     * 2 推流中
     */
    int status = 0;


    /**
     * 设备推流的streamId
     */
    std::string streamId;

    /**
     * 是否为tcp
     */
    bool tcp;

    /**
     * 是否为tcp主动模式
     */
    bool tcpActive;

    /**
     * 自己推流使用的端口
     */
    int localPort;

    /**
     * 使用的流媒体
     */
    std::string mediaServerId;

    /**
     *  invite的callId
     */
    std::string CallId;

    /**
     * 发送时，rtp的pt（uint8_t）,不传时默认为96
     */
    int pt = 96;

    /**
     * 发送时，rtp的负载类型。为true时，负载为ps；为false时，为es；
     */
    bool usePs = true;

    /**
     * 当usePs 为false时，有效。为1时，发送音频；为0时，发送视频；不传时默认为0
     */
    bool onlyAudio = false;
public:
    enum InviteStreamType {
        PLAY, PLAYBACK, PUSH, PROXY, CLOUD_RECORD_PUSH, CLOUD_RECORD_PROXY
    };
private:
    /**
     * 播放类型
     */
    InviteStreamType playType;

    /*byte[] transaction;

    byte[] dialog;*/
public:
    std::string getIp() {
        return ip;
    }

    void setIp(std::string ip) {
        ip = ip;
    }

    int getPort() {
        return port;
    }

    void setPort(int port) {
        port = port;
    }

    std::string getSsrc() {
        return ssrc;
    }

    void setSsrc(std::string ssrc) {
        ssrc = ssrc;
    }

    std::string getPlatformId() {
        return platformId;
    }

    void setPlatformId(std::string platformId) {
        platformId = platformId;
    }

    std::string getDeviceId() {
        return deviceId;
    }

    void setDeviceId(std::string deviceId) {
        deviceId = deviceId;
    }

    std::string getChannelId() {
        return channelId;
    }

    void setChannelId(std::string channelId) {
        channelId = channelId;
    }

    int getStatus() {
        return status;
    }

    void setStatus(int status) {
        status = status;
    }

    std::string getApp() {
        return app;
    }

    void setApp(std::string app) {
        app = app;
    }

    std::string getStreamId() {
        return streamId;
    }

    void setStreamId(std::string streamId) {
        streamId = streamId;
    }

    bool isTcp() {
        return tcp;
    }

    void setTcp(bool tcp) {
        tcp = tcp;
    }

    int getLocalPort() {
        return localPort;
    }

    void setLocalPort(int localPort) {
        localPort = localPort;
    }

    bool isTcpActive() {
        return tcpActive;
    }

    void setTcpActive(bool tcpActive) {
        tcpActive = tcpActive;
    }

    std::string getMediaServerId() {
        return mediaServerId;
    }

    void setMediaServerId(std::string mediaServerId) {
        mediaServerId = mediaServerId;
    }

    std::string getCallId() {
        return CallId;
    }

    void setCallId(std::string callId) {
        CallId = callId;
    }

    InviteStreamType getPlayType() {
        return playType;
    }

    void setPlayType(InviteStreamType playType) {
        playType = playType;
    }

    /*byte[] getTransaction() {
        return transaction;
    }

    void setTransaction(byte[] transaction) {
        transaction = transaction;
    }

    byte[] getDialog() {
        return dialog;
    }

    void setDialog(byte[] dialog) {
        dialog = dialog;
    }*/

    int getPt() {
        return pt;
    }

    void setPt(int pt) {
        pt = pt;
    }

    bool isUsePs() {
        return usePs;
    }

    void setUsePs(bool usePs) {
        usePs = usePs;
    }

    bool isOnlyAudio() {
        return onlyAudio;
    }

    void setOnlyAudio(bool onlyAudio) {
        onlyAudio = onlyAudio;
    }
};