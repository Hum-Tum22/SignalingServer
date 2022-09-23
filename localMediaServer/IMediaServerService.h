#pragma once
#include "localMediaServerInterface.h"
#include "../tools/ownString.h"
#include "SsrcConfig.h"
#include "zlmHttpInterface.h"
#include "../conf/UserSetting.h"
#include "../sesssion/VideoStreamSessionManager.h"
#include "ZLMRTPServerFactory.h"
#include "MediaConfig.h"
#include "MediaDbManager.h"
#include <string>

class MediaServerServiceImpl : public IMediaServerService {

    //private final static Logger logger = LoggerFactory.getLogger(MediaServerServiceImpl.class);

    //@Autowired
    //SipConfig sipConfig;

    //@Value("${server.ssl.enabled:false}")
    bool sslEnabled;

    //@Value("${server.port}")
    int serverPort;

    //@Autowired
    UserSetting userSetting;

    //@Autowired
    ZLMRESTfulUtils zlmresTfulUtils;

    ////@Autowired
    //MediaServerMapper mediaServerMapper;
    MediaServerDbManagr mediaServerMapper;

    ////@Autowired
    //DataSourceTransactionManager dataSourceTransactionManager;

    ////@Autowired
    //TransactionDefinition transactionDefinition;

    //@Autowired
    VideoStreamSessionManager streamSession;

    //@Autowired
    ZLMRTPServerFactory zlmrtpServerFactory;

    //@Autowired
    //RedisUtil redisUtil;
    std::map<std::string, MediaServerItem> mMediaServerItemMap;

    //@Autowired
    //IVideoManagerStorage storager;

    ////@Autowired
    //IStreamProxyService streamProxyService;

    ////@Autowired
    //EventPublisher publisher;

    ////@Autowired
    //JedisUtil jedisUtil;
    MediaConfig mediaConfig;

    std::map<std::string, bool> startGetMedia;
    /**
     * ��ʼ��
     */
    //@Override
    MediaServerServiceImpl();
    void connectZlmServer(MediaServerItem mediaServerItem);
    std::shared_ptr<ZLMServerConfig> getMediaServerConfig(MediaServerItem mediaServerItem);
    void updateVmServer(std::list<MediaServerItem>  mediaServerItemList)
    {
        //logger.info("[zlm] �����ʼ�� ");
        for (auto &mediaServerItem : mediaServerItemList) {
            if (mediaServerItem.getId().empty())
            {
                continue;
            }
            // ����
            if (mediaServerItem.getSsrcConfig().getMediaServerId().empty())
            {
                std::set<std::string> usedset;
                SsrcConfig ssrcConfig(mediaServerItem.getId(), usedset, ""/*sipConfig.getDomain()*/);
                mediaServerItem.setSsrcConfig(ssrcConfig);
                mMediaServerItemMap.insert(std::pair<std::string, MediaServerItem>(userSetting.getServerId() + "_" + mediaServerItem.getId(), mediaServerItem));
                //redisUtil.set(userSetting.getServerId() + "_" + mediaServerItem.getId(), mediaServerItem);
            }
            // ��ѯredis�Ƿ���ڴ�mediaServer
            std::string key = userSetting.getServerId() + "_" + mediaServerItem.getId();
            if (mMediaServerItemMap.find(key) == mMediaServerItemMap.end()) {
                mMediaServerItemMap.insert(std::pair<std::string, MediaServerItem>(key, mediaServerItem));
                //redisUtil.set(key, mediaServerItem);
            }

        }
    }

    //@Override
    SSRCInfo* openRTPServer(MediaServerItem mediaServerItem, std::string streamId, bool ssrcCheck, bool isPlayback)
    {
        return openRTPServer(mediaServerItem, streamId, "", ssrcCheck, isPlayback);
    }

