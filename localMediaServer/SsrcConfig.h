#ifndef SSRC_CONFIG_H_
#define SSRC_CONFIG_H_
#include <iostream>
#include <list>
#include <set>
#include <string>
#define MAX_STRTEAM_COUNT 256

class SsrcConfig
{

    /**
     * zlm流媒体服务器Id
     */
    std::string mediaServerId;

    std::string ssrcPrefix;
    /**
     * zlm流媒体服务器已用会话句柄
     */
    std::set<std::string> isUsed;
    /**
     * zlm流媒体服务器可用会话句柄
     */
    std::set<std::string> notUsed;
public:
    SsrcConfig()
    {
    }

    SsrcConfig(std::string mediaServerId, std::set<std::string> usedSet, std::string sipDomain)
    {
        mediaServerId = mediaServerId;
        ssrcPrefix = sipDomain.substr(3, 8);
        for (int i = 1; i < MAX_STRTEAM_COUNT; i++) {
            std::string ssrc;
            if (i < 10) {
                ssrc = "000" + i;
            }
            else if (i < 100) {
                ssrc = "00" + i;
            }
            else if (i < 1000) {
                ssrc = "0" + i;
            }
            else {
                ssrc = std::to_string(i);
            }
            if (usedSet.empty() || !usedSet.count(ssrc)) {
                notUsed.insert(ssrc);
            }
            else {
                isUsed.insert(ssrc);
            }
            usedSet.find(ssrc);
        }
    }


    /**
     * 获取视频预览的SSRC值,第一位固定为0
     * @return ssrc
     */
    std::string getPlaySsrc() {
        return "0" + getSsrcPrefix() + getSN();
    }

    /**
     * 获取录像回放的SSRC值,第一位固定为1
     *
     */
    std::string getPlayBackSsrc() {
        return "1" + getSsrcPrefix() + getSN();
    }

    /**
     * 释放ssrc，主要用完的ssrc一定要释放，否则会耗尽
     * @param ssrc 需要重置的ssrc
     */
    void releaseSsrc(std::string ssrc) {
        if (ssrc.empty()) {
            return;
        }
        std::string sn = ssrc.substr(6);
        try {
            isUsed.erase(sn);
            notUsed.insert(sn);
        }
        catch (...) {
        }
    }

    /**
     * 获取后四位数SN,随机数
     *
     */
    std::string getSN() {
        std::string sn;
        int index = 0;
        if (notUsed.size() == 0) {
            throw "ssrc已经用完";
        }
        else if (notUsed.size() == 1) {
            sn = *notUsed.find(std::to_string(0));
        }
        else {
            std::srand(notUsed.size() - 1);
            index = std::rand();
            sn = *notUsed.find(std::to_string(index));
        }
        notUsed.erase(std::to_string(index));
        isUsed.insert(sn);
        return sn;
    }

    std::string getSsrcPrefix() {
        return ssrcPrefix;
    }

    std::string getMediaServerId() {
        return mediaServerId;
    }

    void setMediaServerId(std::string strMediaServerId) {
        mediaServerId = strMediaServerId;
    }

    void setSsrcPrefix(std::string strSsrcPrefix) {
        ssrcPrefix = strSsrcPrefix;
    }

    std::set<std::string> getIsUsed() {
        return isUsed;
    }

    void setIsUsed(std::set<std::string> vIsUsed) {
        isUsed = vIsUsed;
    }

    std::set<std::string> getNotUsed() {
        return notUsed;
    }

    void setNotUsed(std::set<std::string> vNotUsed) {
        notUsed = vNotUsed;
    }

    bool checkSsrc(std::string ssrcInResponse) {
        return !isUsed.count(ssrcInResponse);
    }
};
#endif;