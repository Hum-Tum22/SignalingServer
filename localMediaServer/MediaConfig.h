#pragma once
#include "MediaServerItem.h"
#include "../tools/m_Time.h"

#include <iostream>
#include <regex>

class MediaConfig {

    // 修改必须配置，不再支持自动获取
    //Value("${media.id}")
    std::string id;

    //Value("${media.ip}")
    std::string ip;

    //Value("${media.hook-ip:${sip.ip}}")
    std::string hookIp;

    //Value("${sip.ip}")
    std::string sipIp;

    //Value("${sip.domain}")
    std::string sipDomain;

    //Value("${media.sdp-ip:${media.ip}}")
    std::string sdpIp;

    //Value("${media.stream-ip:${media.ip}}")
    std::string streamIp;

    //Value("${media.http-port}")
    int httpPort;

    //Value("${media.http-ssl-port:0}")
    int httpSSlPort = 0;

    //Value("${media.rtmp-port:0}")
    int rtmpPort = 0;

    //Value("${media.rtmp-ssl-port:0}")
    int rtmpSSlPort = 0;

    //Value("${media.rtp-proxy-port:0}")
    int rtpProxyPort = 0;

    //Value("${media.rtsp-port:0}")
    int rtspPort = 0;

    //Value("${media.rtsp-ssl-port:0}")
    int rtspSSLPort = 0;

    //Value("${media.auto-config:true}")
    bool autoConfig = true;

    //Value("${media.secret}")
    std::string secret;

    //Value("${media.stream-none-reader-delay-ms:10000}")
    int streamNoneReaderDelayMS = 10000;

    //Value("${media.rtp.enable}")
    bool rtpEnable;

    //Value("${media.rtp.port-range}")
    std::string rtpPortRange;


    //Value("${media.rtp.send-port-range}")
    std::string sendRtpPortRange;

    //Value("${media.record-assist-port:0}")
    int recordAssistPort = 0;
public:
    std::string getId() {
        return id;
    }

    std::string getIp() {
        return ip;
    }

    std::string getHookIp() {
        if (hookIp.empty()) {
            return sipIp;
        }
        else {
            return hookIp;
        }

    }

    std::string getSipIp() {
        if (sipIp.empty()) {
            return ip;
        }
        else {
            return sipIp;
        }
    }

    int getHttpPort() {
        return httpPort;
    }

    int getHttpSSlPort() {
        return httpSSlPort;
    }

    int getRtmpPort() {
        return rtmpPort;
    }

    int getRtmpSSlPort() {
        return rtmpSSlPort;
    }

    int getRtpProxyPort() {
        if (rtpProxyPort < 0) {
            return 0;
        }
        else {
            return rtpProxyPort;
        }

    }

    int getRtspPort() {
        return rtspPort;
    }

    int getRtspSSLPort() {
        return rtspSSLPort;
    }

    bool isAutoConfig() {
        return autoConfig;
    }

    std::string getSecret() {
        return secret;
    }

    int getStreamNoneReaderDelayMS() {
        return streamNoneReaderDelayMS;
    }

    bool isRtpEnable() {
        return rtpEnable;
    }

    std::string getRtpPortRange() {
        return rtpPortRange;
    }

    int getRecordAssistPort() {
        return recordAssistPort;
    }

    std::string getSdpIp() {
        if (sdpIp.empty()) {
            return ip;
        }
        else {
            if (isValidIPAddress(sdpIp)) {
                return sdpIp;
            }
            else {
                // 按照域名解析
                std::string hostAddress;
                try {
                    hostAddress;// = InetAddress.getByName(sdpIp).getHostAddress();
                }
                catch (...) {
                    throw "域名解析错误";
                }
                return hostAddress;
            }
        }
    }

    std::string getStreamIp() {
        if (streamIp.empty()) {
            return ip;
        }
        else {
            return streamIp;
        }
    }

    std::string getSipDomain() {
        return sipDomain;
    }

    std::string getSendRtpPortRange() {
        return sendRtpPortRange;
    }

    MediaServerItem getMediaSerItem() {
        MediaServerItem mediaServerItem;// = new MediaServerItem();
        mediaServerItem.setId(id);
        mediaServerItem.setIp(ip);
        mediaServerItem.setDefaultServer(true);
        mediaServerItem.setHookIp(getHookIp());
        mediaServerItem.setSdpIp(getSdpIp());
        mediaServerItem.setStreamIp(getStreamIp());
        mediaServerItem.setHttpPort(httpPort);
        mediaServerItem.setHttpSSlPort(httpSSlPort);
        mediaServerItem.setRtmpPort(rtmpPort);
        mediaServerItem.setRtmpSSlPort(rtmpSSlPort);
        mediaServerItem.setRtpProxyPort(getRtpProxyPort());
        mediaServerItem.setRtspPort(rtspPort);
        mediaServerItem.setRtspSSLPort(rtspSSLPort);
        mediaServerItem.setAutoConfig(autoConfig);
        mediaServerItem.setSecret(secret);
        mediaServerItem.setStreamNoneReaderDelayMS(streamNoneReaderDelayMS);
        mediaServerItem.setRtpEnable(rtpEnable);
        mediaServerItem.setRtpPortRange(rtpPortRange);
        mediaServerItem.setSendRtpPortRange(sendRtpPortRange);
        mediaServerItem.setRecordAssistPort(recordAssistPort);
        mediaServerItem.setHookAliveInterval(120);
        CDateTime time0;
        mediaServerItem.setCreateTime(time0.tmFormat());
        mediaServerItem.setUpdateTime(time0.tmFormat());

        return mediaServerItem;
    }

    bool isValidIPAddress(std::string ipAddress) {
        if (ipAddress.empty())
        {
            //"^([1-9]|[1-9]\\d|1\\d{2}|2[0-4]\\d|25[0-5])(\\.(\\d|[1-9]\\d|1\\d{2}|2[0-4]\\d|25[0-5])){3}$"
            std::regex regExpress("(?=(\\b|\\D))(((\\d{1,2})|(1\\d{1,2})|(2[0-4]\\d)|(25[0-5]))\\.){3}((\\d{1,2})|(1\\d{1,2})|(2[0-4]\\d)|(25[0-5]))(?=(\\b|\\D))");
            return std::regex_match(ipAddress, regExpress);
        }
        return false;
    }

};