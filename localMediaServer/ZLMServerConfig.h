#ifndef ZLM_MEDIA_SERVER_CONFIG_H_
#define ZLM_MEDIA_SERVER_CONFIG_H_
#include <iostream>

class ZLMServerConfig
{
    //JSONField(name = "api.apiDebug")
    std::string apiDebug;

    //JSONField(name = "api.secret")
    std::string apiSecret;

    //JSONField(name = "ffmpeg.bin")
    std::string ffmpegBin;

    //JSONField(name = "ffmpeg.cmd")
    std::string ffmpegCmd;

    //JSONField(name = "ffmpeg.log")
    std::string ffmpegLog;

    //JSONField(name = "general.enableVhost")
    std::string generalEnableVhost;

    //JSONField(name = "general.mediaServerId")
    std::string generalMediaServerId;

    //JSONField(name = "general.flowThreshold")
    std::string generalFlowThreshold;

    //JSONField(name = "general.maxStreamWaitMS")
    std::string generalMaxStreamWaitMS;

    //JSONField(name = "general.streamNoneReaderDelayMS")
    int generalStreamNoneReaderDelayMS;

    //JSONField(name = "ip")
    std::string ip;

    std::string sdpIp;

    std::string streamIp;

    std::string hookIp;

    std::string updateTime;

    std::string createTime;

    //JSONField(name = "hls.fileBufSize")
    std::string hlsFileBufSize;

    //JSONField(name = "hls.filePath")
    std::string hlsFilePath;

    //JSONField(name = "hls.segDur")
    std::string hlsSegDur;

    //JSONField(name = "hls.segNum")
    std::string hlsSegNum;

    //JSONField(name = "hook.access_file_except_hls")
    std::string hookAccessFileExceptHLS;

    //JSONField(name = "hook.admin_params")
    std::string hookAdminParams;

    //JSONField(name = "hook.alive_interval")
    int hookAliveInterval;

    //JSONField(name = "hook.enable")
    std::string hookEnable;

    //JSONField(name = "hook.on_flow_report")
    std::string hookOnFlowReport;

    //JSONField(name = "hook.on_http_access")
    std::string hookOnHttpAccess;

    //JSONField(name = "hook.on_play")
    std::string hookOnPlay;

    //JSONField(name = "hook.on_publish")
    std::string hookOnPublish;

    //JSONField(name = "hook.on_record_mp4")
    std::string hookOnRecordMp4;

    //JSONField(name = "hook.on_rtsp_auth")
    std::string hookOnRtspAuth;

    //JSONField(name = "hook.on_rtsp_realm")
    std::string hookOnRtspRealm;

    //JSONField(name = "hook.on_shell_login")
    std::string hookOnShellLogin;

    //JSONField(name = "hook.on_stream_changed")
    std::string hookOnStreamChanged;

    //JSONField(name = "hook.on_stream_none_reader")
    std::string hookOnStreamNoneReader;

    //JSONField(name = "hook.on_stream_not_found")
    std::string hookOnStreamNotFound;

    //JSONField(name = "hook.timeoutSec")
    std::string hookTimeoutSec;

    //JSONField(name = "http.charSet")
    std::string httpCharSet;

    //JSONField(name = "http.keepAliveSecond")
    std::string httpKeepAliveSecond;

    //JSONField(name = "http.maxReqCount")
    std::string httpMaxReqCount;

    //JSONField(name = "http.maxReqSize")
    std::string httpMaxReqSize;

    //JSONField(name = "http.notFound")
    std::string httpNotFound;

    //JSONField(name = "http.port")
    int httpPort;

    //JSONField(name = "http.rootPath")
    std::string httpRootPath;

    //JSONField(name = "http.sendBufSize")
    std::string httpSendBufSize;

    //JSONField(name = "http.sslport")
    int httpSSLport;

    //JSONField(name = "multicast.addrMax")
    std::string multicastAddrMax;

    //JSONField(name = "multicast.addrMin")
    std::string multicastAddrMin;

    //JSONField(name = "multicast.udpTTL")
    std::string multicastUdpTTL;

    //JSONField(name = "record.appName")
    std::string recordAppName;

    //JSONField(name = "record.filePath")
    std::string recordFilePath;

    //JSONField(name = "record.fileSecond")
    std::string recordFileSecond;

    //JSONField(name = "record.sampleMS")
    std::string recordFileSampleMS;

