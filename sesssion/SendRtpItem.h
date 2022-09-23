#pragma once
#include <iostream>

class SendRtpItem {

    /**
     * ����ip
     */
    std::string ip;

    /**
     * �����˿�
     */
    int port;

    /**
     * ������ʶ
     */
    std::string ssrc;

    /**
     * ƽ̨id
     */
    std::string platformId;

    /**
    * ��Ӧ�豸id
    */
    std::string deviceId;

    /**
     * ֱ������Ӧ����
     */
    std::string app;

    /**
      * ͨ��id
      */
    std::string channelId;

    /**
     * ����״̬
     * 0 �ȴ��豸��������
     * 1 �ȴ��ϼ�ƽ̨�ظ�ack
     * 2 ������
     */
    int status = 0;


    /**
     * �豸������streamId
     */
    std::string streamId;

    /**
     * �Ƿ�Ϊtcp
     */
    bool tcp;

    /**
     * �Ƿ�Ϊtcp����ģʽ
     */
    bool tcpActive;

    /**
     * �Լ�����ʹ�õĶ˿�
     */
    int localPort;

    /**
     * ʹ�õ���ý��
     */
    std::string mediaServerId;

    /**
     *  invite��callId
     */
    std::string CallId;

    /**
     * ����ʱ��rtp��pt��uint8_t��,����ʱĬ��Ϊ96
     */
    int pt = 96;

    /**
     * ����ʱ��rtp�ĸ������͡�Ϊtrueʱ������Ϊps��Ϊfalseʱ��Ϊes��
     */
    bool usePs = true;

    /**
     * ��usePs Ϊfalseʱ����Ч��Ϊ1ʱ��������Ƶ��Ϊ0ʱ��������Ƶ������ʱĬ��Ϊ0
     */
    bool onlyAudio = false;
public:
    enum InviteStreamType {
        PLAY, PLAYBACK, PUSH, PROXY, CLOUD_RECORD_PUSH, CLOUD_RECORD_PROXY
    };
private:
    /**
     * ��������
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