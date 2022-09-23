#pragma once

#include <iostream>
#include <map>

#include "../conf/UserSetting.h"
#include "SsrcTransaction.h"


class VideoStreamSessionManager {

	//@Autowired
    //RedisUtil redisUtil;
	std::map<std::string, SsrcTransaction> mSsrcTransactionMap;

	//@Autowired
    UserSetting userSetting;
public:
	enum SessionType {
		play,
		playback,
		download
	};

	/**
	 * 添加一个点播/回放的事务信息
	 * 后续可以通过流Id/callID
	 * @param deviceId 设备ID
	 * @param channelId 通道ID
	 * @param callId 一次请求的CallID
	 * @param stream 流名称
	 * @param mediaServerId 所使用的流媒体ID
	 * @param transaction 事务
	 */
 //   void put(std::string deviceId, std::string channelId, std::string callId, std::string stream, std::string ssrc, std::string mediaServerId, ClientTransaction transaction, SessionType type) {
	//	SsrcTransaction ssrcTransaction;
	//	ssrcTransaction.setDeviceId(deviceId);
	//	ssrcTransaction.setChannelId(channelId);
	//	ssrcTransaction.setStream(stream);
	//	/*byte[] transactionByteArray = SerializeUtils.serialize(transaction);
	//	ssrcTransaction.setTransaction(transactionByteArray);*/
	//	ssrcTransaction.setCallId(callId);
	//	ssrcTransaction.setSsrc(ssrc);
	//	ssrcTransaction.setMediaServerId(mediaServerId);
	//	ssrcTransaction.setType(type);

	//	/*redisUtil.set(VideoManagerConstants.MEDIA_TRANSACTION_USED_PREFIX + userSetting.getServerId()
	//		+ "_" + deviceId + "_" + channelId + "_" + callId + "_" + stream, ssrcTransaction);
	//	redisUtil.set(VideoManagerConstants.MEDIA_TRANSACTION_USED_PREFIX + userSetting.getServerId()
	//		+ "_" + deviceId + "_" + channelId + "_" + callId + "_" + stream, ssrcTransaction);*/
	//}

 //   void put(std::string deviceId, std::string channelId, std::string callId, Dialog dialog) {
	//	SsrcTransaction *ssrcTransaction = getSsrcTransaction(deviceId, channelId, callId, "");
	//	if (ssrcTransaction != NULL) {
	//		//byte[] dialogByteArray = SerializeUtils.serialize(dialog);
	//		//ssrcTransaction.setDialog(dialogByteArray);
	//	}
	//	mSsrcTransactionMap.insert(std::pair<std::string, SsrcTransaction>(userSetting.getServerId()
	//		+ "_" + deviceId + "_" + channelId + "_" + ssrcTransaction->getCallId() + "_"
	//		+ ssrcTransaction->getStream(), *ssrcTransaction));
	//}


 //   ClientTransaction getTransaction(std::string deviceId, std::string channelId, std::string stream, std::string callId) {
	//	SsrcTransaction *ssrcTransaction = getSsrcTransaction(deviceId, channelId, callId, stream);
	//	if (ssrcTransaction == NULL) {
	//		return null;
	//	}
	//	byte[] transactionByteArray = ssrcTransaction.getTransaction();
	//	ClientTransaction clientTransaction = (ClientTransaction)SerializeUtils.deSerialize(transactionByteArray);
	//	return clientTransaction;
	//}

 //   SIPDialog getDialogByStream(std::string deviceId, std::string channelId, std::string stream) {
	//	SsrcTransaction *ssrcTransaction = getSsrcTransaction(deviceId, channelId, "", stream);
	//	if (ssrcTransaction == NULL)
	//	{
	//		return null;
	//	}
	//	byte[] dialogByteArray = ssrcTransaction->getDialog();
	//	if (dialogByteArray == null) {
	//		return null;
	//	}
	//	SIPDialog dialog = (SIPDialog)SerializeUtils.deSerialize(dialogByteArray);
	//	return dialog;
	//}

