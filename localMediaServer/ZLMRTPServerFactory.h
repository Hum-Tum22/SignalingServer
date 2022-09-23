#pragma once

#include "zlmHttpInterface.h"
#include "../sesssion/SendRtpItem.h"
#include "../tools/genuuid.h"
#include <string>

class ZLMRTPServerFactory {

    //@Autowired
    ZLMRESTfulUtils zlmresTfulUtils;

    int portRangeArray[2];
public:
    int getFreePort(MediaServerItem mediaServerItem, int startPort, int endPort, std::set<int> usedFreelist) {
        if (endPort <= startPort) {
            return -1;
        }
        std::string listRtpServerJsonResult = zlmresTfulUtils.listRtpServer(mediaServerItem);
        if (!listRtpServerJsonResult.empty())
        {
            //解析json
            /*JSONArray data = listRtpServerJsonResult.getJSONArray("data");
            if (data != null) {
                for (int i = 0; i < data.size(); i++) {
                    JSONObject dataItem = data.getJSONObject(i);
                    usedFreelist.add(dataItem.getint("port"));
                }
            }*/
        }

        std::map<std::string, std::string> param;
        int result = -1;
        // 设置推流端口
        if (startPort % 2 == 1) {
            startPort++;
        }
        bool checkPort = false;
        for (int i = startPort; i < endPort + 1; i += 2) {
            if (!usedFreelist.count(i)) {
                checkPort = true;
                startPort = i;
                break;
            }
        }
        if (!checkPort) {
            //logger.warn("未找到节点{}上范围[{}-{}]的空闲端口", mediaServerItem.getId(), startPort, endPort);
            return -1;
        }
        param.insert(std::pair<std::string, std::string>("port", std::to_string(startPort)));
        std::string stream(imuuid::uuidgen());
        param.insert(std::pair<std::string, std::string>("enable_tcp", "1"));
        param.insert(std::pair<std::string, std::string>("stream_id", stream));
        param.insert(std::pair<std::string, std::string>("port", "0"));
        std::string openRtpServerResultJson = zlmresTfulUtils.openRtpServer(mediaServerItem, param);

        if (!openRtpServerResultJson.empty()) {
            //解析json
            /*if (openRtpServerResultJson.getint("code") == 0) {
                result = openRtpServerResultJson.getint("port");
                Map<String, Object> closeRtpServerParam = new HashMap<>();
                closeRtpServerParam.put("stream_id", stream);
                zlmresTfulUtils.closeRtpServer(mediaServerItem, closeRtpServerParam);
            }
            else {
                usedFreelist.add(startPort);
                startPort += 2;
                result = getFreePort(mediaServerItem, startPort, endPort, usedFreelist);
            }*/
        }
        else {
            //  检查ZLM状态
            //logger.error("创建RTP Server 失败 {}: 请检查ZLM服务", param.get("port"));
        }
        return result;
    }

    int createRTPServer(MediaServerItem mediaServerItem, std::string streamId, int ssrc) {
        int result = -1;
        // 查询此rtp server 是否已经存在
        std::string rtpInfo = zlmresTfulUtils.getRtpInfo(mediaServerItem, streamId);
        if (!rtpInfo.empty())
        {
            //解析json
            /*if (rtpInfo != null && rtpInfo.getint("code") == 0 && rtpInfo.getbool("exist")) {
                result = rtpInfo.getint("local_port");
                return result;
            }*/
        }
        
        std::map<std::string, std::string> param;
        // 推流端口设置0则使用随机端口
        param.insert(std::pair<std::string, std::string>("enable_tcp", "1"));
        param.insert(std::pair<std::string, std::string>("stream_id", streamId));
        param.insert(std::pair<std::string, std::string>("port", "0"));
        param.insert(std::pair<std::string, std::string>("ssrc", std::to_string(ssrc)));
        std::string openRtpServerResultJson = zlmresTfulUtils.openRtpServer(mediaServerItem, param);

        if (!openRtpServerResultJson.empty())
        {
            //解析json
            /*if (openRtpServerResultJson.getint("code") == 0) {
                result = openRtpServerResultJson.getint("port");
            }
            else {
                logger.error("创建RTP Server 失败 {}: ", openRtpServerResultJson.getString("msg"));
            }*/
        }
        else {
            //  检查ZLM状态
            //logger.error("创建RTP Server 失败 {}: 请检查ZLM服务", param.get("port"));
        }
        return result;
    }