    //JSONField(name = "rtmp.handshakeSecond")
    std::string rtmpHandshakeSecond;

    //JSONField(name = "rtmp.keepAliveSecond")
    std::string rtmpKeepAliveSecond;

    //JSONField(name = "rtmp.modifyStamp")
    std::string rtmpModifyStamp;

    //JSONField(name = "rtmp.port")
    int rtmpPort;

    //JSONField(name = "rtmp.sslport")
    int rtmpSslPort;

    //JSONField(name = "rtp.audioMtuSize")
    std::string rtpAudioMtuSize;

    //JSONField(name = "rtp.clearCount")
    std::string rtpClearCount;

    //JSONField(name = "rtp.cycleMS")
    std::string rtpCycleMS;

    //JSONField(name = "rtp.maxRtpCount")
    std::string rtpMaxRtpCount;

    //JSONField(name = "rtp.videoMtuSize")
    std::string rtpVideoMtuSize;

    //JSONField(name = "rtp_proxy.checkSource")
    std::string rtpProxyCheckSource;

    //JSONField(name = "rtp_proxy.dumpDir")
    std::string rtpProxyDumpDir;

    //JSONField(name = "rtp_proxy.port")
    int rtpProxyPort;

    //JSONField(name = "rtp_proxy.port_range")
    std::string portRange;

    //JSONField(name = "rtp_proxy.timeoutSec")
    std::string rtpProxyTimeoutSec;

    //JSONField(name = "rtsp.authBasic")
    std::string rtspAuthBasic;

    //JSONField(name = "rtsp.handshakeSecond")
    std::string rtspHandshakeSecond;

    //JSONField(name = "rtsp.keepAliveSecond")
    std::string rtspKeepAliveSecond;

    //JSONField(name = "rtsp.port")
    int rtspPort;

    //JSONField(name = "rtsp.sslport")
    int rtspSSlport;

    //JSONField(name = "shell.maxReqSize")
    std::string shellMaxReqSize;

    //JSONField(name = "shell.shell")
    std::string shellPhell;

public:
    ZLMServerConfig():generalStreamNoneReaderDelayMS(10000), hookAliveInterval(60), httpPort(80), httpSSLport(0), rtmpPort(1935)
        , rtmpSslPort(0), rtpProxyPort(0), rtspPort(554), rtspSSlport(0)
    {}
    std::string getHookIp() {
        return hookIp;
    }

    void setHookIp(std::string hookIp) {
        hookIp = hookIp;
    }

    std::string getApiDebug() {
        return apiDebug;
    }

    void setApiDebug(std::string apiDebug) {
        apiDebug = apiDebug;
    }

    std::string getApiSecret() {
        return apiSecret;
    }

    void setApiSecret(std::string apiSecret) {
        apiSecret = apiSecret;
    }

    std::string getFfmpegBin() {
        return ffmpegBin;
    }

    void setFfmpegBin(std::string ffmpegBin) {
        ffmpegBin = ffmpegBin;
    }

    std::string getFfmpegCmd() {
        return ffmpegCmd;
    }

    void setFfmpegCmd(std::string ffmpegCmd) {
        ffmpegCmd = ffmpegCmd;
    }

    std::string getFfmpegLog() {
        return ffmpegLog;
    }

    void setFfmpegLog(std::string ffmpegLog) {
        ffmpegLog = ffmpegLog;
    }

    std::string getGeneralEnableVhost() {
        return generalEnableVhost;
    }

    void setGeneralEnableVhost(std::string generalEnableVhost) {
        generalEnableVhost = generalEnableVhost;
    }

    std::string getGeneralMediaServerId() {
        return generalMediaServerId;
    }

    void setGeneralMediaServerId(std::string generalMediaServerId) {
        generalMediaServerId = generalMediaServerId;
    }

    std::string getGeneralFlowThreshold() {
        return generalFlowThreshold;
    }

    void setGeneralFlowThreshold(std::string generalFlowThreshold) {
        generalFlowThreshold = generalFlowThreshold;
    }

    std::string getGeneralMaxStreamWaitMS() {
        return generalMaxStreamWaitMS;
    }

    void setGeneralMaxStreamWaitMS(std::string generalMaxStreamWaitMS) {
        generalMaxStreamWaitMS = generalMaxStreamWaitMS;
    }

    int getGeneralStreamNoneReaderDelayMS() {
        return generalStreamNoneReaderDelayMS;
    }