    //@Override
    SSRCInfo* openRTPServer(MediaServerItem &mediaServerItem, std::string streamId, std::string presetSsrc, bool ssrcCheck, bool isPlayback)
    {
        if (mediaServerItem.getId().empty()) {
            return NULL;
        }
        // ��ȡmediaServer���õ�ssrc
        std::string key = userSetting.getServerId() + "_" + mediaServerItem.getId();

        SsrcConfig ssrcConfig = mediaServerItem.getSsrcConfig();
        if (ssrcConfig.getMediaServerId().empty()) {
            //logger.info("media server [ {} ] ssrcConfig is null", mediaServerItem.getId());
            return NULL;
        }
        else {
            std::string ssrc;
            if (!presetSsrc.empty()) {
                ssrc = presetSsrc;
            }
            else
            {
                if (isPlayback) {
                    ssrc = ssrcConfig.getPlayBackSsrc();
                }
                else {
                    ssrc = ssrcConfig.getPlaySsrc();
                }
            }

            if (streamId.empty()) {
                streamId = ownString::str_format("%08x", ssrc);
            }
            int rtpServerPort = mediaServerItem.getRtpProxyPort();
            if (mediaServerItem.isRtpEnable()) {
                rtpServerPort = zlmrtpServerFactory.createRTPServer(mediaServerItem, streamId, ssrcCheck ? std::stoi(ssrc.c_str()) : 0);
            }
            mMediaServerItemMap.insert(std::pair<std::string, MediaServerItem>(key, mediaServerItem));
            //redisUtil.set(key, mediaServerItem);
            return new SSRCInfo(rtpServerPort, ssrc, streamId);
        }
    }

    //@Override
    void closeRTPServer(std::string deviceId, std::string channelId, std::string stream)
    {
        std::string mediaServerId = streamSession.getMediaServerId(deviceId, channelId, stream);
        std::string ssrc = streamSession.getSSRC(deviceId, channelId, stream);
        MediaServerItem *mediaServerItem = getOne(mediaServerId);
        if (mediaServerItem != NULL)
        {
            std::string streamId = ownString::str_format("%s_%s", deviceId, channelId);
            zlmrtpServerFactory.closeRTPServer(*mediaServerItem, streamId);
            releaseSsrc(mediaServerItem->getId(), ssrc);
        }
        streamSession.remove(deviceId, channelId, stream);
    }

    //@Override
    void releaseSsrc(std::string mediaServerItemId, std::string ssrc) {
        MediaServerItem *mediaServerItem = getOne(mediaServerItemId);
        if (mediaServerItem == NULL || ssrc.empty()) {
            return;
        }
        SsrcConfig ssrcConfig = mediaServerItem->getSsrcConfig();
        ssrcConfig.releaseSsrc(ssrc);
        mediaServerItem->setSsrcConfig(ssrcConfig);
        std::string key = userSetting.getServerId() + "_" + mediaServerItem->getId();
        mMediaServerItemMap.insert(std::pair<std::string, MediaServerItem>(key, *mediaServerItem));
        //redisUtil.set(key, mediaServerItem);
    }

    /**
     * zlm ��������������������Ϣ�� TODO ������ʹ�õ��豸����ֹͣ����
     */
    //@Override
    void clearRTPServer(MediaServerItem mediaServerItem) {
        std::set<std::string> usedSet;
        mediaServerItem.setSsrcConfig(SsrcConfig(mediaServerItem.getId(), usedSet, ""/*sipConfig.getDomain()*/));
        mMediaServerItemMap.insert(std::pair<std::string, MediaServerItem>(userSetting.getServerId(), mediaServerItem));
        //redisUtil.zAdd(userSetting.getServerId(), mediaServerItem.getId(), 0);

    }