    bool closeRTPServer(MediaServerItem serverItem, std::string streamId) {
        bool result = false;
        if (!serverItem.getId().empty()) {
            std::map<std::string, std::string> param;
            param.insert(std::pair<std::string, std::string>("stream_id", streamId));
            std::string jsonObject = zlmresTfulUtils.closeRtpServer(serverItem, param);
            if (!jsonObject.empty()) {
                //解析json
                /*if (jsonObject.getint("code") == 0) {
                    result = jsonObject.getint("hit") == 1;
                }
                else {
                    logger.error("关闭RTP Server 失败: " + jsonObject.getString("msg"));
                }*/
            }
            else {
                //  检查ZLM状态
                //logger.error("关闭RTP Server 失败: 请检查ZLM服务");
            }
        }
        return result;
    }

    /**
        * 创建一个国标推流
        * @param ip 推流ip
        * @param port 推流端口
        * @param ssrc 推流唯一标识
        * @param platformId 平台id
        * @param channelId 通道id
        * @param tcp 是否为tcp
        * @return SendRtpItem
        */
    SendRtpItem createSendRtpItem(MediaServerItem serverItem, std::string ip, int port, std::string ssrc, std::string platformId, std::string deviceId, std::string channelId, bool tcp) {

        // 使用RTPServer 功能找一个可用的端口
        SendRtpItem sendRtpItem;
        std::string sendRtpPortRange = serverItem.getSendRtpPortRange();
        if (sendRtpPortRange.empty()) {
            return sendRtpItem;
        }
        std::vector<std::string> portRangeStrArray = ownString::vStringSplit(serverItem.getSendRtpPortRange(), ",");
        int localPort = -1; 
        std::set<int> usedFreelist;
        if (portRangeStrArray.size() != 2) {
            localPort = getFreePort(serverItem, 30000, 30500, usedFreelist);
        }
        else {
            localPort = getFreePort(serverItem, std::stoi(portRangeStrArray[0]), std::stoi(portRangeStrArray[1]), usedFreelist);
        }
        if (localPort == -1) {
            //logger.error("没有可用的端口");
            return sendRtpItem;
        }
        sendRtpItem.setIp(ip);
        sendRtpItem.setPort(port);
        sendRtpItem.setSsrc(ssrc);
        sendRtpItem.setPlatformId(platformId);
        sendRtpItem.setDeviceId(deviceId);
        sendRtpItem.setChannelId(channelId);
        sendRtpItem.setTcp(tcp);
        sendRtpItem.setApp("rtp");
        sendRtpItem.setLocalPort(localPort);
        sendRtpItem.setMediaServerId(serverItem.getId());
        return sendRtpItem;
    }

    /**
     * 创建一个直播推流
     * @param ip 推流ip
     * @param port 推流端口
     * @param ssrc 推流唯一标识
     * @param platformId 平台id
     * @param channelId 通道id
     * @param tcp 是否为tcp
     * @return SendRtpItem
     */
    SendRtpItem createSendRtpItem(MediaServerItem serverItem, std::string ip, int port, std::string ssrc, std::string platformId, std::string app, std::string stream, std::string channelId, bool tcp) {
        // 使用RTPServer 功能找一个可用的端口

        SendRtpItem sendRtpItem;
        std::string sendRtpPortRange = serverItem.getSendRtpPortRange();
        if (sendRtpPortRange.empty()) {
            return sendRtpItem;
        }
        std::vector<std::string> portRangeStrArray = ownString::vStringSplit(serverItem.getSendRtpPortRange(), ",");
        int localPort = -1;
        std::set<int> usedFreelist;
        if (portRangeStrArray.size() != 2) {
            localPort = getFreePort(serverItem, 30000, 30500, usedFreelist);
        }
        else {
            localPort = getFreePort(serverItem, std::stoi(portRangeStrArray[0]), std::stoi(portRangeStrArray[1]), usedFreelist);
        }
        if (localPort == -1) {
            //logger.error("没有可用的端口");
            return sendRtpItem;
        }
        sendRtpItem.setIp(ip);
        sendRtpItem.setPort(port);
        sendRtpItem.setSsrc(ssrc);
        sendRtpItem.setApp(app);
        sendRtpItem.setStreamId(stream);
        sendRtpItem.setPlatformId(platformId);
        sendRtpItem.setChannelId(channelId);
        sendRtpItem.setTcp(tcp);
        sendRtpItem.setLocalPort(localPort);
        sendRtpItem.setMediaServerId(serverItem.getId());
        return sendRtpItem;
    }

