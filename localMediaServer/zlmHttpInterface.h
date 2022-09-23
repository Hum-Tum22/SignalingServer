#ifndef ZLM_HTTP_INTERFACE_H_
#define ZLM_HTTP_INTERFACE_H_
#include "../http.h"
#include "MediaServerItem.h"
#include "../tools/ownString.h"


#include <map>

class ZLMRESTfulUtils
{
public:
    std::string sendPost(MediaServerItem mediaServerItem, std::string api, std::map<std::string, std::string> params)
    {
        if (mediaServerItem.getId().empty())
        {
            return "";
        }
        std::string url = ownString::str_format("http://%s:%s/index/api/%s", mediaServerItem.getIp(), std::to_string(mediaServerItem.getHttpPort()), api);
        int paramCount = 0;
        ostringstream ds;
        ds << url;
        for (auto& it : params)
        {
            if (paramCount == 0)
            {
                ds << "?" << it.first << "=" << it.second;
            }
            else
            {
                ds << "&" << it.first << "=" << it.second;
            }
            paramCount++;
        }
        ds.flush();
        std::string responseJSON;
        std::string requestJson;

        //构建json字符串

        //发送post请求
        responseJSON = PostRequest(ds.str(), requestJson);
        if (!responseJSON.empty())
        {
            printf("%s\n", responseJSON.c_str());
        }
        return responseJSON;
    }

    void sendGetForImg(MediaServerItem mediaServerItem, std::string api, std::map<std::string, std::string> params, std::string targetPath, std::string fileName) {
        std::string url = ownString::str_format("http://%s:%s/index/api/%s", mediaServerItem.getIp(), std::to_string(mediaServerItem.getHttpPort()), api);
        int paramCount = 0;
        ostringstream ds;
        ds << url;
        for (auto& it : params)
        {
            if (paramCount == 0)
            {
                ds << "?" << it.first << "=" << it.second;
            }
            else
            {
                ds << "&" << it.first << "=" << it.second;
            }
            paramCount++;
        }
        ds.flush();
        std::string responseJSON = PostRequest(ds.str(), "");
        if (!responseJSON.empty())
        {
            printf("%s\n", responseJSON.c_str());
        }
    }

    std::string getMediaList(MediaServerItem mediaServerItem, std::string app, std::string stream, std::string schema)
    {
        std::map<std::string, std::string> param;
        if (!app.empty())
        {
            param.insert(std::pair<std::string,std::string>("app", app));
        }
        if (!stream.empty())
        {
            param.insert(std::pair<std::string, std::string>("stream", stream));
        }
        if (!schema.empty())
        {
            param.insert(std::pair<std::string, std::string>("schema", schema));
        }
        param.insert(std::pair<std::string, std::string>("vhost", "__defaultVhost__"));
        return sendPost(mediaServerItem, "getMediaList", param);
    }

    std::string getMediaList(MediaServerItem mediaServerItem, std::string app, std::string stream) {
        return getMediaList(mediaServerItem, app, stream, "");
    }

    std::string getMediaList(MediaServerItem mediaServerItem) {
        std::map<std::string, std::string> param;
        return sendPost(mediaServerItem, "getMediaList", param);
    }

    std::string getMediaInfo(MediaServerItem mediaServerItem, std::string app, std::string schema, std::string stream) {
        std::map<std::string, std::string> param;
        param.insert(std::pair<std::string, std::string>("app", app));
        param.insert(std::pair<std::string, std::string>("schema", schema));
        param.insert(std::pair<std::string, std::string>("stream", stream));
        param.insert(std::pair<std::string, std::string>("vhost", "__defaultVhost__"));
        return sendPost(mediaServerItem, "getMediaInfo", param);
    }

    std::string getRtpInfo(MediaServerItem mediaServerItem, std::string stream_id) {
        std::map<std::string, std::string> param;
        param.insert(std::pair<std::string, std::string>("stream_id", stream_id));
        return sendPost(mediaServerItem, "getRtpInfo", param);
    }

    std::string addFFmpegSource(MediaServerItem mediaServerItem, std::string src_url, std::string dst_url, std::string timeout_ms,
        bool enable_hls, bool enable_mp4, std::string ffmpeg_cmd_key) {
        std::map<std::string, std::string> param;
        param.insert(std::pair<std::string, std::string>("src_url", src_url));
        param.insert(std::pair<std::string, std::string>("dst_url", dst_url));
        param.insert(std::pair<std::string, std::string>("timeout_ms", timeout_ms));
        param.insert(std::pair<std::string, std::string>("enable_hls", std::to_string(enable_hls)));
        param.insert(std::pair<std::string, std::string>("enable_mp4", std::to_string(enable_mp4)));
        param.insert(std::pair<std::string, std::string>("ffmpeg_cmd_key", ffmpeg_cmd_key));
        return sendPost(mediaServerItem, "addFFmpegSource", param);
    }

