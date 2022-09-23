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
     * zlm��ý�������Id
     */
    std::string mediaServerId;

    std::string ssrcPrefix;
    /**
     * zlm��ý����������ûỰ���
     */
    std::set<std::string> isUsed;
    /**
     * zlm��ý����������ûỰ���
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
     * ��ȡ��ƵԤ����SSRCֵ,��һλ�̶�Ϊ0
     * @return ssrc
     */
    std::string getPlaySsrc() {
        return "0" + getSsrcPrefix() + getSN();
    }

    /**
     * ��ȡ¼��طŵ�SSRCֵ,��һλ�̶�Ϊ1
     *
     */
    std::string getPlayBackSsrc() {
        return "1" + getSsrcPrefix() + getSN();
    }

    /**
     * �ͷ�ssrc����Ҫ�����ssrcһ��Ҫ�ͷţ������ľ�
     * @param ssrc ��Ҫ���õ�ssrc
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
     * ��ȡ����λ��SN,�����
     *
     */
    std::string getSN() {
        std::string sn;
        int index = 0;
        if (notUsed.size() == 0) {
            throw "ssrc�Ѿ�����";
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