    void setGeneralStreamNoneReaderDelayMS(int generalStreamNoneReaderDelayMS) {
        generalStreamNoneReaderDelayMS = generalStreamNoneReaderDelayMS;
    }

    std::string getIp() {
        return ip;
    }

    void setIp(std::string ip) {
        ip = ip;
    }

    std::string getSdpIp() {
        return sdpIp;
    }

    void setSdpIp(std::string sdpIp) {
        sdpIp = sdpIp;
    }

    std::string getStreamIp() {
        return streamIp;
    }

    void setStreamIp(std::string streamIp) {
        streamIp = streamIp;
    }

    std::string getUpdateTime() {
        return updateTime;
    }

    void setUpdateTime(std::string updateTime) {
        updateTime = updateTime;
    }

    std::string getCreateTime() {
        return createTime;
    }

    void setCreateTime(std::string createTime) {
        createTime = createTime;
    }

    std::string getHlsFileBufSize() {
        return hlsFileBufSize;
    }

    void setHlsFileBufSize(std::string hlsFileBufSize) {
        hlsFileBufSize = hlsFileBufSize;
    }

    std::string getHlsFilePath() {
        return hlsFilePath;
    }

    void setHlsFilePath(std::string hlsFilePath) {
        hlsFilePath = hlsFilePath;
    }

    std::string getHlsSegDur() {
        return hlsSegDur;
    }

    void setHlsSegDur(std::string hlsSegDur) {
        hlsSegDur = hlsSegDur;
    }

    std::string getHlsSegNum() {
        return hlsSegNum;
    }

    void setHlsSegNum(std::string hlsSegNum) {
        hlsSegNum = hlsSegNum;
    }

    std::string getHookAccessFileExceptHLS() {
        return hookAccessFileExceptHLS;
    }

    void setHookAccessFileExceptHLS(std::string hookAccessFileExceptHLS) {
        hookAccessFileExceptHLS = hookAccessFileExceptHLS;
    }

    std::string getHookAdminParams() {
        return hookAdminParams;
    }

    void setHookAdminParams(std::string hookAdminParams) {
        hookAdminParams = hookAdminParams;
    }

    std::string getHookEnable() {
        return hookEnable;
    }

    void setHookEnable(std::string hookEnable) {
        hookEnable = hookEnable;
    }

    std::string getHookOnFlowReport() {
        return hookOnFlowReport;
    }

    void setHookOnFlowReport(std::string hookOnFlowReport) {
        hookOnFlowReport = hookOnFlowReport;
    }

    std::string getHookOnHttpAccess() {
        return hookOnHttpAccess;
    }

    void setHookOnHttpAccess(std::string hookOnHttpAccess) {
        hookOnHttpAccess = hookOnHttpAccess;
    }

    std::string getHookOnPlay() {
        return hookOnPlay;
    }

    void setHookOnPlay(std::string hookOnPlay) {
        hookOnPlay = hookOnPlay;
    }

    std::string getHookOnPublish() {
        return hookOnPublish;
    }

    void setHookOnPublish(std::string hookOnPublish) {
        hookOnPublish = hookOnPublish;
    }

    std::string getHookOnRecordMp4() {
        return hookOnRecordMp4;
    }

    void setHookOnRecordMp4(std::string hookOnRecordMp4) {
        hookOnRecordMp4 = hookOnRecordMp4;
    }

    std::string getHookOnRtspAuth() {
        return hookOnRtspAuth;
    }

    void setHookOnRtspAuth(std::string hookOnRtspAuth) {
        hookOnRtspAuth = hookOnRtspAuth;
    }

    std::string getHookOnRtspRealm() {
        return hookOnRtspRealm;
    }

    void setHookOnRtspRealm(std::string hookOnRtspRealm) {
        hookOnRtspRealm = hookOnRtspRealm;
    }

    std::string getHookOnShellLogin() {
        return hookOnShellLogin;
    }

    void setHookOnShellLogin(std::string hookOnShellLogin) {
        hookOnShellLogin = hookOnShellLogin;
    }

    std::string getHookOnStreamChanged() {
        return hookOnStreamChanged;
    }

    void setHookOnStreamChanged(std::string hookOnStreamChanged) {
        hookOnStreamChanged = hookOnStreamChanged;
    }

    std::string getHookOnStreamNoneReader() {
        return hookOnStreamNoneReader;
    }

