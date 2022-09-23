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
    // 订阅 zlm启动事件, 新的zlm也会从这里进入系统
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

    // 订阅 zlm保活事件, 当zlm离线时做业务的处理
    /*hookSubscribe.addSubscribe(ZLMHttpHookSubscribe.HookType.on_server_keepalive, new JSONObject(),
        (MediaServerItem mediaServerItem, JSONObject response)->{
        std::string mediaServerId = response.getString("mediaServerId");
        if (mediaServerId != null) {
            updateMediaServerKeepalive(mediaServerId, response.getJSONObject("data"));
        }
    });*/

    // 获取zlm信息
    //logger.info("[zlm] 等待默认zlm中...");

    // 获取所有的zlm， 并开启主动连接
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
    //            logger.error("[ {} ]]主动连接失败，不再尝试连接", id);
    //        }
    //        startGetMedia = null;
    //    }
    //    //  TODO 清理数据库中与redis不匹配的zlm
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
        /*logger.info("[ {} ]-[ {}:{} ]主动连接失败, 清理相关资源， 开始尝试重试连接",
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
        //解析json
        /*JSONArray data = responseJson.getJSONArray("data");
        if (data != null && data.size() > 0) {
            zlmServerConfig = JSON.parseObject(JSON.toJSONString(data.get(0)), ZLMServerConfig.class);
        }*/
    }
    else {
        /*logger.error("[ {} ]-[ {}:{} ]主动连接失败, 2s后重试",
            mediaServerItem.getId(), mediaServerItem.getIp(), mediaServerItem.getHttpPort());*/
    }
    return zlmServerConfig;

}