    //@Override
    void update(MediaServerItem mediaSerItem) {
        //mediaServerMapper.update(mediaSerItem);
        MediaServerItem *mediaServerItemInRedis = getOne(mediaSerItem.getId());
        MediaServerItem mediaServerItemInDataBase;// = mediaServerMapper.queryOne(mediaSerItem.getId());
        if (mediaServerItemInRedis != NULL /*&& mediaServerItemInRedis->getSsrcConfig() != NULL*/) {
            mediaServerItemInDataBase.setSsrcConfig(mediaServerItemInRedis->getSsrcConfig());
        }
        else {
            std::set<std::string> usedSet;
            mediaServerItemInDataBase.setSsrcConfig(
                SsrcConfig(
                    mediaServerItemInDataBase.getId(),
                    usedSet,
                    ""/*sipConfig.getDomain()*/
                )
            );
        }
        std::string key = userSetting.getServerId() + "_" + mediaServerItemInDataBase.getId();
        mMediaServerItemMap.insert(std::pair<std::string, MediaServerItem>(key, mediaServerItemInDataBase));
        //redisUtil.set(key, mediaServerItemInDataBase);
    }

    //@Override
    std::list<MediaServerItem> getAll() {
        std::list<MediaServerItem> result;
        //std::list<Object> mediaServerKeys = redisUtil.scan(ownString::str_format("%S*", userSetting.getServerId() + "_"));
        std::string onlineKey = userSetting.getServerId();
        //for (auto &item : mMediaServerItemMap) {
        //    std::string key = item.first;
        //    MediaServerItem mediaServerItem = (MediaServerItem)redisUtil.get(key);
        //    // ���״̬
        //    Double aDouble = redisUtil.zScore(onlineKey, mediaServerItem.getId());
        //    if (aDouble != null) {
        //        mediaServerItem.setStatus(true);
        //    }
        //    result.push_back(mediaServerItem);
        //}
        //result.sort((serverItem1, serverItem2)->{
        //    int sortResult = 0;
        //    LocalDateTime localDateTime1 = LocalDateTime.parse(serverItem1.getCreateTime(), DateUtil.formatter);
        //    LocalDateTime localDateTime2 = LocalDateTime.parse(serverItem2.getCreateTime(), DateUtil.formatter);

        //    sortResult = localDateTime1.compareTo(localDateTime2);
        //    return  sortResult;
        //});
        return result;
    }


   // @Override
    std::list<MediaServerItem> getAllFromDatabase() {
        std::list<MediaServerItem> result;
        return result;
        //return mediaServerMapper.queryAll();
    }

    //@Override
    std::list<MediaServerItem> getAllOnline() {
        std::string key = userSetting.getServerId();
        //Set<std::string> mediaServerIdSet = redisUtil.zRevRange(key, 0, -1);

        std::list<MediaServerItem> result;
        /*if (mediaServerIdSet != null && mediaServerIdSet.size() > 0) {
            for (String mediaServerId : mediaServerIdSet) {
                std::string serverKey = VideoManagerConstants.MEDIA_SERVER_PREFIX + userSetting.getServerId() + "_" + mediaServerId;
                result.push_back((MediaServerItem)redisUtil.get(serverKey));
            }
        }
        Collections.reverse(result);*/
        return result;
    }

    /**
     * ��ȡ����zlm������
     * @param mediaServerId ����id
     * @return MediaServerItem
     */
    //@Override
    MediaServerItem* getOne(std::string mediaServerId)
    {
        if (mediaServerId.empty())
        {
            return NULL;
        }
        std::string key = userSetting.getServerId() + "_" + mediaServerId;
        return NULL;// (MediaServerItem)redisUtil.get(key);
    }

    //@Override
    MediaServerItem getDefaultMediaServer()
    {
        return mediaServerMapper.queryDefault();
    }

    //@Override
    void clearMediaServerForOnline()
    {
        std::string key = userSetting.getServerId();
        //redisUtil.del(key);
    }