    void setHookOnStreamNoneReader(std::string hookOnStreamNoneReader) {
        hookOnStreamNoneReader = hookOnStreamNoneReader;
    }

    std::string getHookOnStreamNotFound() {
        return hookOnStreamNotFound;
    }

    void setHookOnStreamNotFound(std::string hookOnStreamNotFound) {
        hookOnStreamNotFound = hookOnStreamNotFound;
    }

    std::string getHookTimeoutSec() {
        return hookTimeoutSec;
    }

    void setHookTimeoutSec(std::string hookTimeoutSec) {
        hookTimeoutSec = hookTimeoutSec;
    }

    std::string getHttpCharSet() {
        return httpCharSet;
    }

    void setHttpCharSet(std::string httpCharSet) {
        httpCharSet = httpCharSet;
    }

    std::string getHttpKeepAliveSecond() {
        return httpKeepAliveSecond;
    }

    void setHttpKeepAliveSecond(std::string httpKeepAliveSecond) {
        httpKeepAliveSecond = httpKeepAliveSecond;
    }

    std::string getHttpMaxReqCount() {
        return httpMaxReqCount;
    }

    void setHttpMaxReqCount(std::string httpMaxReqCount) {
        httpMaxReqCount = httpMaxReqCount;
    }

    std::string getHttpMaxReqSize() {
        return httpMaxReqSize;
    }

    void setHttpMaxReqSize(std::string httpMaxReqSize) {
        httpMaxReqSize = httpMaxReqSize;
    }

    std::string getHttpNotFound() {
        return httpNotFound;
    }

    void setHttpNotFound(std::string httpNotFound) {
        httpNotFound = httpNotFound;
    }

    int getHttpPort() {
        return httpPort;
    }

    void setHttpPort(int httpPort) {
        httpPort = httpPort;
    }

    std::string getHttpRootPath() {
        return httpRootPath;
    }

    void setHttpRootPath(std::string httpRootPath) {
        httpRootPath = httpRootPath;
    }

    std::string getHttpSendBufSize() {
        return httpSendBufSize;
    }

    void setHttpSendBufSize(std::string httpSendBufSize) {
        httpSendBufSize = httpSendBufSize;
    }

    int getHttpSSLport() {
        return httpSSLport;
    }

    void setHttpSSLport(int httpSSLport) {
        httpSSLport = httpSSLport;
    }

    std::string getMulticastAddrMax() {
        return multicastAddrMax;
    }

    void setMulticastAddrMax(std::string multicastAddrMax) {
        multicastAddrMax = multicastAddrMax;
    }

    std::string getMulticastAddrMin() {
        return multicastAddrMin;
    }

    void setMulticastAddrMin(std::string multicastAddrMin) {
        multicastAddrMin = multicastAddrMin;
    }

    std::string getMulticastUdpTTL() {
        return multicastUdpTTL;
    }

    void setMulticastUdpTTL(std::string multicastUdpTTL) {
        multicastUdpTTL = multicastUdpTTL;
    }

    std::string getRecordAppName() {
        return recordAppName;
    }

    void setRecordAppName(std::string recordAppName) {
        recordAppName = recordAppName;
    }

    std::string getRecordFilePath() {
        return recordFilePath;
    }

    void setRecordFilePath(std::string recordFilePath) {
        recordFilePath = recordFilePath;
    }

    std::string getRecordFileSecond() {
        return recordFileSecond;
    }

    void setRecordFileSecond(std::string recordFileSecond) {
        recordFileSecond = recordFileSecond;
    }

    std::string getRecordFileSampleMS() {
        return recordFileSampleMS;
    }

    void setRecordFileSampleMS(std::string recordFileSampleMS) {
        recordFileSampleMS = recordFileSampleMS;
    }

    std::string getRtmpHandshakeSecond() {
        return rtmpHandshakeSecond;
    }

    void setRtmpHandshakeSecond(std::string rtmpHandshakeSecond) {
        rtmpHandshakeSecond = rtmpHandshakeSecond;
    }

    std::string getRtmpKeepAliveSecond() {
        return rtmpKeepAliveSecond;
    }

    void setRtmpKeepAliveSecond(std::string rtmpKeepAliveSecond) {
        rtmpKeepAliveSecond = rtmpKeepAliveSecond;
    }

    std::string getRtmpModifyStamp() {
        return rtmpModifyStamp;
    }