    std::string delFFmpegSource(MediaServerItem mediaServerItem, std::string key) {
        std::map<std::string, std::string> param;
        param.insert(std::pair<std::string, std::string>("key", key));
        return sendPost(mediaServerItem, "delFFmpegSource", param);
    }

    std::string getMediaServerConfig(MediaServerItem mediaServerItem) {
        std::map<std::string, std::string> param;
        return sendPost(mediaServerItem, "getServerConfig", param);
    }

    std::string setServerConfig(MediaServerItem mediaServerItem, std::map<std::string, std::string> param) {
        return sendPost(mediaServerItem, "setServerConfig", param);
    }

    std::string openRtpServer(MediaServerItem mediaServerItem, std::map<std::string, std::string> param) {
        return sendPost(mediaServerItem, "openRtpServer", param);
    }

    std::string closeRtpServer(MediaServerItem mediaServerItem, std::map<std::string, std::string> param) {
        return sendPost(mediaServerItem, "closeRtpServer", param);
    }

    std::string listRtpServer(MediaServerItem mediaServerItem) {
        std::map<std::string, std::string> param;
        return sendPost(mediaServerItem, "listRtpServer", param);
    }

    std::string startSendRtp(MediaServerItem mediaServerItem, std::map<std::string, std::string> param) {
        return sendPost(mediaServerItem, "startSendRtp", param);
    }

    std::string stopSendRtp(MediaServerItem mediaServerItem, std::map<std::string, std::string> param) {
        return sendPost(mediaServerItem, "stopSendRtp", param);
    }

    std::string restartServer(MediaServerItem mediaServerItem) {
        std::map<std::string, std::string> param;
        return sendPost(mediaServerItem, "restartServer", param);
    }

    std::string addStreamProxy(MediaServerItem mediaServerItem, std::string app, std::string stream, std::string url, bool enable_hls, bool enable_mp4, std::string rtp_type) {
        std::map<std::string, std::string> param;
        param.insert(std::pair<std::string, std::string>("vhost", "__defaultVhost__"));
        param.insert(std::pair<std::string, std::string>("app", app));
        param.insert(std::pair<std::string, std::string>("stream", stream));
        param.insert(std::pair<std::string, std::string>("url", url));
        param.insert(std::pair<std::string, std::string>("enable_hls", std::to_string(enable_hls ? 1 : 0)));
        param.insert(std::pair<std::string, std::string>("enable_mp4", std::to_string(enable_mp4 ? 1 : 0)));
        param.insert(std::pair<std::string, std::string>("enable_rtmp", std::to_string(1)));
        param.insert(std::pair<std::string, std::string>("enable_fmp4", std::to_string(1)));
        param.insert(std::pair<std::string, std::string>("enable_audio", std::to_string(1)));
        param.insert(std::pair<std::string, std::string>("enable_rtsp", std::to_string(1)));
        param.insert(std::pair<std::string, std::string>("add_mute_audio", std::to_string(1)));
        param.insert(std::pair<std::string, std::string>("rtp_type", rtp_type));
        return sendPost(mediaServerItem, "addStreamProxy", param);
    }

    std::string closeStreams(MediaServerItem mediaServerItem, std::string app, std::string stream) {
        std::map<std::string, std::string> param;
        param.insert(std::pair<std::string, std::string>("vhost", "__defaultVhost__"));
        param.insert(std::pair<std::string, std::string>("app", app));
        param.insert(std::pair<std::string, std::string>("stream", stream));
        param.insert(std::pair<std::string, std::string>("force", std::to_string(1)));
        return sendPost(mediaServerItem, "close_streams", param);
    }

    std::string getAllSession(MediaServerItem mediaServerItem) {
        std::map<std::string, std::string> param;
        return sendPost(mediaServerItem, "getAllSession", param);
    }

    void kickSessions(MediaServerItem mediaServerItem, std::string localPortSStr) {
        std::map<std::string, std::string> param;
        param.insert(std::pair<std::string, std::string>("local_port", localPortSStr));
        sendPost(mediaServerItem, "kick_sessions", param);
    }

    void getSnap(MediaServerItem mediaServerItem, std::string flvUrl, int timeout_sec, int expire_sec, std::string targetPath, std::string fileName) {
        std::map<std::string, std::string> param;
        param.insert(std::pair<std::string, std::string>("url", flvUrl));
        param.insert(std::pair<std::string, std::string>("timeout_sec", std::to_string(timeout_sec)));
        param.insert(std::pair<std::string, std::string>("expire_sec", std::to_string(expire_sec)));
        sendGetForImg(mediaServerItem, "getSnap", param, targetPath, fileName);
    }
};
#endif