    //@Override
    std::map<std::string,std::string> add(MediaServerItem mediaServerItem)
    {
        std::map<std::string, std::string> result;
        mediaServerItem.setCreateTime(""/*DateUtil.getNow()*/);
        mediaServerItem.setUpdateTime(""/*DateUtil.getNow()*/);
        mediaServerItem.setHookAliveInterval(120);
        std::string responseJSON = zlmresTfulUtils.getMediaServerConfig(mediaServerItem);
        if (!responseJSON.empty())
        {
            //����json
            /*JSONArray data = responseJSON.getJSONArray("data");
            if (data != null && data.size() > 0) {
                ZLMServerConfig zlmServerConfig = JSON.parseObject(JSON.toJSONString(data.get(0)), ZLMServerConfig.class);
                if (mediaServerMapper.queryOne(zlmServerConfig.getGeneralMediaServerId()) != null) {
                    result.insert(std::pair<std::string, std::string>("errCode", "-1"));
                    result.insert(std::pair<std::string, std::string>("strMsg", ownString::str_format("����ʧ�ܣ�ý�����ID[%s] �Ѵ��ڣ����޸�ý�����������", zlmServerConfig.getGeneralMediaServerId())));
                    return result;
                }
                mediaServerItem.setId(zlmServerConfig.getGeneralMediaServerId());
                zlmServerConfig.setIp(mediaServerItem.getIp());
                mediaServerMapper.add(mediaServerItem);
                zlmServerOnline(zlmServerConfig);
                result.insert(std::pair<std::string, std::string>("errCode", "0"));
                result.insert(std::pair<std::string, std::string>("strMsg", "success"));
            }
            else {
                result.insert(std::pair<std::string, std::string>("errCode", "-1"));
                result.insert(std::pair<std::string, std::string>("strMsg", "����ʧ��"));
            }*/

        }
        else
        {
            result.insert(std::pair<std::string, std::string>("errCode", "-1"));
            result.insert(std::pair<std::string, std::string>("strMsg", "���Ϊ��"));
        }
        return result;
    }

    //@Override
    int addToDatabase(MediaServerItem mediaSerItem) {
        return 0;// mediaServerMapper.add(mediaSerItem);
    }

    //@Override
    int updateToDatabase(MediaServerItem mediaSerItem) {
        int result = 0;
        if (mediaSerItem.isDefaultServer()) {
            //�ύ�����ݿ�
            //TransactionStatus transactionStatus = dataSourceTransactionManager.getTransaction(transactionDefinition);
            //int delResult = mediaServerMapper.delDefault();
            //if (delResult == 0) {
            //    //logger.error("�Ƴ����ݿ�Ĭ��zlm�ڵ�ʧ��");
            //    //����ع�
            //    dataSourceTransactionManager.rollback(transactionStatus);
            //    return 0;
            //}
            //result = mediaServerMapper.add(mediaSerItem);
            //dataSourceTransactionManager.commit(transactionStatus);     //�ֶ��ύ
        }
        else {
            //result = mediaServerMapper.update(mediaSerItem);
        }
        return result;
    }