 //   SIPDialog getDialogByCallId(std::string deviceId, std::string channelId, std::string callId) {
	//	SsrcTransaction ssrcTransaction = getSsrcTransaction(deviceId, channelId, callId, null);
	//	if (ssrcTransaction == null) {
	//		return null;
	//	}
	//	byte[] dialogByteArray = ssrcTransaction.getDialog();
	//	if (dialogByteArray == null) {
	//		return null;
	//	}
	//	return (SIPDialog)SerializeUtils.deSerialize(dialogByteArray);
	//}

    SsrcTransaction* getSsrcTransaction(std::string deviceId, std::string channelId, std::string callId, std::string stream) {

		if (deviceId.empty()) {
			deviceId = "*";
		}
		if (channelId.empty()) {
			channelId = "*";
		}
		if (callId.empty()) {
			callId = "*";
		}
		if (stream.empty()) {
			stream = "*";
		}
		//VideoManagerConstants.MEDIA_TRANSACTION_USED_PREFIX=VMP_MEDIA_TRANSACTION_
		std::string key = userSetting.getServerId() + "_" + deviceId + "_" + channelId + "_" + callId + "_" + stream;
		auto scanResult = mSsrcTransactionMap.find(key);
		if (scanResult == mSsrcTransactionMap.end()) {
			return NULL;
		}
		return &scanResult->second;
	}

    std::list<SsrcTransaction> getSsrcTransactionForAll(std::string deviceId, std::string channelId, std::string callId, std::string stream) {
		if (deviceId.empty()) {
			deviceId = "*";
		}
		if (channelId.empty()) {
			channelId = "*";
		}
		if (callId.empty()) {
			callId = "*";
		}
		if (stream.empty()) {
			stream = "*";
		}
		std::list<SsrcTransaction> result;
		std::string key = userSetting.getServerId() + "_" + deviceId + "_" + channelId + "_" + callId + "_" + stream;
		auto scanResult = mSsrcTransactionMap.find(key);
		if (scanResult != mSsrcTransactionMap.end())
		{
			for (auto& keyObj : mSsrcTransactionMap)
			{
				result.push_back(keyObj.second);
				//result.add((SsrcTransaction)redisUtil.get((String)keyObj));
			}
		}
		return result;
	}

    std::string getMediaServerId(std::string deviceId, std::string channelId, std::string stream) {
		SsrcTransaction *ssrcTransaction = getSsrcTransaction(deviceId, channelId, "", stream);
		if (ssrcTransaction == NULL) {
			return NULL;
		}
		return ssrcTransaction->getMediaServerId();
	}

    std::string getSSRC(std::string deviceId, std::string channelId, std::string stream) {
		SsrcTransaction *ssrcTransaction = getSsrcTransaction(deviceId, channelId, "", stream);
		if (ssrcTransaction == NULL) {
			return NULL;
		}
		return ssrcTransaction->getSsrc();
	}

    void remove(std::string deviceId, std::string channelId, std::string stream) {
		SsrcTransaction *ssrcTransaction = getSsrcTransaction(deviceId, channelId, "", stream);
		if (ssrcTransaction == NULL) {
			return;
		}
		mSsrcTransactionMap.erase(userSetting.getServerId() + "_"
			+ deviceId + "_" + channelId + "_" + ssrcTransaction->getCallId() + "_" + ssrcTransaction->getStream());
	}


    /*List<SsrcTransaction> getAllSsrc() {
		List<Object> ssrcTransactionKeys = redisUtil.scan(String.format("%s_*_*_*_*", userSetting.getServerId()));
		List<SsrcTransaction> result = new ArrayList<>();
		for (int i = 0; i < ssrcTransactionKeys.size(); i++) {
			std::string key = (String)ssrcTransactionKeys.get(i);
			SsrcTransaction ssrcTransaction = (SsrcTransaction)redisUtil.get(key);
			result.add(ssrcTransaction);
		}
		return result;
	}*/
};