#ifndef USER_SETTING_H_
#define USER_SETTING_H_
#include <iostream>
#include <vector>

class UserSetting
{
    bool savePositionHistory;
    bool autoApplyPlay;
    bool seniorSdp;
    int playTimeout;
    int platformPlayTimeout;
    bool interfaceAuthentication;
    bool recordPushLive;
    bool recordSip;
    bool logInDatebase;
    bool redisConfig;
    std::string serverId;
    std::string thirdPartyGBIdReg;
    std::vector<std::string> interfaceAuthenticationExcludes;
public:
    UserSetting()
    {
        savePositionHistory = false;
        autoApplyPlay = false;
        seniorSdp = false;
        playTimeout = 18000;
        platformPlayTimeout = 60000;
        interfaceAuthentication = true;
        recordPushLive = true;
        recordSip = true;
        logInDatebase = true;
        redisConfig = true;
        serverId = "000000";
        thirdPartyGBIdReg = "[\\s\\S]*";
    }
    bool getSavePositionHistory() {
        return savePositionHistory;
    }

    bool isSavePositionHistory() {
        return savePositionHistory;
    }

    bool isAutoApplyPlay() {
        return autoApplyPlay;
    }

    bool isSeniorSdp() {
        return seniorSdp;
    }

    int getPlayTimeout() {
        return playTimeout;
    }

    bool isInterfaceAuthentication() {
        return interfaceAuthentication;
    }

    bool isRecordPushLive() {
        return recordPushLive;
    }

    std::vector<std::string> getInterfaceAuthenticationExcludes() {
        return interfaceAuthenticationExcludes;
    }

    void setSavePositionHistory(bool bSavePositionHistory) {
        savePositionHistory = bSavePositionHistory;
    }

    void setAutoApplyPlay(bool bAutoApplyPlay) {
        autoApplyPlay = bAutoApplyPlay;
    }

    void setSeniorSdp(bool bSeniorSdp) {
        seniorSdp = bSeniorSdp;
    }

    void setPlayTimeout(int iPlayTimeout) {
        playTimeout = iPlayTimeout;
    }

    void setInterfaceAuthentication(bool bInterfaceAuthentication) {
        interfaceAuthentication = bInterfaceAuthentication;
    }

    void setRecordPushLive(bool bRecordPushLive) {
        recordPushLive = bRecordPushLive;
    }

    void setInterfaceAuthenticationExcludes(std::vector<std::string> vInterfaceAuthenticationExcludes) {
        interfaceAuthenticationExcludes = vInterfaceAuthenticationExcludes;
    }

    bool getLogInDatebase() {
        return logInDatebase;
    }

    void setLogInDatebase(bool bLogInDatebase) {
        logInDatebase = bLogInDatebase;
    }

    std::string getServerId() {
        return serverId;
    }

    void setServerId(std::string strServerId) {
        serverId = strServerId;
    }

    std::string getThirdPartyGBIdReg() {
        return thirdPartyGBIdReg;
    }

    void setThirdPartyGBIdReg(std::string strThirdPartyGBIdReg) {
        thirdPartyGBIdReg = strThirdPartyGBIdReg;
    }

    bool getRedisConfig() {
        return redisConfig;
    }

    void setRedisConfig(bool bRedisConfig) {
        redisConfig = bRedisConfig;
    }

    bool getRecordSip() {
        return recordSip;
    }

    void setRecordSip(bool bRecordSip) {
        recordSip = bRecordSip;
    }

    int getPlatformPlayTimeout() {
        return platformPlayTimeout;
    }

    void setPlatformPlayTimeout(int iPlatformPlayTimeout) {
        platformPlayTimeout = iPlatformPlayTimeout;
    }
};
#endif