    /**
     * ����zlm����
     * @param zlmServerConfig zlm����Я���Ĳ���
     */
    //@Override
    void zlmServerOnline(ZLMServerConfig zlmServerConfig) {
        /*logger.info("[ZLM] �������� : {} -> {}:{}",
            zlmServerConfig.getGeneralMediaServerId(), zlmServerConfig.getIp(), zlmServerConfig.getHttpPort());*/

        MediaServerItem serverItem;// = mediaServerMapper.queryOne(zlmServerConfig.getGeneralMediaServerId());
        if (serverItem.getId().empty()) {
            //logger.warn("[δע���zlm] �ܽӽ��룺{}����{}��{}", zlmServerConfig.getGeneralMediaServerId(), zlmServerConfig.getIp(), zlmServerConfig.getHttpPort());
            //logger.warn("����ZLM��<general.mediaServerId>�����Ƿ���WVP��<media.id>һ��");
            return;
        }
        serverItem.setHookAliveInterval(zlmServerConfig.getHookAliveInterval());
        if (serverItem.getHttpPort() == 0) {
            serverItem.setHttpPort(zlmServerConfig.getHttpPort());
        }
        if (serverItem.getHttpSSlPort() == 0) {
            serverItem.setHttpSSlPort(zlmServerConfig.getHttpSSLport());
        }
        if (serverItem.getRtmpPort() == 0) {
            serverItem.setRtmpPort(zlmServerConfig.getRtmpPort());
        }
        if (serverItem.getRtmpSSlPort() == 0) {
            serverItem.setRtmpSSlPort(zlmServerConfig.getRtmpSslPort());
        }
        if (serverItem.getRtspPort() == 0) {
            serverItem.setRtspPort(zlmServerConfig.getRtspPort());
        }
        if (serverItem.getRtspSSLPort() == 0) {
            serverItem.setRtspSSLPort(zlmServerConfig.getRtspSSlport());
        }
        if (serverItem.getRtpProxyPort() == 0) {
            serverItem.setRtpProxyPort(zlmServerConfig.getRtpProxyPort());
        }
        serverItem.setStatus(true);

        if (serverItem.getId().empty()) {
            //logger.warn("[δע���zlm] serverItemȱ��ID�� �޷����룺{}��{}", zlmServerConfig.getIp(), zlmServerConfig.getHttpPort());
            return;
        }
        //mediaServerMapper.update(serverItem);
        std::string key = userSetting.getServerId() + "_" + zlmServerConfig.getGeneralMediaServerId();
        /*if (redisUtil.get(key) == null) {
            SsrcConfig ssrcConfig(zlmServerConfig.getGeneralMediaServerId(), null, sipConfig.getDomain());
            serverItem.setSsrcConfig(ssrcConfig);
        }
        else {
            MediaServerItem mediaServerItemInRedis = (MediaServerItem)redisUtil.get(key);
            serverItem.setSsrcConfig(mediaServerItemInRedis.getSsrcConfig());
        }
        redisUtil.set(key, serverItem);*/
        resetOnlineServerItem(serverItem);
        updateMediaServerKeepalive(serverItem.getId(), "");
        if (serverItem.isAutoConfig()) {
            setZLMConfig(serverItem, std::stoi(zlmServerConfig.getHookEnable()));
        }
        //publisher.zlmOnlineEventPublish(serverItem.getId());
        /*logger.info("[ZLM] ���ӳɹ� {} - {}:{} ",
            zlmServerConfig.getGeneralMediaServerId(), zlmServerConfig.getIp(), zlmServerConfig.getHttpPort());*/
    }


    //@Override
    void zlmServerOffline(std::string mediaServerId) {
        removeMediaItem(mediaServerId);
    }

    //@Override
    void resetOnlineServerItem(MediaServerItem serverItem) {
        // ���»���
        std::string key = userSetting.getServerId();
        // ʹ��zset�ķ�����Ϊ��ǰ�������� Ĭ��ֵ����Ϊ0
        //if (redisUtil.zScore(key, serverItem.getId()) == null)
        //{  
        //    // ������������Ĭ��ֵ �Ѵ���������
        //    redisUtil.zAdd(key, serverItem.getId(), 0L);
        //    // ��ѯ����������
        //    std::string responseJson = zlmresTfulUtils.getMediaList(serverItem, "", "", "rtmp");
        //    if (!responseJson.empty())
        //    {
        //        //json����
        //        /*Integer code = mediaList.getInteger("code");
        //        if (code == 0) {
        //            JSONArray data = mediaList.getJSONArray("data");
        //            if (data != null) {
        //                redisUtil.zAdd(key, serverItem.getId(), data.size());
        //            }
        //        }*/
        //    }
        //}
        //else {
        //    clearRTPServer(serverItem);
        //}

    }


    //@Override
    void addCount(std::string mediaServerId) {
        if (mediaServerId.empty()) {
            return;
        }
        std::string key = userSetting.getServerId();
        //redisUtil.zIncrScore(key, mediaServerId, 1);

    }

    //@Override
    void removeCount(std::string mediaServerId) {
        std::string key = userSetting.getServerId();
        //redisUtil.zIncrScore(key, mediaServerId, -1);
    }