    void setRtmpModifyStamp(std::string rtmpModifyStamp) {
        rtmpModifyStamp = rtmpModifyStamp;
    }

    int getRtmpPort() {
        return rtmpPort;
    }

    void setRtmpPort(int rtmpPort) {
        rtmpPort = rtmpPort;
    }

    int getRtmpSslPort() {
        return rtmpSslPort;
    }

    void setRtmpSslPort(int rtmpSslPort) {
        rtmpSslPort = rtmpSslPort;
    }

    std::string getRtpAudioMtuSize() {
        return rtpAudioMtuSize;
    }

    void setRtpAudioMtuSize(std::string rtpAudioMtuSize) {
        rtpAudioMtuSize = rtpAudioMtuSize;
    }

    std::string getRtpClearCount() {
        return rtpClearCount;
    }

    void setRtpClearCount(std::string rtpClearCount) {
        rtpClearCount = rtpClearCount;
    }

    std::string getRtpCycleMS() {
        return rtpCycleMS;
    }

    void setRtpCycleMS(std::string rtpCycleMS) {
        rtpCycleMS = rtpCycleMS;
    }

    std::string getRtpMaxRtpCount() {
        return rtpMaxRtpCount;
    }

    void setRtpMaxRtpCount(std::string rtpMaxRtpCount) {
        rtpMaxRtpCount = rtpMaxRtpCount;
    }

    std::string getRtpVideoMtuSize() {
        return rtpVideoMtuSize;
    }

    void setRtpVideoMtuSize(std::string rtpVideoMtuSize) {
        rtpVideoMtuSize = rtpVideoMtuSize;
    }

    std::string getRtpProxyCheckSource() {
        return rtpProxyCheckSource;
    }

    void setRtpProxyCheckSource(std::string rtpProxyCheckSource) {
        rtpProxyCheckSource = rtpProxyCheckSource;
    }

    std::string getRtpProxyDumpDir() {
        return rtpProxyDumpDir;
    }

    void setRtpProxyDumpDir(std::string rtpProxyDumpDir) {
        rtpProxyDumpDir = rtpProxyDumpDir;
    }

    int getRtpProxyPort() {
        return rtpProxyPort;
    }

    void setRtpProxyPort(int rtpProxyPort) {
        rtpProxyPort = rtpProxyPort;
    }

    std::string getRtpProxyTimeoutSec() {
        return rtpProxyTimeoutSec;
    }

    void setRtpProxyTimeoutSec(std::string rtpProxyTimeoutSec) {
        rtpProxyTimeoutSec = rtpProxyTimeoutSec;
    }

    std::string getRtspAuthBasic() {
        return rtspAuthBasic;
    }

    void setRtspAuthBasic(std::string rtspAuthBasic) {
        rtspAuthBasic = rtspAuthBasic;
    }

    std::string getRtspHandshakeSecond() {
        return rtspHandshakeSecond;
    }

    void setRtspHandshakeSecond(std::string rtspHandshakeSecond) {
        rtspHandshakeSecond = rtspHandshakeSecond;
    }

    std::string getRtspKeepAliveSecond() {
        return rtspKeepAliveSecond;
    }

    void setRtspKeepAliveSecond(std::string rtspKeepAliveSecond) {
        rtspKeepAliveSecond = rtspKeepAliveSecond;
    }

    int getRtspPort() {
        return rtspPort;
    }

    void setRtspPort(int rtspPort) {
        rtspPort = rtspPort;
    }

    int getRtspSSlport() {
        return rtspSSlport;
    }

    void setRtspSSlport(int rtspSSlport) {
        rtspSSlport = rtspSSlport;
    }

    std::string getShellMaxReqSize() {
        return shellMaxReqSize;
    }

    void setShellMaxReqSize(std::string shellMaxReqSize) {
        shellMaxReqSize = shellMaxReqSize;
    }

    std::string getShellPhell() {
        return shellPhell;
    }

    void setShellPhell(std::string shellPhell) {
        shellPhell = shellPhell;
    }

    int getHookAliveInterval() {
        return hookAliveInterval;
    }

    void setHookAliveInterval(int hookAliveInterval) {
        hookAliveInterval = hookAliveInterval;
    }

    std::string getPortRange() {
        return portRange;
    }

    void setPortRange(std::string portRange) {
        portRange = portRange;
    }
};
#endif