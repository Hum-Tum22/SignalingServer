#pragma once

#include <iostream>

#include "VideoStreamSessionManager.h"

class VideoStreamSessionManager;
class SsrcTransaction
{

    std::string deviceId;
    std::string channelId;
    std::string callId;
    std::string stream;
    std::string transaction;
    std::string dialog;
    std::string mediaServerId;
    std::string ssrc;
    int sessionType;
public:
    std::string getDeviceId() {
        return deviceId;
    }

    void setDeviceId(std::string strDeviceId) {
        deviceId = strDeviceId;
    }

    std::string getChannelId() {
        return channelId;
    }

    void setChannelId(std::string strChannelId) {
        channelId = strChannelId;
    }

    std::string getCallId() {
        return callId;
    }

    void setCallId(std::string strCallId) {
        callId = strCallId;
    }

    std::string getStream() {
        return stream;
    }

    void setStream(std::string strStream) {
        stream = strStream;
    }

    std::string getTransaction() {
        return transaction;
    }

    void setTransaction(std::string strTransaction) {
        transaction = strTransaction;
    }

    std::string getDialog() {
        return dialog;
    }

    void setDialog(std::string strDialog) {
        dialog = strDialog;
    }

    std::string getMediaServerId() {
        return mediaServerId;
    }

    void setMediaServerId(std::string strMediaServerId) {
        mediaServerId = strMediaServerId;
    }

    std::string getSsrc() {
        return ssrc;
    }

    void setSsrc(std::string strSsrc) {
        ssrc = strSsrc;
    }

    int getType() {
        return sessionType;
    }

    void setType(int eType) {
        sessionType = eType;
    }
};