    /**
     * ��ȡ������͵Ľڵ�
     * @return MediaServerItem
     */
    //@Override
    MediaServerItem* getMediaServerForMinimumLoad() {
        std::string key = userSetting.getServerId();

        //if (redisUtil.zSize(key) == null || redisUtil.zSize(key) == 0) {
        //    logger.info("��ȡ������͵Ľڵ�ʱ�����߽ڵ�");
        //    return null;
        //}

        //// ��ȡ������͵ģ���������͵�
        //Set<Object> objects = redisUtil.ZRange(key, 0, -1);
        //ArrayList<Object> mediaServerObjectS = new ArrayList<>(objects);

        std::string mediaServerId;// = (String)mediaServerObjectS.get(0);
        return getOne(mediaServerId);
    }

    /**
     * ��zlm���������л�������
     * @param mediaServerItem ����ID
     * @param restart �Ƿ�����zlm
     */
    //@Override
    void setZLMConfig(MediaServerItem mediaServerItem, bool restart)
    {
        /*logger.info("[ZLM] �������� ��{} -> {}:{}",
            mediaServerItem.getId(), mediaServerItem.getIp(), mediaServerItem.getHttpPort());*/
        std::string protocol = sslEnabled ? "https" : "http";
        std::string hookPrex = ownString::str_format("%s://%s:%s/index/hook", protocol, mediaServerItem.getHookIp(), std::to_string(serverPort));
        std::string recordHookPrex;
        if (mediaServerItem.getRecordAssistPort() != 0) {
            recordHookPrex = ownString::str_format("http://127.0.0.1:%s/api/record", std::to_string(mediaServerItem.getRecordAssistPort()));
        }
        std::map<std::string, std::string> param;
        param.insert(std::pair<std::string, std::string>("api.secret", mediaServerItem.getSecret()));
        //"%s -fflags nobuffer -i %s -c:a aac -strict -2 -ar 44100 -ab 48k -c:v libx264  -f flv %s"
        param.insert(std::pair<std::string, std::string>("ffmpeg.cmd", "%s -fflags nobuffer -i %s -c:a aac -strict -2 -ar 44100 -ab 48k -c:v libx264  -f flv %s"));
        param.insert(std::pair<std::string, std::string>("hook.enable", "1"));
        param.insert(std::pair<std::string, std::string>("hook.on_flow_report", ""));
        param.insert(std::pair<std::string, std::string>("hook.on_play", ownString::str_format("%s/on_play", hookPrex)));
        param.insert(std::pair<std::string, std::string>("hook.on_http_access", ""));
        param.insert(std::pair<std::string, std::string>("hook.on_publish", ownString::str_format("%s/on_publish", hookPrex)));
        param.insert(std::pair<std::string, std::string>("hook.on_record_mp4", ((!recordHookPrex.empty()) ? ownString::str_format("%s/on_record_mp4", recordHookPrex) : "")));
        param.insert(std::pair<std::string, std::string>("hook.on_record_ts", ""));
        param.insert(std::pair<std::string, std::string>("hook.on_rtsp_auth", ""));
        param.insert(std::pair<std::string, std::string>("hook.on_rtsp_realm", ""));
        param.insert(std::pair<std::string, std::string>("hook.on_server_started", ownString::str_format("%s/on_server_started", hookPrex)));
        param.insert(std::pair<std::string, std::string>("hook.on_shell_login", ownString::str_format("%s/on_shell_login", hookPrex)));
        param.insert(std::pair<std::string, std::string>("hook.on_stream_changed", ownString::str_format("%s/on_stream_changed", hookPrex)));
        param.insert(std::pair<std::string, std::string>("hook.on_stream_none_reader", ownString::str_format("%s/on_stream_none_reader", hookPrex)));
        param.insert(std::pair<std::string, std::string>("hook.on_stream_not_found", ownString::str_format("%s/on_stream_not_found", hookPrex)));
        param.insert(std::pair<std::string, std::string>("hook.on_server_keepalive", ownString::str_format("%s/on_server_keepalive", hookPrex)));
        param.insert(std::pair<std::string, std::string>("hook.timeoutSec", "20"));
        param.insert(std::pair<std::string, std::string>("general.streamNoneReaderDelayMS", (mediaServerItem.getStreamNoneReaderDelayMS() == -1 ? "3600000" : std::to_string(mediaServerItem.getStreamNoneReaderDelayMS()))));
        // �����Ͽ�������ڳ�ʱʱ�������������ϼ�����������������������Ų��š�
        // ��0�رմ�����(�����Ͽ��ᵼ�������Ͽ�������)
        // �˲�����Ӧ���ڲ�������ʱʱ��
        // �Ż�����Ϣ�Ը�����յ���ע���¼�
        param.insert(std::pair<std::string, std::string>("general.continue_push_ms", "3000"));
        // ���ȴ�δ��ʼ����Trackʱ�䣬��λ���룬��ʱ֮������δ��ʼ����Track, ���ô�ѡ���Ż���Щ��Ƶ����Ĳ��淶����
        // ��zlm֧�ָ�ÿ��rtpServer���ùر���Ƶ��ʱ����Բ����ô�ѡ��
        param.insert(std::pair<std::string, std::string>("general.wait_track_ready_ms", "3000"));
        if (mediaServerItem.isRtpEnable() && !mediaServerItem.getRtpPortRange().empty()) {
            std::string strRtpPortRange = mediaServerItem.getRtpPortRange();
            strRtpPortRange = strRtpPortRange.replace(strRtpPortRange.find("-"), 1, ",");
            param.insert(std::pair<std::string, std::string>("rtp_proxy.port_range", strRtpPortRange));
        }

        std::string responseJSON = zlmresTfulUtils.setServerConfig(mediaServerItem, param);

        if (!responseJSON.empty())
        {
            //json����
            /*&& responseJSON.getInteger("code") == 0
            if (restart) {
                logger.info("[ZLM] ���óɹ�,��ʼ�����Ա�֤������Ч {} -> {}:{}",
                    mediaServerItem.getId(), mediaServerItem.getIp(), mediaServerItem.getHttpPort());
                zlmresTfulUtils.restartServer(mediaServerItem);
            }
            else {
                logger.info("[ZLM] ���óɹ� {} -> {}:{}",
                    mediaServerItem.getId(), mediaServerItem.getIp(), mediaServerItem.getHttpPort());
            }*/
        }
        else
        {
            /*logger.info("[ZLM] ����zlmʧ�� {} -> {}:{}",
                mediaServerItem.getId(), mediaServerItem.getIp(), mediaServerItem.getHttpPort());*/
        }


    }


