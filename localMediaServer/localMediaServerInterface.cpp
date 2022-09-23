#include "localMediaServerInterface.h"
#include "../device/DeviceManager.h"
#include "../http.h"

int StartOneLiveMediaStream(const string& channelId, const string& fromId, const string &callId, const string &ssrc)
{
	//�ж������ƽ̨
	//ParentPlatform platform = storager.queryParentPlatByServerGBId(fromId);
	//if (platform == NULL)
	//{
	//	//inviteFromDeviceHandle(evt, requesterId);
	//	return -1;
	//}
	//else
	//{
	//	//��ƽ̨���д�ͨ��
	//	DeviceChannel channel = storager.queryChannelInParentPlatform(fromId, channelId);
	//	GbStream gbStream = storager.queryStreamInParentPlatform(fromId, channelId);
	//	PlatformCatalog catalog = storager.getCatalog(channelId);
	//	MediaServerItem mediaServerItem = null;
	//	// ����ͨ��������ֱ����
	//	if (channel != null && gbStream == null) {
	//		if (channel.getStatus() == 0) {
	//			//responseAck(evt, Response.BAD_REQUEST, "channel [" + channel.getChannelId() + "] offline");
	//			return -2;
	//		}
	//		//responseAck(evt, Response.CALL_IS_BEING_FORWARDED); // ͨ�����ڣ���181������ת����
	//	}
	//	else if (channel == null && gbStream != null) {
	//		string mediaServerId = gbStream.getMediaServerId();
	//		mediaServerItem = mediaServerService.getOne(mediaServerId);
	//		if (mediaServerItem == null) {
	//			//logger.info("[ app={}, stream={} ]�Ҳ���zlm {}������410", gbStream.getApp(), gbStream.getStream(), mediaServerId);
	//			//responseAck(evt, Response.GONE);
	//			return -3;
	//		}
	//		//responseAck(evt, Response.CALL_IS_BEING_FORWARDED); // ͨ�����ڣ���181������ת����
	//	}
	//	else if (catalog != null) {
	//		//responseAck(evt, Response.BAD_REQUEST, "catalog channel can not play"); // Ŀ¼��֧�ֵ㲥
	//		return -4;
	//	}
	//	else {
	//		//logger.info("ͨ�������ڣ�����404");
	//		//responseAck(evt, Response.NOT_FOUND); // ͨ�������ڣ���404����Դ������
	//		return -5;
	//	}
	//	// ����sdp��Ϣ, ʹ��jainsip �Դ���sdp������ʽ
	//	String contentString = new String(request.getRawContent());

	//	// jainSip��֧��y=�ֶΣ� �Ƴ��Խ�����
	//	int ssrcIndex = contentString.indexOf("y=");
	//	// ����Ƿ���y�ֶ�
	//	String ssrcDefault = "0000000000";
	//	String ssrc;
	//	SessionDescription sdp;
	//	if (ssrcIndex >= 0) {
	//		//ssrc�涨����Ϊ10�ֽڣ���ȡ���³����Ա���������С�f=���ֶ�
	//		ssrc = contentString.substring(ssrcIndex + 2, ssrcIndex + 12);
	//		String substring = contentString.substring(0, contentString.indexOf("y="));
	//		sdp = SdpFactory.getInstance().createSessionDescription(substring);
	//	}
	//	else {
	//		ssrc = ssrcDefault;
	//		sdp = SdpFactory.getInstance().createSessionDescription(contentString);
	//	}
	//	String sessionName = sdp.getSessionName().getValue();

	//	Long startTime = null;
	//	Long stopTime = null;
	//	Instant start = null;
	//	Instant end = null;
	//	if (sdp.getTimeDescriptions(false) != null && sdp.getTimeDescriptions(false).size() > 0) {
	//		TimeDescriptionImpl timeDescription = (TimeDescriptionImpl)(sdp.getTimeDescriptions(false).get(0));
	//		TimeField startTimeFiled = (TimeField)timeDescription.getTime();
	//		startTime = startTimeFiled.getStartTime();
	//		stopTime = startTimeFiled.getStopTime();

	//		start = Instant.ofEpochSecond(startTime);
	//		end = Instant.ofEpochSecond(stopTime);
	//	}
	//	//  ��ȡ֧�ֵĸ�ʽ
	//	Vector mediaDescriptions = sdp.getMediaDescriptions(true);
	//	// �鿴�Ƿ�֧��PS ����96
	//	//String ip = null;
	//	int port = -1;
	//	boolean mediaTransmissionTCP = false;
	//	Boolean tcpActive = null;
	//	for (Object description : mediaDescriptions) {
	//		MediaDescription mediaDescription = (MediaDescription)description;
	//		Media media = mediaDescription.getMedia();

	//		Vector mediaFormats = media.getMediaFormats(false);
	//		if (mediaFormats.contains("96")) {
	//			port = media.getMediaPort();
	//			//String mediaType = media.getMediaType();
	//			String protocol = media.getProtocol();

	//			// ����TCP��������udp�� ��ǰĬ��udp
	//			if ("TCP/RTP/AVP".equals(protocol)) {
	//				String setup = mediaDescription.getAttribute("setup");
	//				if (setup != null) {
	//					mediaTransmissionTCP = true;
	//					if ("active".equals(setup)) {
	//						tcpActive = true;
	//						// ��֧��tcp����
	//						responseAck(evt, Response.NOT_IMPLEMENTED, "tcp active not support"); // Ŀ¼��֧�ֵ㲥
	//						return;
	//					}
	//					else if ("passive".equals(setup)) {
	//						tcpActive = false;
	//					}
	//				}
	//			}
	//			break;
	//		}
	//	}
	//	if (port == -1) {
	//		logger.info("��֧�ֵ�ý���ʽ������415");
	//		// �ظ���֧�ֵĸ�ʽ
	//		responseAck(evt, Response.UNSUPPORTED_MEDIA_TYPE); // ��֧�ֵĸ�ʽ����415
	//		return;
	//	}
	//	String username = sdp.getOrigin().getUsername();
	//	String addressStr = sdp.getOrigin().getAddress();

	//	logger.info("[�ϼ��㲥]�û���{}�� ͨ����{}, ��ַ��{}:{}�� ssrc��{}", username, channelId, addressStr, port, ssrc);
	//	Device device = null;
	//	// ͨ�� channel �� gbStream �Ƿ�Ϊnull ֵ�ж���Դ��ֱ�������ʹ���
	//	if (channel != null) {
	//		device = storager.queryVideoDeviceByPlatformIdAndChannelId(requesterId, channelId);
	//		if (device == null) {
	//			logger.warn("�㲥ƽ̨{}��ͨ��{}ʱδ�ҵ��豸��Ϣ", requesterId, channel);
	//			responseAck(evt, Response.SERVER_INTERNAL_ERROR);
	//			return;
	//		}
	//		mediaServerItem = playService.getNewMediaServerItem(device);
	//		if (mediaServerItem == null) {
	//			logger.warn("δ�ҵ����õ�zlm");
	//			responseAck(evt, Response.BUSY_HERE);
	//			return;
	//		}
	//		SendRtpItem sendRtpItem = zlmrtpServerFactory.createSendRtpItem(mediaServerItem, addressStr, port, ssrc, requesterId,
	//			device.getDeviceId(), channelId,
	//			mediaTransmissionTCP);
	//		if (tcpActive != null) {
	//			sendRtpItem.setTcpActive(tcpActive);
	//		}
	//		if (sendRtpItem == null) {
	//			logger.warn("�������˿���Դ����");
	//			responseAck(evt, Response.BUSY_HERE);
	//			return;
	//		}
	//		sendRtpItem.setCallId(callIdHeader.getCallId());
	//		sendRtpItem.setPlayType("Play".equals(sessionName) ? InviteStreamType.PLAY : InviteStreamType.PLAYBACK);

	//		Long finalStartTime = startTime;
	//		Long finalStopTime = stopTime;
	//		ZLMHttpHookSubscribe.Event hookEvent = (mediaServerItemInUSe, responseJSON)->{
	//			String app = responseJSON.getString("app");
	//			String stream = responseJSON.getString("stream");
	//			logger.info("[�ϼ��㲥]�¼��Ѿ���ʼ������ �ظ�200OK(SDP)�� {}/{}", app, stream);
	//			//     * 0 �ȴ��豸��������
	//			//     * 1 �¼��Ѿ��������ȴ��ϼ�ƽ̨�ظ�ack
	//			//     * 2 ������
	//			sendRtpItem.setStatus(1);
	//			redisCatchStorage.updateSendRTPSever(sendRtpItem);

	//			StringBuffer content = new StringBuffer(200);
	//			content.append("v=0\r\n");
	//			content.append("o=" + channelId + " 0 0 IN IP4 " + mediaServerItemInUSe.getSdpIp() + "\r\n");
	//			content.append("s=" + sessionName + "\r\n");
	//			content.append("c=IN IP4 " + mediaServerItemInUSe.getSdpIp() + "\r\n");
	//			if ("Playback".equals(sessionName)) {
	//				content.append("t=" + finalStartTime + " " + finalStopTime + "\r\n");
	//			}
	//			else {
	//				content.append("t=0 0\r\n");
	//			}
	//			content.append("m=video " + sendRtpItem.getLocalPort() + " RTP/AVP 96\r\n");
	//			content.append("a=sendonly\r\n");
	//			content.append("a=rtpmap:96 PS/90000\r\n");
	//			content.append("y=" + ssrc + "\r\n");
	//			content.append("f=\r\n");

	//			try {
	//				// ��ʱδ�յ�AckӦ�ûظ�bye,��ǰ�ȴ�ʱ��Ϊ10��
	//				dynamicTask.startDelay(callIdHeader.getCallId(), ()->{
	//					logger.info("Ack �ȴ���ʱ");
	//					mediaServerService.releaseSsrc(mediaServerItemInUSe.getId(), ssrc);
	//					// �ظ�bye
	//					cmderFroPlatform.streamByeCmd(platform, callIdHeader.getCallId());
	//				}, 60 * 1000);
	//				responseSdpAck(evt, content.toString(), platform);

	//			}
	//			catch (SipException e) {
	//				e.printStackTrace();
	//			}
	//			catch (InvalidArgumentException e) {
	//				e.printStackTrace();
	//			}
	//			catch (ParseException e) {
	//				e.printStackTrace();
	//			}
	//		};
	//		SipSubscribe.Event errorEvent = ((event) -> {
	//			// δ֪����ֱ��ת���豸�㲥�Ĵ���
	//			Response response = null;
	//			try {
	//				response = getMessageFactory().createResponse(event.statusCode, evt.getRequest());
	//				ServerTransaction serverTransaction = getServerTransaction(evt);
	//				serverTransaction.sendResponse(response);
	//				if (serverTransaction.getDialog() != null) {
	//					serverTransaction.getDialog().delete();
	//				}
	//			}
	//			catch (ParseException | SipException | InvalidArgumentException e) {
	//				e.printStackTrace();
	//			}
	//		});
	//		sendRtpItem.setApp("rtp");
	//		if ("Playback".equals(sessionName)) {
	//			sendRtpItem.setPlayType(InviteStreamType.PLAYBACK);
	//			SSRCInfo ssrcInfo = mediaServerService.openRTPServer(mediaServerItem, null, true, true);
	//			sendRtpItem.setStreamId(ssrcInfo.getStream());
	//			// д��redis�� ��ʱʱ�ظ�
	//			redisCatchStorage.updateSendRTPSever(sendRtpItem);
	//			playService.playBack(mediaServerItem, ssrcInfo, device.getDeviceId(), channelId, DateUtil.formatter.format(start),
	//				DateUtil.formatter.format(end), null, result -> {
	//				if (result.getCode() != 0) {
	//					logger.warn("¼��ط�ʧ��");
	//					if (result.getEvent() != null) {
	//						errorEvent.response(result.getEvent());
	//					}
	//					redisCatchStorage.deleteSendRTPServer(platform.getServerGBId(), channelId, callIdHeader.getCallId(), null);
	//					try {
	//						responseAck(evt, Response.REQUEST_TIMEOUT);
	//					}
	//					catch (SipException e) {
	//						e.printStackTrace();
	//					}
	//					catch (InvalidArgumentException e) {
	//						e.printStackTrace();
	//					}
	//					catch (ParseException e) {
	//						e.printStackTrace();
	//					}
	//				}
	//				else {
	//					if (result.getMediaServerItem() != null) {
	//						hookEvent.response(result.getMediaServerItem(), result.getResponse());
	//					}
	//				}
	//			});
	//		}
	//		else {
	//			sendRtpItem.setPlayType(InviteStreamType.PLAY);
	//			SsrcTransaction playTransaction = sessionManager.getSsrcTransaction(device.getDeviceId(), channelId, "play", null);
	//			if (playTransaction != null) {
	//				Boolean streamReady = zlmrtpServerFactory.isStreamReady(mediaServerItem, "rtp", playTransaction.getStream());
	//				if (!streamReady) {
	//					playTransaction = null;
	//				}
	//			}
	//			if (playTransaction == null) {
	//				String streamId = null;
	//				if (mediaServerItem.isRtpEnable()) {
	//					streamId = String.format("%s_%s", device.getDeviceId(), channelId);
	//				}
	//				SSRCInfo ssrcInfo = mediaServerService.openRTPServer(mediaServerItem, streamId, null, device.isSsrcCheck(), false);
	//				sendRtpItem.setStreamId(ssrcInfo.getStream());
	//				// д��redis�� ��ʱʱ�ظ�
	//				redisCatchStorage.updateSendRTPSever(sendRtpItem);
	//				playService.play(mediaServerItem, ssrcInfo, device, channelId, hookEvent, errorEvent, (code, msg)->{
	//					logger.info("[�ϼ��㲥]��ʱ, �û���{}�� ͨ����{}", username, channelId);
	//					redisCatchStorage.deleteSendRTPServer(platform.getServerGBId(), channelId, callIdHeader.getCallId(), null);
	//				}, null);
	//			}
	//			else {
	//				sendRtpItem.setStreamId(playTransaction.getStream());
	//				// д��redis�� ��ʱʱ�ظ�
	//				redisCatchStorage.updateSendRTPSever(sendRtpItem);
	//				JSONObject jsonObject = new JSONObject();
	//				jsonObject.put("app", sendRtpItem.getApp());
	//				jsonObject.put("stream", sendRtpItem.getStreamId());
	//				hookEvent.response(mediaServerItem, jsonObject);
	//			}
	//		}
	//	}
	//	else if (gbStream != null) {

	//		Boolean streamReady = zlmrtpServerFactory.isStreamReady(mediaServerItem, gbStream.getApp(), gbStream.getStream());
	//		if (!streamReady) {
	//			if ("proxy".equals(gbStream.getStreamType())) {
	//				// TODO ����������ʹ�豸����
	//				logger.info("[ app={}, stream={} ]ͨ�����ߣ���������ʼ����", gbStream.getApp(), gbStream.getStream());
	//				responseAck(evt, Response.BAD_REQUEST, "channel [" + gbStream.getGbId() + "] offline");
	//			}
	//			else if ("push".equals(gbStream.getStreamType())) {
	//				if (!platform.isStartOfflinePush()) {
	//					responseAck(evt, Response.TEMPORARILY_UNAVAILABLE, "channel unavailable");
	//					return;
	//				}
	//				// ����redis��Ϣ��ʹ�豸����
	//				logger.info("[ app={}, stream={} ]ͨ�����ߣ�����redis��Ϣ�����豸��ʼ����", gbStream.getApp(), gbStream.getStream());
	//				MessageForPushChannel messageForPushChannel = new MessageForPushChannel();
	//				messageForPushChannel.setType(1);
	//				messageForPushChannel.setGbId(gbStream.getGbId());
	//				messageForPushChannel.setApp(gbStream.getApp());
	//				messageForPushChannel.setStream(gbStream.getStream());
	//				// TODO ��ȡ�͸��صĽڵ�
	//				messageForPushChannel.setMediaServerId(gbStream.getMediaServerId());
	//				messageForPushChannel.setPlatFormId(platform.getServerGBId());
	//				messageForPushChannel.setPlatFormName(platform.getName());
	//				redisCatchStorage.sendStreamPushRequestedMsg(messageForPushChannel);
	//				// ���ó�ʱ
	//				dynamicTask.startDelay(callIdHeader.getCallId(), ()->{
	//					logger.info("[ app={}, stream={} ] �ȴ��豸��ʼ������ʱ", gbStream.getApp(), gbStream.getStream());
	//					try {
	//						mediaListManager.removedChannelOnlineEventLister(gbStream.getGbId());
	//						responseAck(evt, Response.REQUEST_TIMEOUT); // ��ʱ
	//					}
	//					catch (SipException e) {
	//						e.printStackTrace();
	//					}
	//					catch (InvalidArgumentException e) {
	//						e.printStackTrace();
	//					}
	//					catch (ParseException e) {
	//						e.printStackTrace();
	//					}
	//				}, userSetting.getPlatformPlayTimeout());
	//				// ��Ӽ���
	//				MediaServerItem finalMediaServerItem = mediaServerItem;
	//				int finalPort = port;
	//				boolean finalMediaTransmissionTCP = mediaTransmissionTCP;
	//				Boolean finalTcpActive = tcpActive;
	//				mediaListManager.addChannelOnlineEventLister(gbStream.getGbId(), (app, stream)->{
	//					SendRtpItem sendRtpItem = zlmrtpServerFactory.createSendRtpItem(finalMediaServerItem, addressStr, finalPort, ssrc, requesterId,
	//						app, stream, channelId, finalMediaTransmissionTCP);

	//					if (sendRtpItem == null) {
	//						logger.warn("�������˿���Դ����");
	//						try {
	//							responseAck(evt, Response.BUSY_HERE);
	//						}
	//						catch (SipException e) {
	//							e.printStackTrace();
	//						}
	//						catch (InvalidArgumentException e) {
	//							e.printStackTrace();
	//						}
	//						catch (ParseException e) {
	//							e.printStackTrace();
	//						}
	//						return;
	//					}
	//					if (finalTcpActive != null) {
	//						sendRtpItem.setTcpActive(finalTcpActive);
	//					}
	//					sendRtpItem.setPlayType(InviteStreamType.PUSH);
	//					// д��redis�� ��ʱʱ�ظ�
	//					sendRtpItem.setStatus(1);
	//					sendRtpItem.setCallId(callIdHeader.getCallId());
	//					byte[] dialogByteArray = SerializeUtils.serialize(evt.getDialog());
	//					sendRtpItem.setDialog(dialogByteArray);
	//					byte[] transactionByteArray = SerializeUtils.serialize(evt.getServerTransaction());
	//					sendRtpItem.setTransaction(transactionByteArray);
	//					redisCatchStorage.updateSendRTPSever(sendRtpItem);
	//					sendStreamAck(finalMediaServerItem, sendRtpItem, platform, evt);

	//				});
	//			}
	//		}
	//		else {
	//			SendRtpItem sendRtpItem = zlmrtpServerFactory.createSendRtpItem(mediaServerItem, addressStr, port, ssrc, requesterId,
	//				gbStream.getApp(), gbStream.getStream(), channelId,
	//				mediaTransmissionTCP);


	//			if (sendRtpItem == null) {
	//				logger.warn("�������˿���Դ����");
	//				responseAck(evt, Response.BUSY_HERE);
	//				return;
	//			}
	//			if (tcpActive != null) {
	//				sendRtpItem.setTcpActive(tcpActive);
	//			}
	//			sendRtpItem.setPlayType(InviteStreamType.PUSH);
	//			// д��redis�� ��ʱʱ�ظ�
	//			sendRtpItem.setStatus(1);
	//			sendRtpItem.setCallId(callIdHeader.getCallId());
	//			byte[] dialogByteArray = SerializeUtils.serialize(evt.getDialog());
	//			sendRtpItem.setDialog(dialogByteArray);
	//			byte[] transactionByteArray = SerializeUtils.serialize(evt.getServerTransaction());
	//			sendRtpItem.setTransaction(transactionByteArray);
	//			redisCatchStorage.updateSendRTPSever(sendRtpItem);
	//			sendStreamAck(mediaServerItem, sendRtpItem, platform, evt);
	//		}


	//	}
	//}
    IDeviceMngrSvr& devmng = GetIDeviceMngr();
    list<GBDeviceChannel> chlist = devmng.GetGBDeviceMapper().GetGBDeviceChannelMapper().queryChannelByChannelId(channelId);
    for (auto& it : chlist)
    {
        //
        string strUrl("http://192.168.1.232/index/api/addStreamProxy?secret=035c73f7-bb6b-4889-a715-d9eb2d1925cc&vhost=__defaultVhost__&app=live&stream=0&url=rtsp://192.168.1.231:554/live?channel=1&stream=0");
        string strReponse = GetRequest(strUrl);
    }
	return 0;
}
int StartOneVodMediaStream()
{
	return -1;
}

