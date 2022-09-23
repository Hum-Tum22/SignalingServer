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
            //����json
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
        // ���������˿�
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
            //logger.warn("δ�ҵ��ڵ�{}�Ϸ�Χ[{}-{}]�Ŀ��ж˿�", mediaServerItem.getId(), startPort, endPort);
            return -1;
        }
        param.insert(std::pair<std::string, std::string>("port", std::to_string(startPort)));
        std::string stream(imuuid::uuidgen());
        param.insert(std::pair<std::string, std::string>("enable_tcp", "1"));
        param.insert(std::pair<std::string, std::string>("stream_id", stream));
        param.insert(std::pair<std::string, std::string>("port", "0"));
        std::string openRtpServerResultJson = zlmresTfulUtils.openRtpServer(mediaServerItem, param);

        if (!openRtpServerResultJson.empty()) {
            //����json
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
            //  ���ZLM״̬
            //logger.error("����RTP Server ʧ�� {}: ����ZLM����", param.get("port"));
        }
        return result;
    }

    int createRTPServer(MediaServerItem mediaServerItem, std::string streamId, int ssrc) {
        int result = -1;
        // ��ѯ��rtp server �Ƿ��Ѿ�����
        std::string rtpInfo = zlmresTfulUtils.getRtpInfo(mediaServerItem, streamId);
        if (!rtpInfo.empty())
        {
            //����json
            /*if (rtpInfo != null && rtpInfo.getint("code") == 0 && rtpInfo.getbool("exist")) {
                result = rtpInfo.getint("local_port");
                return result;
            }*/
        }
        
        std::map<std::string, std::string> param;
        // �����˿�����0��ʹ������˿�
        param.insert(std::pair<std::string, std::string>("enable_tcp", "1"));
        param.insert(std::pair<std::string, std::string>("stream_id", streamId));
        param.insert(std::pair<std::string, std::string>("port", "0"));
        param.insert(std::pair<std::string, std::string>("ssrc", std::to_string(ssrc)));
        std::string openRtpServerResultJson = zlmresTfulUtils.openRtpServer(mediaServerItem, param);

        if (!openRtpServerResultJson.empty())
        {
            //����json
            /*if (openRtpServerResultJson.getint("code") == 0) {
                result = openRtpServerResultJson.getint("port");
            }
            else {
                logger.error("����RTP Server ʧ�� {}: ", openRtpServerResultJson.getString("msg"));
            }*/
        }
        else {
            //  ���ZLM״̬
            //logger.error("����RTP Server ʧ�� {}: ����ZLM����", param.get("port"));
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
                //����json
                /*if (jsonObject.getint("code") == 0) {
                    result = jsonObject.getint("hit") == 1;
                }
                else {
                    logger.error("�ر�RTP Server ʧ��: " + jsonObject.getString("msg"));
                }*/
            }
            else {
                //  ���ZLM״̬
                //logger.error("�ر�RTP Server ʧ��: ����ZLM����");
            }
        }
        return result;
    }

    /**
        * ����һ����������
        * @param ip ����ip
        * @param port �����˿�
        * @param ssrc ����Ψһ��ʶ
        * @param platformId ƽ̨id
        * @param channelId ͨ��id
        * @param tcp �Ƿ�Ϊtcp
        * @return SendRtpItem
        */
    SendRtpItem createSendRtpItem(MediaServerItem serverItem, std::string ip, int port, std::string ssrc, std::string platformId, std::string deviceId, std::string channelId, bool tcp) {

        // ʹ��RTPServer ������һ�����õĶ˿�
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
            //logger.error("û�п��õĶ˿�");
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
     * ����һ��ֱ������
     * @param ip ����ip
     * @param port �����˿�
     * @param ssrc ����Ψһ��ʶ
     * @param platformId ƽ̨id
     * @param channelId ͨ��id
     * @param tcp �Ƿ�Ϊtcp
     * @return SendRtpItem
     */
    SendRtpItem createSendRtpItem(MediaServerItem serverItem, std::string ip, int port, std::string ssrc, std::string platformId, std::string app, std::string stream, std::string channelId, bool tcp) {
        // ʹ��RTPServer ������һ�����õĶ˿�

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
            //logger.error("û�п��õĶ˿�");
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
     * ����zlm RESTFUL API ���� startSendRtp
     */
    std::string startSendRtpStream(MediaServerItem mediaServerItem, std::map<std::string, std::string>param) {
        return zlmresTfulUtils.startSendRtp(mediaServerItem, param);
    }

    /**
     * ��ѯ��ת�Ƶ����Ƿ����
     */
    bool isRtpReady(MediaServerItem mediaServerItem, std::string streamId) {
        std::string mediaInfo = zlmresTfulUtils.getMediaInfo(mediaServerItem, "rtp", "rtmp", streamId);
        if (!mediaInfo.empty())
        {
            //����json
            //if(mediaInfo.find("code") != std::size_type::npos )
            //(mediaInfo.getInteger("code") == 0 && mediaInfo.getBoolean("online"));
        }
        return false;
    }

    /**
     * ��ѯ��ת�Ƶ����Ƿ����
     */
    bool isStreamReady(MediaServerItem mediaServerItem, std::string app, std::string streamId) {
        std::string mediaInfo = zlmresTfulUtils.getMediaInfo(mediaServerItem, app, "rtmp", streamId);
        if (!mediaInfo.empty())
        {
            //����json
            //if(mediaInfo.find("code") != std::size_type::npos )
        }
        return false;
        //return (mediaInfo.getint("code") == 0 && mediaInfo.getbool("online"));
    }

    /**
     * ��ѯת�Ƶ����Ƿ��������ۿ���
     * @param streamId
     * @return
     */
    int totalReaderCount(MediaServerItem mediaServerItem, std::string app, std::string streamId) {
        std::string mediaInfo = zlmresTfulUtils.getMediaInfo(mediaServerItem, app, "rtmp", streamId);
        if (!mediaInfo.empty())
        {
            // ����json
        }
        return false;
        /*int code = mediaInfo.getint("code");
        if (mediaInfo == null) {
            return 0;
        }
        if (code < 0) {
            logger.warn("��ѯ��({}/{})�Ƿ��������ۿ���ʱ�õ��� {}", app, streamId, mediaInfo.getString("msg"));
            return -1;
        }
        if (code == 0 && !mediaInfo.getbool("online")) {
            logger.warn("��ѯ��({}/{})�Ƿ��������ۿ���ʱ�õ��� {}", app, streamId, mediaInfo.getString("msg"));
            return -1;
        }
        return mediaInfo.getint("totalReaderCount");*/
    }

    /**
     * ����zlm RESTful API ���� stopSendRtp
     */
    bool stopSendRtpStream(MediaServerItem mediaServerItem, std::map<std::string, std::string>param) {
        bool result = false;
        std::string jsonObject = zlmresTfulUtils.stopSendRtp(mediaServerItem, param);
        if (!jsonObject.empty())
        {
            //����json
        }
        return false;
        /*if (jsonObject == null) {
            logger.error("ֹͣRTP����ʧ��: ����ZLM����");
        }
        else if (jsonObject.getint("code") == 0) {
            result = true;
            logger.info("ֹͣRTP�����ɹ�");
        }
        else {
            logger.error("ֹͣRTP����ʧ��: {}, ������{}", jsonObject.getString("msg"), JSONObject.toJSON(param));
        }*/
        return result;
    }

    void closeAllSendRtpStream() {

    }
};