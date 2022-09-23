#include "IMediaServerService.h"


MediaServerServiceImpl::MediaServerServiceImpl()
{
    clearMediaServerForOnline();
    MediaServerItem defaultMediaServer = getDefaultMediaServer();
    if (0/*defaultMediaServer == null*/) {
        addToDatabase(mediaConfig.getMediaSerItem());
    }
    else {
        MediaServerItem mediaSerItem = mediaConfig.getMediaSerItem();
        updateToDatabase(mediaSerItem);
    }
    syncCatchFromDatabase();
    // ���� zlm�����¼�, �µ�zlmҲ����������ϵͳ
    /*hookSubscribe.addSubscribe(ZLMHttpHookSubscribe.HookType.on_server_started, new JSONObject(),
        (MediaServerItem mediaServerItem, JSONObject response)->{
        ZLMServerConfig zlmServerConfig = JSONObject.toJavaObject(response, ZLMServerConfig.class);
        if (zlmServerConfig != null) {
            if (startGetMedia != null) {
                startGetMedia.remove(zlmServerConfig.getGeneralMediaServerId());
            }
            zlmServerOnline(zlmServerConfig);
        }
    });*/

    // ���� zlm�����¼�, ��zlm����ʱ��ҵ��Ĵ���
    /*hookSubscribe.addSubscribe(ZLMHttpHookSubscribe.HookType.on_server_keepalive, new JSONObject(),
        (MediaServerItem mediaServerItem, JSONObject response)->{
        std::string mediaServerId = response.getString("mediaServerId");
        if (mediaServerId != null) {
            updateMediaServerKeepalive(mediaServerId, response.getJSONObject("data"));
        }
    });*/

    // ��ȡzlm��Ϣ
    //logger.info("[zlm] �ȴ�Ĭ��zlm��...");

    // ��ȡ���е�zlm�� ��������������
    std::list<MediaServerItem> all = getAllFromDatabase();
    updateVmServer(all);
    if (all.size() == 0) {
        all.push_back(mediaConfig.getMediaSerItem());
    }
    for (MediaServerItem mediaServerItem : all) {
        startGetMedia.insert(std::pair<std::string, bool>(mediaServerItem.getId(), true));
        connectZlmServer(mediaServerItem);
    }
    std::string taskKey = "zlm-connect-timeout";
    //dynamicTask.startDelay(taskKey, ()->{
    //    if (startGetMedia != null) {
    //        Set<std::string> allZlmId = startGetMedia.keySet();
    //        for (auto &id : allZlmId) {
    //            logger.error("[ {} ]]��������ʧ�ܣ����ٳ�������", id);
    //        }
    //        startGetMedia = null;
    //    }
    //    //  TODO �������ݿ�����redis��ƥ���zlm
    //}, 60 * 1000);
}
void MediaServerServiceImpl::connectZlmServer(MediaServerItem mediaServerItem)
{
    std::string connectZlmServerTaskKey = "connect-zlm-" + mediaServerItem.getId();
    std::shared_ptr<ZLMServerConfig> zlmServerConfigFirst = getMediaServerConfig(mediaServerItem);
    if (zlmServerConfigFirst != NULL) {
        zlmServerConfigFirst->setIp(mediaServerItem.getIp());
        zlmServerConfigFirst->setHttpPort(mediaServerItem.getHttpPort());
        startGetMedia.erase(mediaServerItem.getId());
        zlmServerOnline(*zlmServerConfigFirst.get());
    }
    else {
        /*logger.info("[ {} ]-[ {}:{} ]��������ʧ��, ���������Դ�� ��ʼ������������",
            mediaServerItem.getId(), mediaServerItem.getIp(), mediaServerItem.getHttpPort());*/
        //publisher.zlmOfflineEventPublish(mediaServerItem.getId());
    }

    /*dynamicTask.startCron(connectZlmServerTaskKey, ()->{
        ZLMServerConfig zlmServerConfig = getMediaServerConfig(mediaServerItem);
        if (zlmServerConfig != null) {
            dynamicTask.stop(connectZlmServerTaskKey);
            zlmServerConfig.setIp(mediaServerItem.getIp());
            zlmServerConfig.setHttpPort(mediaServerItem.getHttpPort());
            startGetMedia.remove(mediaServerItem.getId());
            zlmServerOnline(zlmServerConfig);
        }
    }, 2000);*/
}
std::shared_ptr<ZLMServerConfig> MediaServerServiceImpl::getMediaServerConfig(MediaServerItem mediaServerItem)
{
    if (!mediaServerItem.isDefaultServer() && getOne(mediaServerItem.getId()) == NULL)
    {
        return NULL;
    }
    if (startGetMedia.find(mediaServerItem.getId()) == startGetMedia.end() || !startGetMedia.find(mediaServerItem.getId())->second)
    {
        return NULL;
    }
    std::string responseJson = zlmresTfulUtils.getMediaServerConfig(mediaServerItem);
    std::shared_ptr<ZLMServerConfig> zlmServerConfig = std::make_shared<ZLMServerConfig>();
    if (!responseJson.empty()) {
        //����json
        /*JSONArray data = responseJson.getJSONArray("data");
        if (data != null && data.size() > 0) {
            zlmServerConfig = JSON.parseObject(JSON.toJSONString(data.get(0)), ZLMServerConfig.class);
        }*/
    }
    else {
        /*logger.error("[ {} ]-[ {}:{} ]��������ʧ��, 2s������",
            mediaServerItem.getId(), mediaServerItem.getIp(), mediaServerItem.getHttpPort());*/
    }
    return zlmServerConfig;

}