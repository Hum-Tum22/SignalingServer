#ifndef LOCAL_MEDIA_SERVER_INTERFACE_H_
#define LOCAL_MEDIA_SERVER_INTERFACE_H_
#include <list>
#include "MediaServerItem.h"
#include "ZLMServerConfig.h"
#include "SSRCInfo.h"

int StartOneLiveMediaStream(const std::string& channelId, const std::string& fromId, const std::string& callId, const std::string& ssrc);
// vod => Video On Demand
int StartOneVodMediaStream();


/**
 * 媒体服务节点
 */
class IMediaServerService
{
public:
    virtual std::list<MediaServerItem> getAll() = 0;

    virtual std::list<MediaServerItem> getAllFromDatabase() = 0;

    virtual std::list<MediaServerItem> getAllOnline() = 0;

    virtual MediaServerItem *getOne(std::string generalMediaServerId) = 0;

    virtual void syncCatchFromDatabase() = 0;

    /**
     * 新的节点加入
     * @param zlmServerConfig
     * @return
     */
    virtual void zlmServerOnline(ZLMServerConfig zlmServerConfig) = 0;

    /**
     * 节点离线
     * @param mediaServerId
     * @return
     */
    virtual void zlmServerOffline(std::string mediaServerId) = 0;

    virtual MediaServerItem *getMediaServerForMinimumLoad() = 0;

    virtual void setZLMConfig(MediaServerItem mediaServerItem, bool restart) = 0;

    virtual void updateVmServer(std::list<MediaServerItem>  mediaServerItemList) = 0;

    virtual SSRCInfo *openRTPServer(MediaServerItem mediaServerItem, std::string streamId, bool ssrcCheck, bool isPlayback) = 0;

    virtual SSRCInfo *openRTPServer(MediaServerItem mediaServerItem, std::string streamId, std::string ssrc, bool ssrcCheck, bool isPlayback) = 0;

    virtual void closeRTPServer(std::string deviceId, std::string channelId, std::string ssrc) = 0;

    virtual void clearRTPServer(MediaServerItem mediaServerItem) = 0;

    virtual void update(MediaServerItem mediaSerItem) = 0;

    virtual void addCount(std::string mediaServerId) = 0;

    virtual void removeCount(std::string mediaServerId) = 0;

    virtual void releaseSsrc(std::string mediaServerItemId, std::string ssrc) = 0;

    virtual void clearMediaServerForOnline() = 0;

    virtual std::map<std::string, std::string> add(MediaServerItem mediaSerItem) = 0;

    virtual int addToDatabase(MediaServerItem mediaSerItem) = 0;

    virtual int updateToDatabase(MediaServerItem mediaSerItem) = 0;

    virtual void resetOnlineServerItem(MediaServerItem serverItem) = 0;

    virtual std::map<std::string, std::string> checkMediaServer(std::string ip, int port, std::string secret) = 0;

    virtual bool checkMediaRecordServer(std::string ip, int port) = 0;

    virtual void removeMediaItem(std::string id) = 0;

    virtual void deleteDb(std::string id) = 0;

    virtual MediaServerItem getDefaultMediaServer() = 0;

    virtual void updateMediaServerKeepalive(std::string mediaServerId, std::string data) = 0;
};
#endif