    /**
     * 调用zlm RESTFUL API —— startSendRtp
     */
    std::string startSendRtpStream(MediaServerItem mediaServerItem, std::map<std::string, std::string>param) {
        return zlmresTfulUtils.startSendRtp(mediaServerItem, param);
    }

    /**
     * 查询待转推的流是否就绪
     */
    bool isRtpReady(MediaServerItem mediaServerItem, std::string streamId) {
        std::string mediaInfo = zlmresTfulUtils.getMediaInfo(mediaServerItem, "rtp", "rtmp", streamId);
        if (!mediaInfo.empty())
        {
            //解析json
            //if(mediaInfo.find("code") != std::size_type::npos )
            //(mediaInfo.getInteger("code") == 0 && mediaInfo.getBoolean("online"));
        }
        return false;
    }

    /**
     * 查询待转推的流是否就绪
     */
    bool isStreamReady(MediaServerItem mediaServerItem, std::string app, std::string streamId) {
        std::string mediaInfo = zlmresTfulUtils.getMediaInfo(mediaServerItem, app, "rtmp", streamId);
        if (!mediaInfo.empty())
        {
            //解析json
            //if(mediaInfo.find("code") != std::size_type::npos )
        }
        return false;
        //return (mediaInfo.getint("code") == 0 && mediaInfo.getbool("online"));
    }

    /**
     * 查询转推的流是否有其它观看者
     * @param streamId
     * @return
     */
    int totalReaderCount(MediaServerItem mediaServerItem, std::string app, std::string streamId) {
        std::string mediaInfo = zlmresTfulUtils.getMediaInfo(mediaServerItem, app, "rtmp", streamId);
        if (!mediaInfo.empty())
        {
            // 解析json
        }
        return false;
        /*int code = mediaInfo.getint("code");
        if (mediaInfo == null) {
            return 0;
        }
        if (code < 0) {
            logger.warn("查询流({}/{})是否有其它观看者时得到： {}", app, streamId, mediaInfo.getString("msg"));
            return -1;
        }
        if (code == 0 && !mediaInfo.getbool("online")) {
            logger.warn("查询流({}/{})是否有其它观看者时得到： {}", app, streamId, mediaInfo.getString("msg"));
            return -1;
        }
        return mediaInfo.getint("totalReaderCount");*/
    }

    /**
     * 调用zlm RESTful API —— stopSendRtp
     */
    bool stopSendRtpStream(MediaServerItem mediaServerItem, std::map<std::string, std::string>param) {
        bool result = false;
        std::string jsonObject = zlmresTfulUtils.stopSendRtp(mediaServerItem, param);
        if (!jsonObject.empty())
        {
            //解析json
        }
        return false;
        /*if (jsonObject == null) {
            logger.error("停止RTP推流失败: 请检查ZLM服务");
        }
        else if (jsonObject.getint("code") == 0) {
            result = true;
            logger.info("停止RTP推流成功");
        }
        else {
            logger.error("停止RTP推流失败: {}, 参数：{}", jsonObject.getString("msg"), JSONObject.toJSON(param));
        }*/
        return result;
    }

    void closeAllSendRtpStream() {

    }
};