    //@Override
    std::map<std::string, std::string> checkMediaServer(std::string ip, int port, std::string secret) {
        std::map<std::string, std::string> result;
       /* if (mediaServerMapper.queryOneByHostAndPort(ip, port).em) {
            result.insert(std::pair<std::string, std::string>("errCode", "-1"));
            result.insert(std::pair<std::string, std::string>("strMsg", "�������Ѵ���"));
            return result;
        }*/
        MediaServerItem mediaServerItem;
        mediaServerItem.setIp(ip);
        mediaServerItem.setHttpPort(port);
        mediaServerItem.setSecret(secret);
        std::string responseJSON = zlmresTfulUtils.getMediaServerConfig(mediaServerItem);
        if (responseJSON.empty()) {
            result.insert(std::pair<std::string, std::string>("errCode", "-1"));
            result.insert(std::pair<std::string, std::string>("strMsg", "����ʧ��"));
            return result;
        }
        //����json
        /*JSONArray data = responseJSON.getJSONArray("data");
        ZLMServerConfig zlmServerConfig = JSON.parseObject(JSON.toJSONString(data.get(0)), ZLMServerConfig.class);
        if (zlmServerConfig == null) {
            result.insert(std::pair<std::string, std::string>("errCode", "-1"));
            result.insert(std::pair<std::string, std::string>("strMsg", "��ȡ����ʧ��"));
            return result;
        }
        if (mediaServerMapper.queryOne(zlmServerConfig.getGeneralMediaServerId()) != null) {
            result.insert(std::pair<std::string, std::string>("errCode", "-1"));
            result.insert(std::pair<std::string, std::string>("strMsg", ownString::str_format("ý�����ID [%s] �Ѵ��ڣ����޸�ý�����������", zlmServerConfig.getGeneralMediaServerId())));
            return result;
        }
        mediaServerItem.setHttpSSlPort(zlmServerConfig.getHttpPort());
        mediaServerItem.setRtmpPort(zlmServerConfig.getRtmpPort());
        mediaServerItem.setRtmpSSlPort(zlmServerConfig.getRtmpSslPort());
        mediaServerItem.setRtspPort(zlmServerConfig.getRtspPort());
        mediaServerItem.setRtspSSLPort(zlmServerConfig.getRtspSSlport());
        mediaServerItem.setRtpProxyPort(zlmServerConfig.getRtpProxyPort());
        mediaServerItem.setStreamIp(ip);
        mediaServerItem.setHookIp(sipConfig.getIp());
        mediaServerItem.setSdpIp(ip);
        mediaServerItem.setStreamNoneReaderDelayMS(zlmServerConfig.getGeneralStreamNoneReaderDelayMS());*/

        result.insert(std::pair<std::string, std::string>("errCode", "0"));
        result.insert(std::pair<std::string, std::string>("strMsg", "success"));
        result.insert(std::pair<std::string, std::string>("Data", mediaServerItem.getJsonString()));
        
        return result;
    }