//void process(RequestEvent evt) {
//	//  Invite Request��Ϣʵ�֣�����Ϣһ��Ϊ������Ϣ���ϼ����¼�����������Ƶָ��
//	try {
//		Request request = evt.getRequest();
//		SipURI sipURI = (SipURI)request.getRequestURI();
//		//��subject��ȡchannelId,���ٴ�request-line��ȡ�� ��Щƽ̨request-line��ƽ̨������룬�����豸������롣
//		//String channelId = sipURI.getUser();
//
//
//		// ��ѯ�����Ƿ������ϼ�ƽ̨\�豸
//		ParentPlatform platform = storager.queryParentPlatByServerGBId(requesterId);
//		if (platform == null) {
//			inviteFromDeviceHandle(evt, requesterId);
//		}
//		else {
//			// ��ѯƽ̨���Ƿ��и�ͨ��
//			DeviceChannel channel = storager.queryChannelInParentPlatform(requesterId, channelId);
//			GbStream gbStream = storager.queryStreamInParentPlatform(requesterId, channelId);
//			PlatformCatalog catalog = storager.getCatalog(channelId);
//			MediaServerItem mediaServerItem = null;
//			// ����ͨ��������ֱ����
//			if (channel != null && gbStream == null) {
//				if (channel.getStatus() == 0) {
//					logger.info("ͨ�����ߣ�����400");
//					responseAck(evt, Response.BAD_REQUEST, "channel [" + channel.getChannelId() + "] offline");
//					return;
//				}
//				responseAck(evt, Response.CALL_IS_BEING_FORWARDED); // ͨ�����ڣ���181������ת����
//			}
//			else if (channel == null && gbStream != null) {
//				String mediaServerId = gbStream.getMediaServerId();
//				mediaServerItem = mediaServerService.getOne(mediaServerId);
//				if (mediaServerItem == null) {
//					logger.info("[ app={}, stream={} ]�Ҳ���zlm {}������410", gbStream.getApp(), gbStream.getStream(), mediaServerId);
//					responseAck(evt, Response.GONE);
//					return;
//				}
//				responseAck(evt, Response.CALL_IS_BEING_FORWARDED); // ͨ�����ڣ���181������ת����
//			}
//			else if (catalog != null) {
//				responseAck(evt, Response.BAD_REQUEST, "catalog channel can not play"); // Ŀ¼��֧�ֵ㲥
//				return;
//			}
//			else {
//				logger.info("ͨ�������ڣ�����404");
//				responseAck(evt, Response.NOT_FOUND); // ͨ�������ڣ���404����Դ������
//				return;
//			}
//			// ����sdp��Ϣ, ʹ��jainsip �Դ���sdp������ʽ
//			String contentString = new String(request.getRawContent());
//
//			// jainSip��֧��y=�ֶΣ� �Ƴ��Խ�����
//			int ssrcIndex = contentString.indexOf("y=");
//			// ����Ƿ���y�ֶ�
//			String ssrcDefault = "0000000000";
//			String ssrc;
//			SessionDescription sdp;
//			if (ssrcIndex >= 0) {
//				//ssrc�涨����Ϊ10�ֽڣ���ȡ���³����Ա���������С�f=���ֶ�
//				ssrc = contentString.substring(ssrcIndex + 2, ssrcIndex + 12);
//				String substring = contentString.substring(0, contentString.indexOf("y="));
//				sdp = SdpFactory.getInstance().createSessionDescription(substring);
//			}
//			else {
//				ssrc = ssrcDefault;
//				sdp = SdpFactory.getInstance().createSessionDescription(contentString);
//			}
//			String sessionName = sdp.getSessionName().getValue();
//
//			Long startTime = null;
//			Long stopTime = null;
//			Instant start = null;
//			Instant end = null;
//			if (sdp.getTimeDescriptions(false) != null && sdp.getTimeDescriptions(false).size() > 0) {
//				TimeDescriptionImpl timeDescription = (TimeDescriptionImpl)(sdp.getTimeDescriptions(false).get(0));
//				TimeField startTimeFiled = (TimeField)timeDescription.getTime();
//				startTime = startTimeFiled.getStartTime();
//				stopTime = startTimeFiled.getStopTime();
//
//				start = Instant.ofEpochSecond(startTime);
//				end = Instant.ofEpochSecond(stopTime);
//			}
//			//  ��ȡ֧�ֵĸ�ʽ
//			Vector mediaDescriptions = sdp.getMediaDescriptions(true);
//			// �鿴�Ƿ�֧��PS ����96
//			//String ip = null;
//			int port = -1;
//			boolean mediaTransmissionTCP = false;
//			Boolean tcpActive = null;
//			for (Object description : mediaDescriptions) {
//				MediaDescription mediaDescription = (MediaDescription)description;
//				Media media = mediaDescription.getMedia();
//
//				Vector mediaFormats = media.getMediaFormats(false);
//				if (mediaFormats.contains("96")) {
//					port = media.getMediaPort();
//					//String mediaType = media.getMediaType();
//					String protocol = media.getProtocol();
//
//					// ����TCP��������udp�� ��ǰĬ��udp
//					if ("TCP/RTP/AVP".equals(protocol)) {
//						String setup = mediaDescription.getAttribute("setup");
//						if (setup != null) {
//							mediaTransmissionTCP = true;
//							if ("active".equals(setup)) {
//								tcpActive = true;
//								// ��֧��tcp����
//								responseAck(evt, Response.NOT_IMPLEMENTED, "tcp active not support"); // Ŀ¼��֧�ֵ㲥
//								return;
//							}
//							else if ("passive".equals(setup)) {
//								tcpActive = false;
//							}
//						}
//					}
//					break;
//				}
//			}
//			if (port == -1) {
//				logger.info("��֧�ֵ�ý���ʽ������415");
//				// �ظ���֧�ֵĸ�ʽ
//				responseAck(evt, Response.UNSUPPORTED_MEDIA_TYPE); // ��֧�ֵĸ�ʽ����415
//				return;
//			}
//			String username = sdp.getOrigin().getUsername();
//			String addressStr = sdp.getOrigin().getAddress();
//
//			logger.info("[�ϼ��㲥]�û���{}�� ͨ����{}, ��ַ��{}:{}�� ssrc��{}", username, channelId, addressStr, port, ssrc);
//			Device device = null;
//			// ͨ�� channel �� gbStream �Ƿ�Ϊnull ֵ�ж���Դ��ֱ�������ʹ���
//			if (channel != null) {
//				device = storager.queryVideoDeviceByPlatformIdAndChannelId(requesterId, channelId);
//				if (device == null) {
//					logger.warn("�㲥ƽ̨{}��ͨ��{}ʱδ�ҵ��豸��Ϣ", requesterId, channel);
//					responseAck(evt, Response.SERVER_INTERNAL_ERROR);
//					return;
//				}
//				mediaServerItem = playService.getNewMediaServerItem(device);
//				if (mediaServerItem == null) {
//					logger.warn("δ�ҵ����õ�zlm");
//					responseAck(evt, Response.BUSY_HERE);
//					return;
//				}
//				SendRtpItem sendRtpItem = zlmrtpServerFactory.createSendRtpItem(mediaServerItem, addressStr, port, ssrc, requesterId,
//					device.getDeviceId(), channelId,
//					mediaTransmissionTCP);
//				if (tcpActive != null) {
//					sendRtpItem.setTcpActive(tcpActive);
//				}
//				if (sendRtpItem == null) {
//					logger.warn("�������˿���Դ����");
//					responseAck(evt, Response.BUSY_HERE);
//					return;
//				}
//				sendRtpItem.setCallId(callIdHeader.getCallId());
//				sendRtpItem.setPlayType("Play".equals(sessionName) ? InviteStreamType.PLAY : InviteStreamType.PLAYBACK);
//
//				Long finalStartTime = startTime;
//				Long finalStopTime = stopTime;
//				ZLMHttpHookSubscribe.Event hookEvent = (mediaServerItemInUSe, responseJSON)->{
//					String app = responseJSON.getString("app");
//					String stream = responseJSON.getString("stream");
//					logger.info("[�ϼ��㲥]�¼��Ѿ���ʼ������ �ظ�200OK(SDP)�� {}/{}", app, stream);
//					//     * 0 �ȴ��豸��������
//					//     * 1 �¼��Ѿ��������ȴ��ϼ�ƽ̨�ظ�ack
//					//     * 2 ������
//					sendRtpItem.setStatus(1);
//					redisCatchStorage.updateSendRTPSever(sendRtpItem);
//
//					StringBuffer content = new StringBuffer(200);
//					content.append("v=0\r\n");
//					content.append("o=" + channelId + " 0 0 IN IP4 " + mediaServerItemInUSe.getSdpIp() + "\r\n");
//					content.append("s=" + sessionName + "\r\n");
//					content.append("c=IN IP4 " + mediaServerItemInUSe.getSdpIp() + "\r\n");
//					if ("Playback".equals(sessionName)) {
//						content.append("t=" + finalStartTime + " " + finalStopTime + "\r\n");
//					}
//					else {
//						content.append("t=0 0\r\n");
//					}
//					content.append("m=video " + sendRtpItem.getLocalPort() + " RTP/AVP 96\r\n");
//					content.append("a=sendonly\r\n");
//					content.append("a=rtpmap:96 PS/90000\r\n");
//					content.append("y=" + ssrc + "\r\n");
//					content.append("f=\r\n");
//
//					try {
//						// ��ʱδ�յ�AckӦ�ûظ�bye,��ǰ�ȴ�ʱ��Ϊ10��
//						dynamicTask.startDelay(callIdHeader.getCallId(), ()->{
//							logger.info("Ack �ȴ���ʱ");
//							mediaServerService.releaseSsrc(mediaServerItemInUSe.getId(), ssrc);
//							// �ظ�bye
//							cmderFroPlatform.streamByeCmd(platform, callIdHeader.getCallId());
//						}, 60 * 1000);
//						responseSdpAck(evt, content.toString(), platform);
//
//					}
//					catch (SipException e) {
//						e.printStackTrace();
//					}
//					catch (InvalidArgumentException e) {
//						e.printStackTrace();
//					}
//					catch (ParseException e) {
//						e.printStackTrace();
//					}
//				};
//				SipSubscribe.Event errorEvent = ((event) -> {
//					// δ֪����ֱ��ת���豸�㲥�Ĵ���
//					Response response = null;
//					try {
//						response = getMessageFactory().createResponse(event.statusCode, evt.getRequest());
//						ServerTransaction serverTransaction = getServerTransaction(evt);
//						serverTransaction.sendResponse(response);
//						if (serverTransaction.getDialog() != null) {
//							serverTransaction.getDialog().delete();
//						}
//					}
//					catch (ParseException | SipException | InvalidArgumentException e) {
//						e.printStackTrace();
//					}
//				});
//				sendRtpItem.setApp("rtp");
//				if ("Playback".equals(sessionName)) {
//					sendRtpItem.setPlayType(InviteStreamType.PLAYBACK);
//					SSRCInfo ssrcInfo = mediaServerService.openRTPServer(mediaServerItem, null, true, true);
//					sendRtpItem.setStreamId(ssrcInfo.getStream());
//					// д��redis�� ��ʱʱ�ظ�
//					redisCatchStorage.updateSendRTPSever(sendRtpItem);
//					playService.playBack(mediaServerItem, ssrcInfo, device.getDeviceId(), channelId, DateUtil.formatter.format(start),
//						DateUtil.formatter.format(end), null, result -> {
//						if (result.getCode() != 0) {
//							logger.warn("¼��ط�ʧ��");
//							if (result.getEvent() != null) {
//								errorEvent.response(result.getEvent());
//							}
//							redisCatchStorage.deleteSendRTPServer(platform.getServerGBId(), channelId, callIdHeader.getCallId(), null);
//							try {
//								responseAck(evt, Response.REQUEST_TIMEOUT);
//							}
//							catch (SipException e) {
//								e.printStackTrace();
//							}
//							catch (InvalidArgumentException e) {
//								e.printStackTrace();
//							}
//							catch (ParseException e) {
//								e.printStackTrace();
//							}
//						}
//						else {
//							if (result.getMediaServerItem() != null) {
//								hookEvent.response(result.getMediaServerItem(), result.getResponse());
//							}
//						}
//					});
//				}
//				else {
//					sendRtpItem.setPlayType(InviteStreamType.PLAY);
//					SsrcTransaction playTransaction = sessionManager.getSsrcTransaction(device.getDeviceId(), channelId, "play", null);
//					if (playTransaction != null) {
//						Boolean streamReady = zlmrtpServerFactory.isStreamReady(mediaServerItem, "rtp", playTransaction.getStream());
//						if (!streamReady) {
//							playTransaction = null;
//						}
//					}
//					if (playTransaction == null) {
//						String streamId = null;
//						if (mediaServerItem.isRtpEnable()) {
//							streamId = String.format("%s_%s", device.getDeviceId(), channelId);
//						}
//						SSRCInfo ssrcInfo = mediaServerService.openRTPServer(mediaServerItem, streamId, null, device.isSsrcCheck(), false);
//						sendRtpItem.setStreamId(ssrcInfo.getStream());
//						// д��redis�� ��ʱʱ�ظ�
//						redisCatchStorage.updateSendRTPSever(sendRtpItem);
//						playService.play(mediaServerItem, ssrcInfo, device, channelId, hookEvent, errorEvent, (code, msg)->{
//							logger.info("[�ϼ��㲥]��ʱ, �û���{}�� ͨ����{}", username, channelId);
//							redisCatchStorage.deleteSendRTPServer(platform.getServerGBId(), channelId, callIdHeader.getCallId(), null);
//						}, null);
//					}
//					else {
//						sendRtpItem.setStreamId(playTransaction.getStream());
//						// д��redis�� ��ʱʱ�ظ�
//						redisCatchStorage.updateSendRTPSever(sendRtpItem);
//						JSONObject jsonObject = new JSONObject();
//						jsonObject.put("app", sendRtpItem.getApp());
//						jsonObject.put("stream", sendRtpItem.getStreamId());
//						hookEvent.response(mediaServerItem, jsonObject);
//					}
//				}
//			}
//			else if (gbStream != null) {
//
//				Boolean streamReady = zlmrtpServerFactory.isStreamReady(mediaServerItem, gbStream.getApp(), gbStream.getStream());
//				if (!streamReady) {
//					if ("proxy".equals(gbStream.getStreamType())) {
//						// TODO ����������ʹ�豸����
//						logger.info("[ app={}, stream={} ]ͨ�����ߣ���������ʼ����", gbStream.getApp(), gbStream.getStream());
//						responseAck(evt, Response.BAD_REQUEST, "channel [" + gbStream.getGbId() + "] offline");
//					}
//					else if ("push".equals(gbStream.getStreamType())) {
//						if (!platform.isStartOfflinePush()) {
//							responseAck(evt, Response.TEMPORARILY_UNAVAILABLE, "channel unavailable");
//							return;
//						}
//						// ����redis��Ϣ��ʹ�豸����
//						logger.info("[ app={}, stream={} ]ͨ�����ߣ�����redis��Ϣ�����豸��ʼ����", gbStream.getApp(), gbStream.getStream());
//						MessageForPushChannel messageForPushChannel = new MessageForPushChannel();
//						messageForPushChannel.setType(1);
//						messageForPushChannel.setGbId(gbStream.getGbId());
//						messageForPushChannel.setApp(gbStream.getApp());
//						messageForPushChannel.setStream(gbStream.getStream());
//						// TODO ��ȡ�͸��صĽڵ�
//						messageForPushChannel.setMediaServerId(gbStream.getMediaServerId());
//						messageForPushChannel.setPlatFormId(platform.getServerGBId());
//						messageForPushChannel.setPlatFormName(platform.getName());
//						redisCatchStorage.sendStreamPushRequestedMsg(messageForPushChannel);
//						// ���ó�ʱ
//						dynamicTask.startDelay(callIdHeader.getCallId(), ()->{
//							logger.info("[ app={}, stream={} ] �ȴ��豸��ʼ������ʱ", gbStream.getApp(), gbStream.getStream());
//							try {
//								mediaListManager.removedChannelOnlineEventLister(gbStream.getGbId());
//								responseAck(evt, Response.REQUEST_TIMEOUT); // ��ʱ
//							}
//							catch (SipException e) {
//								e.printStackTrace();
//							}
//							catch (InvalidArgumentException e) {
//								e.printStackTrace();
//							}
//							catch (ParseException e) {
//								e.printStackTrace();
//							}
//						}, userSetting.getPlatformPlayTimeout());
//						// ��Ӽ���
//						MediaServerItem finalMediaServerItem = mediaServerItem;
//						int finalPort = port;
//						boolean finalMediaTransmissionTCP = mediaTransmissionTCP;
//						Boolean finalTcpActive = tcpActive;
//						mediaListManager.addChannelOnlineEventLister(gbStream.getGbId(), (app, stream)->{
//							SendRtpItem sendRtpItem = zlmrtpServerFactory.createSendRtpItem(finalMediaServerItem, addressStr, finalPort, ssrc, requesterId,
//								app, stream, channelId, finalMediaTransmissionTCP);
//
//							if (sendRtpItem == null) {
//								logger.warn("�������˿���Դ����");
//								try {
//									responseAck(evt, Response.BUSY_HERE);
//								}
//								catch (SipException e) {
//									e.printStackTrace();
//								}
//								catch (InvalidArgumentException e) {
//									e.printStackTrace();
//								}
//								catch (ParseException e) {
//									e.printStackTrace();
//								}
//								return;
//							}
//							if (finalTcpActive != null) {
//								sendRtpItem.setTcpActive(finalTcpActive);
//							}
//							sendRtpItem.setPlayType(InviteStreamType.PUSH);
//							// д��redis�� ��ʱʱ�ظ�
//							sendRtpItem.setStatus(1);
//							sendRtpItem.setCallId(callIdHeader.getCallId());
//							byte[] dialogByteArray = SerializeUtils.serialize(evt.getDialog());
//							sendRtpItem.setDialog(dialogByteArray);
//							byte[] transactionByteArray = SerializeUtils.serialize(evt.getServerTransaction());
//							sendRtpItem.setTransaction(transactionByteArray);
//							redisCatchStorage.updateSendRTPSever(sendRtpItem);
//							sendStreamAck(finalMediaServerItem, sendRtpItem, platform, evt);
//
//						});
//					}
//				}
//				else {
//					SendRtpItem sendRtpItem = zlmrtpServerFactory.createSendRtpItem(mediaServerItem, addressStr, port, ssrc, requesterId,
//						gbStream.getApp(), gbStream.getStream(), channelId,
//						mediaTransmissionTCP);
//
//
//					if (sendRtpItem == null) {
//						logger.warn("�������˿���Դ����");
//						responseAck(evt, Response.BUSY_HERE);
//						return;
//					}
//					if (tcpActive != null) {
//						sendRtpItem.setTcpActive(tcpActive);
//					}
//					sendRtpItem.setPlayType(InviteStreamType.PUSH);
//					// д��redis�� ��ʱʱ�ظ�
//					sendRtpItem.setStatus(1);
//					sendRtpItem.setCallId(callIdHeader.getCallId());
//					byte[] dialogByteArray = SerializeUtils.serialize(evt.getDialog());
//					sendRtpItem.setDialog(dialogByteArray);
//					byte[] transactionByteArray = SerializeUtils.serialize(evt.getServerTransaction());
//					sendRtpItem.setTransaction(transactionByteArray);
//					redisCatchStorage.updateSendRTPSever(sendRtpItem);
//					sendStreamAck(mediaServerItem, sendRtpItem, platform, evt);
//				}
//
//
//			}
//
//		}
//
//	}
//	catch (SipException | InvalidArgumentException | ParseException e) {
//		e.printStackTrace();
//		logger.warn("sdp��������");
//		e.printStackTrace();
//	}
//	catch (SdpParseException e) {
//		e.printStackTrace();
//	}
//	catch (SdpException e) {
//		e.printStackTrace();
//	}
//}