    //@Override
    bool checkMediaRecordServer(std::string ip, int port) {
        bool result = false;
        std::string url = ownString::str_format("http://%s:%s/index/api/record", ip, std::to_string(port));

        std::string responseJSON = PostRequest(url, "");
        if (!responseJSON.empty())
        {
            printf("%s\n", responseJSON.c_str());
            /*Response response = client.newCall(request).execute();
            if (response != null) {
                result = true;
            }*/
        }
        return result;
    }

    //@Override
    void removeMediaItem(std::string id) {
        //redisUtil.zRemove(userSetting.getServerId(), id);
        std::string key = userSetting.getServerId() + "_" + id;
        //redisUtil.del(key);
    }
    //@Override
    void deleteDb(std::string id) {
        //ͬ��ɾ�����ݿ��е�����
        //mediaServerMapper.delOne(id);
    }

    //@Override
    void updateMediaServerKeepalive(std::string mediaServerId, std::string data) {
        MediaServerItem *mediaServerItem = getOne(mediaServerId);
        if (mediaServerItem == NULL) {
            // zlm��������

            //logger.warn("[����ZLM ������Ϣ]ʧ�ܣ�δ�ҵ���ý����Ϣ");
            return;
        }
        std::string key = userSetting.getServerId() + "_" + mediaServerId;
        int hookAliveInterval = mediaServerItem->getHookAliveInterval() + 2;
        //redisUtil.set(key, data, hookAliveInterval);
    }

    //@Override
    void syncCatchFromDatabase() {
        std::list<MediaServerItem> allInCatch = getAll();
        std::list<MediaServerItem> allInDatabase;// = mediaServerMapper.queryAll();
        std::map<std::string, MediaServerItem> mediaServerItemMap;

        for (MediaServerItem mediaServerItem : allInDatabase) {
            mediaServerItemMap.insert(std::pair<std::string, MediaServerItem>(mediaServerItem.getId(), mediaServerItem));
        }
        for (MediaServerItem mediaServerItem : allInCatch) {
            /*if (mediaServerItemMap.get(mediaServerItem) == null)
            {
                removeMediaItem(mediaServerItem.getId());
            }*/
        }
    }

};