#include "http.h"

#include "myjsondef.h"
#include "writer.h"
#include "stringbuffer.h"
#include "document.h"

#include "device/DeviceManager.h"
#include <iostream>
#include <sstream>
#include "UaClientCall.h"
#include "SipServer.h"
#include "tools/ownString.h"

using namespace std;

#define HTTP_TIMEOUT 10
size_t getUrlResponse(char* buffer, size_t size, size_t count, string* response)
{
	size_t recv_size = size * count;
	if (recv_size > 0)
	{
		response->append(buffer);
	}
	//response->clear();
	return recv_size;
}
string GetRequest(const string& url)
{
	string response;
	CURL* handle = curl_easy_init();
	if (handle == NULL)
		return "";
	//设置url
	curl_easy_setopt(handle, CURLOPT_URL, url.c_str());
	curl_easy_setopt(handle, CURLOPT_READFUNCTION, NULL);
	//注册回调函数
	curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, &getUrlResponse);
	//获取信息
	curl_easy_setopt(handle, CURLOPT_WRITEDATA, &response);

	curl_easy_setopt(handle, CURLOPT_NOSIGNAL, 1);
	curl_easy_setopt(handle, CURLOPT_CONNECTTIMEOUT, HTTP_TIMEOUT);
	curl_easy_setopt(handle, CURLOPT_TIMEOUT, HTTP_TIMEOUT);
	
	//执行请求
	CURLcode code = curl_easy_perform(handle);
	if (code == CURLE_OK)
		cout << response << endl;
	//释放
	curl_easy_cleanup(handle);
	return response;
}
size_t curlWriteCallback(char* buff, size_t size, size_t nmemb, string* response)
{
	size_t send_size = size * nmemb;
	if (send_size > 0)
	{
		response->append(buff);
	}
	//response->clear();
	return send_size;
}
string PostRequest(const string& url, const string& data)
{
	string response;
	CURL* handle = curl_easy_init();
	if (handle == NULL)
		return "";
	//设置url
	curl_easy_setopt(handle, CURLOPT_URL, url.c_str());
	curl_easy_setopt(handle, CURLOPT_POST, 1);
	curl_slist* plist = curl_slist_append(NULL, "Content-Type:application/json;charset=UTF-8");
	curl_easy_setopt(handle, CURLOPT_HTTPHEADER, plist);
	curl_easy_setopt(handle, CURLOPT_WRITEDATA, &response);
	curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, &curlWriteCallback);
	curl_easy_setopt(handle, CURLOPT_POSTFIELDS, data.c_str());
	curl_easy_setopt(handle, CURLOPT_POSTFIELDSIZE, data.size());

	curl_easy_setopt(handle, CURLOPT_NOSIGNAL, 1);
	curl_easy_setopt(handle, CURLOPT_CONNECTTIMEOUT, HTTP_TIMEOUT);
	curl_easy_setopt(handle, CURLOPT_TIMEOUT, HTTP_TIMEOUT);

	//执行请求
	CURLcode code = curl_easy_perform(handle);
	if (code == CURLE_OK)
		cout << response << endl;
	//释放
	curl_easy_cleanup(handle);
	return response;
}
static const char* g_msg200Ok = "HTTP/1.1 200 OK\r\n"
"Content-Type: application/json;charset=utf-8\r\n"
"Content-Length: 0\r\n"
"Access-Control-Allow-Methods: POST, GET\r\n"
"Access-Control-Allow-Headers:Content-Type, Access-Control-Allow-Headers, Authorization, X-Requested-With\r\n"
"Access-Control-Allow-Origin: *\r\n\r\n";

static const char* g_msg200Ok_msg = "HTTP/1.1 200 OK\r\n"
"Content-Type: application/json;charset=utf-8\r\n"
"Content-Length: %ld\r\n"
"Access-Control-Allow-Methods: POST, GET, OPTIONS\r\n"
"Access-Control-Allow-Headers:Content-Type, Access-Control-Allow-Headers, Authorization, X-Requested-With\r\n"
"Access-Control-Allow-Origin: *\r\n\r\n%s";

static const char* g_msg200jsonerror = "HTTP/1.1 200 Json Error\r\n"
"Content-Type: application/json;charset=utf-8\r\n"
"Content-Length: 0\r\n"
"Access-Control-Allow-Methods: POST, GET, OPTIONS\r\n"
"Access-Control-Allow-Headers:Content-Type, Access-Control-Allow-Headers, Authorization, X-Requested-With\r\n"
"Access-Control-Allow-Origin: *\r\n\r\n";

static const char* g_msg200Unauth_msg = "HTTP/1.1 200 Unauthorized\r\n"
"Content-Type: application/json;charset=utf-8\r\n"
"Content-Length: %ld\r\n"
"Access-Control-Allow-Methods: POST, GET\r\n"
"Access-Control-Allow-Headers:Content-Type, Access-Control-Allow-Headers, Authorization, X-Requested-With\r\n"
"Access-Control-Allow-Origin: *\r\n\r\n%s";

static const char* g_msg401 = "HTTP/1.1 401 Unauthorized\r\n"
"Content-Type: application/json;charset=utf-8\r\n"
"Content-Length: 0\r\n"
"Access-Control-Allow-Methods: POST, GET\r\n"
"Access-Control-Allow-Headers:Content-Type, Access-Control-Allow-Headers, Authorization, X-Requested-With\r\n"
"Access-Control-Allow-Origin: *\r\n\r\n";

static const char* g_msg401Denyed_msg = "HTTP/1.1 401 denyed\r\n"
"Content-Type: application/json;charset=utf-8\r\n"
"Content-Length: %d\r\n"
"Access-Control-Allow-Methods: POST, GET\r\n"
"Access-Control-Allow-Headers:Content-Type, Access-Control-Allow-Headers, Authorization, X-Requested-With\r\n"
"Access-Control-Allow-Origin: *\r\n\r\n%s";

struct mg_serve_http_opts g_webOpts;
HttpServer::HttpServer(int port):m_run_status(0), httpcon(NULL)
{
	m_address = to_string(port);
	memset(&m_mgr, 0, sizeof(m_mgr));
	memset(&g_webOpts, 0, sizeof(g_webOpts));
	g_webOpts.document_root = "./web";
	g_webOpts.index_files = "index.html";
	g_webOpts.enable_directory_listing = "yes";
	mg_mgr_init(&m_mgr, NULL);
	init();
}
HttpServer::~HttpServer()
{
	mg_mgr_free(&m_mgr);
}
bool HttpServer::init(const string& rootpath, const string& indexfile)
{
	g_webOpts.document_root = "./web";
	g_webOpts.index_files = "index.html";
	g_webOpts.enable_directory_listing = "yes";

	struct mg_connection* httpcon = mg_bind(&m_mgr, m_address.c_str(), HandleDefault);
	if (httpcon == NULL)
	{
		return false;
	}
	//websocket请求接口
	//mg_register_http_endpoint(httpcon, "/ws", WebsocketMessageProcess);

	mg_register_http_endpoint(httpcon, "/device/channels", DeviceChannelList);
	mg_register_http_endpoint(httpcon, "/device/open_live", StartLive);
	mg_register_http_endpoint(httpcon, "/device/close_live", StopLive);
	mg_register_http_endpoint(httpcon, "/userAction", UserManager);
	mg_register_http_endpoint(httpcon, "/index/hook/on_stream_none_reader", zlmHookStreamNoneReader);
	mg_register_http_endpoint(httpcon, "/index/hook/on_publish", zlmHookPublish);
	//mg_register_http_endpoint(httpcon, "//index/hook/on_play", UserManager);
	mg_register_http_endpoint(httpcon, "/index/hook/on_stream_changed", zlmHookStreamChanged);
	mg_register_http_endpoint(httpcon, "/index/hook/on_stream_not_found", zlmHookStreamNotFound);
	mg_register_http_endpoint(httpcon, "/index/hook/on_send_rtp_stopped", zlmHookSendRtpStopped);
	
	mg_set_protocol_http_websocket(httpcon);

	m_run_status = 1;
	mTLoop = thread(&StartFun, this);
	return true;
}
void HttpServer::loop()
{
	int count = 0;
	while (m_run_status)
	{
		mg_mgr_poll(&m_mgr, 1000);
		if (count++ > 60)
		{
			string wid;
			string url = "http://82.157.16.246:8080/device/auth-gb";

			rapidjson::StringBuffer response;
			rapidjson::Writer<rapidjson::StringBuffer> writer(response);
			writer.StartObject();
			writer.Key("gbid"); writer.String("");
			writer.Key("account"); writer.String("");
			writer.Key("passwd"); writer.String("");
			writer.EndObject();



			string strresponse = PostRequest(url, response.GetString());
			rapidjson::Document document;
			document.Parse((char*)strresponse.c_str());
			if (!document.HasParseError())
			{
				if (document.HasMember("data"))
				{
					rapidjson::Value& msbody = document["data"];
					wid = json_check_string(msbody, "wid");
					if (wid.empty())
					{
						string url = "http://82.157.16.246/device/line";

						rapidjson::StringBuffer res;
						rapidjson::Writer<rapidjson::StringBuffer> writers(res);
						writers.StartObject();

						writers.Key("onlineWids");
						writer.StartArray();
						writer.String("34020000001320000001");
						writer.EndArray();

						writers.Key("offlineWids");
						writer.StartArray();
						writer.String("34020000001320000002");
						writer.EndArray();

						writers.EndObject();
						string strRes = PostRequest(url, res.GetString());
						cout << strRes << endl;
					}
				}
			}
			count = 0;
		}
	}
	return ;
}
void HttpServer::StartFun(void* data)
{
	HttpServer* pThis = (HttpServer*)data;
	if (pThis)
		pThis->loop();
	return;
}
void WebAddUserAction(struct mg_connection* conn, rapidjson::Document& document, rapidjson::StringBuffer& response)
{
	int errcode = 0;
	if (document.HasMember("data"))
	{
		rapidjson::Value& msbody = document["data"];
		string strUser = json_check_string(msbody, "user");
		string strName = json_check_string(msbody, "name");
		string strDomain = json_check_string(msbody, "domain");
		string strPswd = json_check_string(msbody, "password");
		string strEmail = json_check_string(msbody, "email");
		if ("addUser")
		{
			//ok
		}
		else
		{
			//error 
			errcode = -1;
		}
	}
	rapidjson::Writer<rapidjson::StringBuffer> writer(response);
	writer.StartObject();
	writer.Key("code"); writer.Int(errcode);
	writer.Key("msg"); writer.String("");
	writer.EndObject();
	return;
}
void WebDelUserAction(struct mg_connection* conn, rapidjson::Document& document, rapidjson::StringBuffer& response)
{
	int errcode = 0;
	if (document.HasMember("data"))
	{
		rapidjson::Value& msbody = document["data"];
		string strUser = json_check_string(msbody, "user");
		/*string strName = json_check_string(msbody, "name");
		string strDomain = json_check_string(msbody, "domain");
		string strPswd = json_check_string(msbody, "password");
		string strEmail = json_check_string(msbody, "email");*/
		if ("deluser")
		{
			//ok
		}
		else
		{
			//error 
			errcode = -1;
		}
	}
	rapidjson::Writer<rapidjson::StringBuffer> writer(response);
	writer.StartObject();
	writer.Key("code"); writer.Int(errcode);
	writer.Key("msg"); writer.String("");
	writer.EndObject();
	return;
}
void WebChangeUserAction(struct mg_connection* conn, rapidjson::Document& document, rapidjson::StringBuffer& response)
{
	int errcode = 0;
	if (document.HasMember("data"))
	{
		rapidjson::Value& msbody = document["data"];
		string strUser = json_check_string(msbody, "user");
		string strName = json_check_string(msbody, "name");
		string strDomain = json_check_string(msbody, "domain");
		string strPswd = json_check_string(msbody, "password");
		string strEmail = json_check_string(msbody, "email");
		if ("changeuser")
		{
			//ok
		}
		else
		{
			//error 
			errcode = -1;
		}
	}
	rapidjson::Writer<rapidjson::StringBuffer> writer(response);
	writer.StartObject();
	writer.Key("code"); writer.Int(errcode);
	writer.Key("msg"); writer.String("");
	writer.EndObject();
	return;
}
void HttpServer::HandleDefault(struct mg_connection* nc, int ev, void* ev_data)
{
	struct http_message* hm = (struct http_message*)ev_data;
	if (!hm)
	{
		mg_printf(nc, g_msg200Ok);
		nc->flags |= MG_F_SEND_AND_CLOSE;
		return;
	}
	switch (ev)
	{
	case MG_EV_HTTP_REQUEST:
		mg_serve_http(nc, hm, g_webOpts);
		break;
	default:
		break;
	}
	return;
}
void HttpServer::WebsocketMessageProcess(struct mg_connection* websocketC, char* message, int len)
{
	return ;
}
void HttpServer::DeviceOnLineState(struct mg_connection* nc, int ev, void* ev_data)
{
	struct http_message* hm = (struct http_message*)ev_data;
	if (!hm || !hm->body.len ||
		(strncmp(hm->method.p, "GET", hm->method.len) &&
			strncmp(hm->method.p, "POST", hm->method.len) &&
			strncmp(hm->method.p, "PUT", hm->method.len))
		)
	{
		mg_printf(nc, g_msg200Ok);
		nc->flags |= MG_F_SEND_AND_CLOSE;
		return;
	}
	else
	{
		std::string jsonStr(hm->body.p, hm->body.len);
		rapidjson::Document document;
		document.Parse((char*)jsonStr.c_str());
		if (document.HasParseError())
		{
			mg_printf(nc, g_msg200jsonerror);
			nc->flags |= MG_F_SEND_AND_CLOSE;
			return;
		}

		std::string cmd = json_check_string(document, "cmd");
		rapidjson::StringBuffer response;
		/*if (strcmp(cmdGetPreviewUrl, cmd.c_str()) == 0)
			WebGetPreviewUrlAction(nc, document, response, userInfo);*/
		//else if(strcmp(cmdGetPlaybackUrl, cmd.c_str()) == 0)
		//    WebIpcPresetCtrlAction(nc, document, response, userInfo);
		//else if(strcmp(cmdPlaybackCtrl, cmd.c_str()) == 0)
		//    WebIpcCruiseCtrlAction(nc, document, response, userInfo);
		/*else
			CommonResponseParamToJson(cmd.c_str(), -1, _ER_INVALID_CMD, response);*/
		mg_printf(nc, g_msg200Ok_msg, response.GetSize(), response.GetString());
		nc->flags |= MG_F_SEND_AND_CLOSE;
	}
	return;
}
void HttpServer::DeviceChannelList(struct mg_connection* nc, int ev, void* ev_data)
{
	struct http_message* hm = (struct http_message*)ev_data;
	if (!hm || !hm->body.len ||
		(strncmp(hm->method.p, "GET", hm->method.len) &&
			strncmp(hm->method.p, "POST", hm->method.len) &&
			strncmp(hm->method.p, "PUT", hm->method.len))
		)
	{
		mg_printf(nc, g_msg200Ok);
		nc->flags |= MG_F_SEND_AND_CLOSE;
		return;
	}
	else
	{
		std::string jsonStr(hm->body.p, hm->body.len);
		rapidjson::Document document;
		document.Parse((char*)jsonStr.c_str());
		if (document.HasParseError())
		{
			mg_printf(nc, g_msg200jsonerror);
			nc->flags |= MG_F_SEND_AND_CLOSE;
			return;
		}

		string wid = json_check_string(document, "wid");
		string channel = json_check_string(document, "channel");
		list<std::shared_ptr<Device>> devlist;
		IDeviceMngrSvr& devMgr = GetIDeviceMngr();
		rapidjson::StringBuffer response;
		rapidjson::Writer<rapidjson::StringBuffer> writer(response);
		if (channel.empty())
		{
			writer.StartObject();
			writer.Key("code"); writer.Int(0);
			writer.Key("msg"); writer.String("");
			writer.Key("data");
			writer.StartArray();
			devMgr.GetAllDeviceList(devlist);
			for (auto& iter : devlist)
			{
				if (iter->getDevAccessProtocal() == Device::DEV_ACCESS_GB28181)
				{
					std::shared_ptr<SipServerDeviceInfo> devInfo = std::static_pointer_cast<SipServerDeviceInfo>(iter);
					writer.StartObject();
					writer.Key("channel"); writer.String(devInfo->getDeviceId().c_str());
					writer.Key("description"); writer.String("");
					writer.Key("name"); writer.String(devInfo->getName().c_str());
					writer.EndObject();
				}
			}
			writer.EndArray();
			writer.EndObject();
		}
		else
		{
			writer.StartObject();
			writer.Key("code"); writer.Int(0);
			writer.Key("msg"); writer.String("");
			writer.Key("data");
			writer.StartArray();
			list<std::shared_ptr<IDeviceChannel>> channellist;
			std::shared_ptr<Device> devInfo = devMgr.GetDeviceChannelList(channel, channellist);
			for (auto& itr : channellist)
			{
				std::shared_ptr<GBDeviceChannel> chlInfo = std::static_pointer_cast<GBDeviceChannel>(itr);
				if (chlInfo)
				{
					writer.StartObject();
					writer.Key("channel"); writer.String(chlInfo->getChannelId().c_str());
					writer.Key("description"); writer.String("");
					writer.Key("name"); writer.String(chlInfo->getName().c_str());
					writer.EndObject();
				}
			}
			writer.EndArray();
			writer.EndObject();
		}
		
		mg_printf(nc, g_msg200Ok_msg, response.GetSize(), response.GetString());
		nc->flags |= MG_F_SEND_AND_CLOSE;
	}
	return;
}
bool HttpIsStreamExist(std::string schema, std::string app, std::string streamId)
{
	sipserver::SipServer* pSvr = GetServer();
	if (pSvr)
	{
		rapidjson::Document document;
		std::ostringstream ss;
		ss << "http://" << pSvr->zlmHost << ":" << pSvr->zlmHttpPort << "/index/api/getRtpInfo?secret=035c73f7-bb6b-4889-a715-d9eb2d1925cc&"
			<< "stream=" << streamId;
		ss.flush();
		std::string strResponse = GetRequest(ss.str());
		document.Parse((char*)strResponse.c_str());
		if (!document.HasParseError())
		{
			if (document.HasMember("exist"))
			{
				if (document["exist"].IsBool())
				{
					return document["online"].GetBool();
				}
				else
				{
					cout << "online not bool type" << endl;
				}
			}
		}
	}
	return false;
}
std::string GetMediaConfigInfo()
{
	sipserver::SipServer* pSvr = GetServer();
	if (pSvr)
	{
		rapidjson::Document document;
		std::ostringstream ss;
		ss << "http://" << pSvr->zlmHost << ":" << pSvr->zlmHttpPort << "/index/api/getServerConfig?secret=035c73f7-bb6b-4889-a715-d9eb2d1925cc";
		ss.flush();
		std::string strResponse = GetRequest(ss.str());
		document.Parse((char*)strResponse.c_str());
		if (!document.HasParseError())
		{
			if (document.HasMember("code"))
			{
				int iCode = json_check_int32(document, "code");
				if (iCode == 0 && document.HasMember("data") && document["data"].IsArray())
				{
					rapidjson::Value& msbody = document["data"][0];
					rapidjson::StringBuffer sbBuf;
					rapidjson::Writer<rapidjson::StringBuffer> jWriter(sbBuf);
					msbody.Accept(jWriter);
					return std::string(sbBuf.GetString());
				}
			}
		}
	}
	return "";
}
void HttpServer::StartLive(struct mg_connection* nc, int ev, void* ev_data)
{
	struct http_message* hm = (struct http_message*)ev_data;
	if (!hm || !hm->body.len ||
		(strncmp(hm->method.p, "GET", hm->method.len) &&
			strncmp(hm->method.p, "POST", hm->method.len) &&
			strncmp(hm->method.p, "PUT", hm->method.len))
		)
	{
		mg_printf(nc, g_msg200Ok);
		nc->flags |= MG_F_SEND_AND_CLOSE;
		return;
	}
	else
	{
		std::string jsonStr(hm->body.p, hm->body.len);
		rapidjson::Document document;
		document.Parse((char*)jsonStr.c_str());
		if (document.HasParseError())
		{
			mg_printf(nc, g_msg200jsonerror);
			nc->flags |= MG_F_SEND_AND_CLOSE;
			return;
		}

		std::string wid = json_check_string(document, "wid");
		std::string channel = json_check_string(document, "channel");
		if (channel.empty())
			channel = wid;
		std::string app = json_check_string(document, "app");
		std::string stream_Id = json_check_string(document, "stream");
		std::string deviceId;

		rapidjson::StringBuffer response;
		rapidjson::Writer<rapidjson::StringBuffer> writer(response);
		//查询流是否存在
		bool isStream = false;
		sipserver::SipServer* pSvr = GetServer();
		if (pSvr)
		{
			resip::UaMgr* pUaMgr = pSvr->GetUaManager();
			if (pUaMgr)
			{
				resip::UaMgr::streamStatus smStatus = pUaMgr->getStreamStatus(channel);
				if (smStatus == resip::UaMgr::_UERAGERNT_NOT_STREAM)
				{
					IDeviceMngrSvr& DevMgr = GetIDeviceMngr();
					SipServerDeviceInfo devuinfo;
					list<GBDeviceChannel> chlist = DevMgr.GetGBDeviceMapper().GetGBDeviceChannelMapper().queryChannelByChannelId(channel);
					for (auto& it : chlist)
					{
						if (it.getStatus())
						{
							devuinfo = DevMgr.GetGBDeviceMapper().getDeviceByDeviceId(it.getDeviceId());
							break;
						}
					}
					if (!devuinfo.getDeviceId().empty())
					{
						deviceId = devuinfo.getDeviceId();
						if (stream_Id.empty())
						{
							stream_Id = std::str_format("%s_%s", deviceId.c_str(), channel.c_str());
						}
						//判断流存在
						ownThreadPool::myThreadPool& tPool = ownThreadPool::GetThreadPool();
						tPool.submitTask(std::make_shared<resip::RequestStreamTask>(devuinfo.getDeviceId(), devuinfo.getIp(), devuinfo.getPort(), channel, *pUaMgr, pUaMgr->GetAvailableRtpPort()));
					}
					else
					{
						std::cout << "******************* channel:" << channel << " not found" << std::endl;
					}
				}
				else if (smStatus == resip::UaMgr::_UERAGERNT_STREAM_OK)
				{
					UaClientCall* pCall = pUaMgr->reTurnCallByStreamId(channel);
					if (pCall)
					{
						if (stream_Id.empty())
						{
							stream_Id = std::str_format("%s_%s", pCall->mMyUacInviteVideoInfo.devId.c_str(), channel.c_str());
						}
					}
					isStream = true;
					
				}
				if (smStatus != resip::UaMgr::_UERAGERNT_STREAM_OK)
				{
					for (int i = 0; i < 2 * 10; i++)
					{
						smStatus = pUaMgr->getStreamStatus(channel);
						if (smStatus == resip::UaMgr::_UERAGERNT_STREAM_OK)
						{
							std::this_thread::sleep_for(std::chrono::milliseconds(500));
							//if (HttpIsStreamExist("rtsp", "rtp", stream_Id))
							{
								isStream = true;
								break;
							}
						}
						std::this_thread::sleep_for(std::chrono::milliseconds(500));
					}
					if (!isStream)
					{
						std::cout << "******************* channel:" << channel << " zlm timeout " << std::endl;
					}
				}
			}
		}

		//存在
		if (isStream)
		{
			if (stream_Id.empty())
			{
				IDeviceMngrSvr& DevMgr = GetIDeviceMngr();
				SipServerDeviceInfo devuinfo;
				list<GBDeviceChannel> chlist = DevMgr.GetGBDeviceMapper().GetGBDeviceChannelMapper().queryChannelByChannelId(channel);
				for (auto& it : chlist)
				{
					if (it.getStatus())
					{
						devuinfo = DevMgr.GetGBDeviceMapper().getDeviceByDeviceId(it.getDeviceId());
						break;
					}
				}
				if (!devuinfo.getDeviceId().empty())
				{
					deviceId = devuinfo.getDeviceId();
					if (stream_Id.empty())
					{
						stream_Id = std::str_format("%s_%s", deviceId.c_str(), channel.c_str());
					}
				}
			}
			writer.StartObject();
			writer.Key("code"); writer.Int(0);
			writer.Key("msg"); writer.String("");
			writer.Key("data");
			writer.StartObject();
				writer.Key("app"); writer.String("rtp");
				writer.Key("stream"); writer.String(stream_Id.c_str());
				writer.Key("channelId"); writer.String(channel.c_str());
				writer.Key("deviceID"); writer.String(deviceId.c_str());
				writer.Key("host"); writer.String(pSvr->zlmHost.c_str());
				std::string url = std::str_format("http://%s:%d/rtp/%s.live.mp4", pSvr->zlmHost.c_str(), pSvr->zlmHttpPort, stream_Id.c_str());
				writer.Key("fmp4"); writer.String(url.c_str());
				std::string rtcUrl = 
					std::str_format("https://%s:443/index/api/webrtc?app=rtp&stream=%s&type=play",
						pSvr->zlmHost.c_str(), stream_Id.c_str());
				writer.Key("rtc"); writer.String(rtcUrl.c_str());
				//writer.Key("mediainfo"); writer.String(GetMediaConfigInfo().c_str());
				std::string mediaInfo = GetMediaConfigInfo();
				writer.Key("mediainfo"); writer.RawValue(mediaInfo.c_str(), mediaInfo.size(), rapidjson::kObjectType);
			
				/*std::vector<std::string> stdvc;
				stdvc.*/

				/*writer.Key("app"); writer.String("rtp");
				writer.Key("app"); writer.String("rtp");
				writer.Key("app"); writer.String("rtp");
				writer.Key("app"); writer.String("rtp");*/

				/*{
					"app": "rtp",
					"stream" : "34020000002000000001_34020000001320000015",
					"deviceID" : "34020000002000000001",
					"channelId" : "34020000001320000015",
					"flv" : "http://192.168.1.232:80/rtp/34020000002000000001_34020000001320000015.live.flv",
					"https_flv" : "https://192.168.1.232:443/rtp/34020000002000000001_34020000001320000015.live.flv",
					"ws_flv" : "ws://192.168.1.232:80/rtp/34020000002000000001_34020000001320000015.live.flv",
					"wss_flv" : "wss://192.168.1.232:443/rtp/34020000002000000001_34020000001320000015.live.flv",
					"fmp4" : "http://192.168.1.232:80/rtp/34020000002000000001_34020000001320000015.live.mp4",
					"https_fmp4" : "https://192.168.1.232:443/rtp/34020000002000000001_34020000001320000015.live.mp4",
					"ws_fmp4" : "ws://192.168.1.232:80/rtp/34020000002000000001_34020000001320000015.live.mp4",
					"wss_fmp4" : "wss://192.168.1.232:443/rtp/34020000002000000001_34020000001320000015.live.mp4",
					"hls" : "http://192.168.1.232:80/rtp/34020000002000000001_34020000001320000015/hls.m3u8",
					"https_hls" : "https://192.168.1.232:443/rtp/34020000002000000001_34020000001320000015/hls.m3u8",
					"ws_hls" : "ws://192.168.1.232:80/rtp/34020000002000000001_34020000001320000015/hls.m3u8",
					"wss_hls" : "wss://192.168.1.232:443/rtp/34020000002000000001_34020000001320000015/hls.m3u8",
					"ts" : "http://192.168.1.232:80/rtp/34020000002000000001_34020000001320000015.live.ts",
					"https_ts" : "https://192.168.1.232:443/rtp/34020000002000000001_34020000001320000015.live.ts",
					"ws_ts" : "ws://192.168.1.232:80/rtp/34020000002000000001_34020000001320000015.live.ts",
					"wss_ts" : "wss://192.168.1.232:443/rtp/34020000002000000001_34020000001320000015.live.ts",
					"rtmp" : "rtmp://192.168.1.232:1935/rtp/34020000002000000001_34020000001320000015",
					"rtmps" : null,
					"rtsp" : "rtsp://192.168.1.232:554/rtp/34020000002000000001_34020000001320000015",
					"rtsps" : null,
					"rtc" : "https://192.168.1.232:443/index/api/webrtc?app=rtp&stream=34020000002000000001_34020000001320000015&type=play",
					"mediaServerId" : "FQ3TF8yT83wh5Wvz",
					"tracks" : [{
					"codecIdName": null,
						"codecId" : 0,
						"channels" : 0,
						"sampleBit" : 0,
						"ready" : true,
						"fps" : 0,
						"width" : 1920,
						"codecType" : 0,
						"sampleRate" : 0,
						"height" : 1080
				}],
					"startTime": null,
						"endTime" : null,
						"progress" : 0.0,
						"transactionInfo" : null
				}
				}*/
				writer.EndObject();
			writer.EndObject();
		}
		else
		{
			writer.StartObject();
			writer.Key("code"); writer.Int(1);
			writer.Key("msg"); writer.String(std::GbkToUtf8("播放失败").c_str());
			writer.EndObject();
		}
		mg_printf(nc, g_msg200Ok_msg, response.GetSize(), response.GetString());
		nc->flags |= MG_F_SEND_AND_CLOSE;
	}
	return;
}
void HttpServer::StopLive(struct mg_connection* nc, int ev, void* ev_data)
{
	struct http_message* hm = (struct http_message*)ev_data;
	if (!hm || !hm->body.len ||
		(strncmp(hm->method.p, "GET", hm->method.len) &&
			strncmp(hm->method.p, "POST", hm->method.len) &&
			strncmp(hm->method.p, "PUT", hm->method.len))
		)
	{
		mg_printf(nc, g_msg200Ok);
		nc->flags |= MG_F_SEND_AND_CLOSE;
		return;
	}
	else
	{
		std::string jsonStr(hm->body.p, hm->body.len);
		rapidjson::Document document;
		document.Parse((char*)jsonStr.c_str());
		if (document.HasParseError())
		{
			mg_printf(nc, g_msg200jsonerror);
			nc->flags |= MG_F_SEND_AND_CLOSE;
			return;
		}

		std::string wid = json_check_string(document, "wid");
		std::string channel = json_check_string(document, "channel");
		std::string app = json_check_string(document, "app");
		std::string stream_Id = json_check_string(document, "stream");
		std::string ssrc = json_check_string(document, "ssrc");

		sipserver::SipServer* pSvr = GetServer();
		if (pSvr)
		{
			resip::UaMgr* pUaMgr = pSvr->GetUaManager();
			if (pUaMgr)
			{
				pUaMgr->CloseStreamStreamId(channel);
			}
		}
		rapidjson::StringBuffer response;
		rapidjson::Writer<rapidjson::StringBuffer> writer(response);
		writer.StartObject();
		writer.Key("code"); writer.Int(0);
		writer.Key("msg"); writer.String("");
		writer.EndObject();

		mg_printf(nc, g_msg200Ok_msg, response.GetSize(), response.GetString());
		nc->flags |= MG_F_SEND_AND_CLOSE;
	}
	return;
}
void HttpServer::UserManager(struct mg_connection* nc, int ev, void* ev_data)
{
	struct http_message* hm = (struct http_message*)ev_data;
	if (!hm || !hm->body.len || (strncmp(hm->method.p, "GET", hm->method.len) &&
			strncmp(hm->method.p, "POST", hm->method.len) &&strncmp(hm->method.p, "PUT", hm->method.len))
		)
	{
		mg_printf(nc, g_msg200Ok);
		nc->flags |= MG_F_SEND_AND_CLOSE;
		return;
	}
	else
	{
		std::string jsonStr(hm->body.p, hm->body.len);
		rapidjson::Document document;
		document.Parse((char*)jsonStr.c_str());
		if (document.HasParseError())
		{
			mg_printf(nc, g_msg200jsonerror);
			nc->flags |= MG_F_SEND_AND_CLOSE;
			return;
		}

		std::string cmd = json_check_string(document, "cmd");
		rapidjson::StringBuffer response;
		if (strcmp("addUser", cmd.c_str()) == 0)
			WebAddUserAction(nc, document, response);
		else if(strcmp("delUser", cmd.c_str()) == 0)
			WebDelUserAction(nc, document, response);
		else if(strcmp("changeUser", cmd.c_str()) == 0)
			WebChangeUserAction(nc, document, response);
		else
		{
			rapidjson::Writer<rapidjson::StringBuffer> writer(response);
			writer.StartObject();
			writer.Key("code"); writer.Int(-2);
			writer.Key("msg"); writer.String("");
			writer.EndObject();
		}
		mg_printf(nc, g_msg200Ok_msg, response.GetSize(), response.GetString());
		nc->flags |= MG_F_SEND_AND_CLOSE;
	}
	return;
}
void HttpServer::zlmHookPublish(struct mg_connection* nc, int ev, void* ev_data)
{
	struct http_message* hm = (struct http_message*)ev_data;
	if (!hm || !hm->body.len || (strncmp(hm->method.p, "GET", hm->method.len) &&
		strncmp(hm->method.p, "POST", hm->method.len) && strncmp(hm->method.p, "PUT", hm->method.len))
		)
	{
		mg_printf(nc, g_msg200Ok);
		nc->flags |= MG_F_SEND_AND_CLOSE;
		return;
	}
	else
	{
		std::string jsonStr(hm->body.p, hm->body.len);
		cout << "*******************************\n"
			<< jsonStr << "\n"
			<< "*******************************\n" << endl;
		rapidjson::Document document;
		document.Parse((char*)jsonStr.c_str());
		if (document.HasParseError())
		{
			mg_printf(nc, g_msg200jsonerror);
			nc->flags |= MG_F_SEND_AND_CLOSE;
			return;
		}

		std::string strApp = json_check_string(document, "app");
		std::string strId = json_check_string(document, "id");
		std::string strIp = json_check_string(document, "ip");
		std::string strMediaServerId = json_check_string(document, "mediaServerId");
		int iOriginType = json_check_int32(document, "originType");
		std::string strOriginTypeStr = json_check_string(document, "originTypeStr");
		std::string strParams = json_check_string(document, "params");
		int iPort = json_check_int32(document, "port");
		std::string strSchema = json_check_string(document, "schema");
		std::string strStream = json_check_string(document, "stream");
		std::string strVhost = json_check_string(document, "vhost");
		
		rapidjson::StringBuffer response;
		rapidjson::Writer<rapidjson::StringBuffer> writer(response);
		writer.StartObject();
		writer.Key("code"); writer.Int(0);
		writer.Key("add_mute_audio"); writer.Bool(true);
		writer.Key("continue_push_ms"); writer.Int(10000);
		writer.Key("enable_audio"); writer.Bool(true);
		writer.Key("enable_fmp4"); writer.Bool(true);
		writer.Key("enable_hls"); writer.Bool(true);
		writer.Key("enable_mp4"); writer.Bool(false);
		writer.Key("enable_rtmp"); writer.Bool(true);
		writer.Key("enable_rtsp"); writer.Bool(true);
		writer.Key("enable_ts"); writer.Bool(true);
		writer.Key("hls_save_path"); writer.String("/hls_save_path/");
		writer.Key("modify_stamp"); writer.Bool(false);
		writer.Key("mp4_as_player"); writer.Bool(false);
		writer.Key("mp4_max_second"); writer.Int(3600);
		writer.Key("mp4_save_path"); writer.String("/mp4_save_path/");
		writer.EndObject();
		mg_printf(nc, g_msg200Ok_msg, response.GetSize(), response.GetString());
		nc->flags |= MG_F_SEND_AND_CLOSE;
	}
	return;
}
void HttpServer::zlmHookStreamNoneReader(struct mg_connection* nc, int ev, void* ev_data)
{
	struct http_message* hm = (struct http_message*)ev_data;
	if (!hm || !hm->body.len || (strncmp(hm->method.p, "GET", hm->method.len) &&
		strncmp(hm->method.p, "POST", hm->method.len) && strncmp(hm->method.p, "PUT", hm->method.len))
		)
	{
		mg_printf(nc, g_msg200Ok);
		nc->flags |= MG_F_SEND_AND_CLOSE;
		return;
	}
	else
	{
		std::string jsonStr(hm->body.p, hm->body.len);
		cout << "*******************************\n"
			<< jsonStr << "\n"
			<< "*******************************\n" << endl;
		rapidjson::Document document;
		document.Parse((char*)jsonStr.c_str());
		if (document.HasParseError())
		{
			mg_printf(nc, g_msg200jsonerror);
			nc->flags |= MG_F_SEND_AND_CLOSE;
			return;
		}

		std::string strApp = json_check_string(document, "app");
		std::string strMediaServerId = json_check_string(document, "mediaServerId");
		std::string strSchema = json_check_string(document, "schema");
		std::string strStream = json_check_string(document, "stream");
		std::string strVhost = json_check_string(document, "vhost");
		if (strApp == "rtp")
		{
			sipserver::SipServer* pSvr = GetServer();
			if (pSvr)
			{
				resip::UaMgr* pUaMgr = pSvr->GetUaManager();
				if (pUaMgr)
				{
					std::vector<std::string> streamIdVct = vStringSplit(strStream, "_");
					UaClientCall *pCall = pUaMgr->reTurnCallByStreamId(streamIdVct[1]);
					if (pCall)
					{
						pCall->terminateCall();
					}
				}
			}
		}

		rapidjson::StringBuffer response;
		rapidjson::Writer<rapidjson::StringBuffer> writer(response);
		writer.StartObject();
		writer.Key("close"); writer.Bool(true);
		writer.Key("code"); writer.Int(0);
		writer.EndObject();

		mg_printf(nc, g_msg200Ok_msg, response.GetSize(), response.GetString());
		nc->flags |= MG_F_SEND_AND_CLOSE;
	}
	return;
}
void HttpServer::zlmHookStreamChanged(struct mg_connection* nc, int ev, void* ev_data)
{
	struct http_message* hm = (struct http_message*)ev_data;
	if (!hm || !hm->body.len || (strncmp(hm->method.p, "GET", hm->method.len) &&
		strncmp(hm->method.p, "POST", hm->method.len) && strncmp(hm->method.p, "PUT", hm->method.len))
		)
	{
		mg_printf(nc, g_msg200Ok);
		nc->flags |= MG_F_SEND_AND_CLOSE;
		return;
	}
	else
	{
		std::string jsonStr(hm->body.p, hm->body.len);
		cout << "*******************************\n"
			<< jsonStr << "\n"
			<< "*******************************\n" << endl;
		rapidjson::Document document;
		document.Parse((char*)jsonStr.c_str());
		if (document.HasParseError())
		{
			mg_printf(nc, g_msg200jsonerror);
			nc->flags |= MG_F_SEND_AND_CLOSE;
			return;
		}
		int iAliveSecond = json_check_int32(document, "aliveSecond");
		std::string strApp = json_check_string(document, "app");
		int iBytesSpeed = json_check_int32(document, "bytesSpeed");
		uint64_t llCreateStamp = json_check_int64(document, "createStamp");
		bool bIsRecordingHLS = json_check_int64(document, "isRecordingHLS");
		bool bIsRecordingMP4 = json_check_int64(document, "isRecordingMP4");
		std::string strMediaServerId = json_check_string(document, "mediaServerId");
		if (document.HasMember("originSock"))
		{
			rapidjson::Value& msbody = document["originSock"];
			std::string strIdentifier = json_check_string(msbody, "identifier");
			std::string strLocal_ip = json_check_string(msbody, "local_ip");
			int iLocal_port = json_check_int32(msbody, "local_port");
			std::string strPeer_ip = json_check_string(msbody, "peer_ip");
			int iPeer_port = json_check_int32(msbody, "peer_port");
		}
		int iOriginType = json_check_int32(document, "originType");
		std::string strOriginTypeStr = json_check_string(document, "originTypeStr");
		std::string strOriginUrl = json_check_string(document, "originUrl");
		int iReaderCount = json_check_int32(document, "readerCount");
		bool bRegist = json_check_bool(document, "regist");
		std::string strSchema = json_check_string(document, "schema");
		std::string strStream = json_check_string(document, "stream");
		int iTotalReaderCount = json_check_int32(document, "totalReaderCount");
		if (document.HasMember("tracks") && document["tracks"].IsArray())
		{
			rapidjson::Value& msbody = document["tracks"];
			for (unsigned int i = 0; i < msbody.Size(); i++)
			{
				int iChannels = json_check_int32(msbody[i], "channels");
				int iCodec_id = json_check_int32(msbody[i], "codec_id");
				std::string strCodec_id_name = json_check_string(msbody[i], "codec_id_name");
				int iCodec_type = json_check_int32(msbody[i], "codec_type");
				int iLoss = json_check_int32(msbody[i], "loss");
				bool bReady = json_check_bool(msbody[i], "ready");
				int iSample_bit = json_check_int32(msbody[i], "sample_bit");
				int iSample_rate = json_check_int32(msbody[i], "sample_rate");
				int iFps = json_check_int32(msbody[i], "fps");
				int iHeight = json_check_int32(msbody[i], "height");
				int iWidth = json_check_int32(msbody[i], "width");
			}
		}
		std::string strVhost = json_check_string(document, "vhost");
		if (strSchema == "rtp")
		{
			sipserver::SipServer* pSvr = GetServer();
			if (pSvr)
			{
				resip::UaMgr* pUaMgr = pSvr->GetUaManager();
				if (pUaMgr)
				{
					std::vector<std::string> streamIdVct = vStringSplit(strStream, "_");
					UaClientCall* pCall = pUaMgr->reTurnCallByStreamId(streamIdVct[1]);
					if (pCall)
					{
						switch (pCall->mMyUacInviteVideoInfo.state)
						{
						case UaClientCall::UacInviteVideoInfo::_RES_CONNECT:
							pCall->mMyUacInviteVideoInfo.state = UaClientCall::UacInviteVideoInfo::_RES_OK;
							break;
						default:
							break;
						}
					}
				}
			}
		}
		rapidjson::StringBuffer response;
		rapidjson::Writer<rapidjson::StringBuffer> writer(response);
		writer.StartObject();
		writer.Key("code"); writer.Int(0);
		writer.Key("msg"); writer.String("success");
		writer.EndObject();

		mg_printf(nc, g_msg200Ok_msg, response.GetSize(), response.GetString());
		nc->flags |= MG_F_SEND_AND_CLOSE;
	}
	return;
}
void HttpServer::zlmHookStreamNotFound(struct mg_connection* nc, int ev, void* ev_data)
{
	struct http_message* hm = (struct http_message*)ev_data;
	if (!hm || !hm->body.len || (strncmp(hm->method.p, "GET", hm->method.len) &&
		strncmp(hm->method.p, "POST", hm->method.len) && strncmp(hm->method.p, "PUT", hm->method.len))
		)
	{
		mg_printf(nc, g_msg200Ok);
		nc->flags |= MG_F_SEND_AND_CLOSE;
		return;
	}
	else
	{
		std::string jsonStr(hm->body.p, hm->body.len);
		cout << "*******************************\n"
			<< jsonStr << "\n"
			<< "*******************************\n" << endl;
		rapidjson::Document document;
		document.Parse((char*)jsonStr.c_str());
		if (document.HasParseError())
		{
			mg_printf(nc, g_msg200jsonerror);
			nc->flags |= MG_F_SEND_AND_CLOSE;
			return;
		}

		/*std::string cmd = json_check_string(document, "cmd");
		rapidjson::StringBuffer response;
		if (strcmp("addUser", cmd.c_str()) == 0)
			WebAddUserAction(nc, document, response);
		else if (strcmp("delUser", cmd.c_str()) == 0)
			WebDelUserAction(nc, document, response);
		else if (strcmp("changeUser", cmd.c_str()) == 0)
			WebChangeUserAction(nc, document, response);
		else
		{
			rapidjson::Writer<rapidjson::StringBuffer> writer(response);
			writer.StartObject();
			writer.Key("code"); writer.Int(-2);
			writer.Key("msg"); writer.String("");
			writer.EndObject();
		}
		mg_printf(nc, g_msg200Ok_msg, response.GetSize(), response.GetString());*/
		nc->flags |= MG_F_SEND_AND_CLOSE;
	}
	return;
}
void HttpServer::zlmHookSendRtpStopped(struct mg_connection* nc, int ev, void* ev_data)
{
	struct http_message* hm = (struct http_message*)ev_data;
	if (!hm || !hm->body.len || (strncmp(hm->method.p, "GET", hm->method.len) &&
		strncmp(hm->method.p, "POST", hm->method.len) && strncmp(hm->method.p, "PUT", hm->method.len))
		)
	{
		mg_printf(nc, g_msg200Ok);
		nc->flags |= MG_F_SEND_AND_CLOSE;
		return;
	}
	else
	{
		std::string jsonStr(hm->body.p, hm->body.len);
		cout << "*******************************\n"
			<< jsonStr << "\n"
			<< "*******************************\n" << endl;
		rapidjson::Document document;
		document.Parse((char*)jsonStr.c_str());
		if (document.HasParseError())
		{
			mg_printf(nc, g_msg200jsonerror);
			nc->flags |= MG_F_SEND_AND_CLOSE;
			return;
		}

		/*std::string cmd = json_check_string(document, "cmd");
		rapidjson::StringBuffer response;
		if (strcmp("addUser", cmd.c_str()) == 0)
			WebAddUserAction(nc, document, response);
		else if (strcmp("delUser", cmd.c_str()) == 0)
			WebDelUserAction(nc, document, response);
		else if (strcmp("changeUser", cmd.c_str()) == 0)
			WebChangeUserAction(nc, document, response);
		else
		{
			rapidjson::Writer<rapidjson::StringBuffer> writer(response);
			writer.StartObject();
			writer.Key("code"); writer.Int(-2);
			writer.Key("msg"); writer.String("");
			writer.EndObject();
		}
		mg_printf(nc, g_msg200Ok_msg, response.GetSize(), response.GetString());*/
		nc->flags |= MG_F_SEND_AND_CLOSE;
	}
	return;
}
// Print HTTP response and signal that we're done
//static void fn(struct mg_connection* c, int ev, void* ev_data, void* fn_data)
//{
//	if (ev == MG_EV_OPEN)
//	{
//		// Connection created. Store connect expiration time in c->label
//		*(uint64_t*)c->label = mg_millis() + s_timeout_ms;
//	}
//	else if (ev == MG_EV_POLL)
//	{
//		if (mg_millis() > *(uint64_t*)c->label &&
//			(c->is_connecting || c->is_resolving)) {
//			mg_error(c, "Connect timeout");
//		}
//	}
//	else if (ev == MG_EV_CONNECT)
//	{
//		// Connected to server. Extract host name from URL
//		struct mg_str host = mg_url_host(s_url);
//
//		// If s_url is https://, tell client connection to use TLS
//		if (mg_url_is_ssl(s_url)) {
//			struct mg_tls_opts opts = { .ca = "ca.pem", .srvname = host };
//			mg_tls_init(c, &opts);
//		}
//
//		// Send request
//		int content_length = s_post_data ? strlen(s_post_data) : 0;
//		mg_printf(c,
//			"%s %s HTTP/1.0\r\n"
//			"Host: %.*s\r\n"
//			"Content-Type: octet-stream\r\n"
//			"Content-Length: %d\r\n"
//			"\r\n",
//			s_post_data ? "POST" : "GET", mg_url_uri(s_url), (int)host.len,
//			host.ptr, content_length);
//		mg_send(c, s_post_data, content_length);
//	}
//	else if (ev == MG_EV_HTTP_MSG)
//	{
//		// Response is received. Print it
//		struct mg_http_message* hm = (struct mg_http_message*)ev_data;
//		printf("%.*s", (int)hm->message.len, hm->message.ptr);
//		c->is_closing = 1;         // Tell mongoose to close this connection
//		*(bool*)fn_data = true;  // Tell event loop to stop
//	}
//	else if (ev == MG_EV_ERROR)
//	{
//		*(bool*)fn_data = true;  // Error, tell event loop to stop
//	}
//}
//
//int main(int argc, char* argv[]) {
//	const char* log_level = getenv("LOG_LEVEL");  // Allow user to set log level
//	if (log_level == NULL) log_level = "4";       // Default is verbose
//
//	struct mg_mgr mgr;              // Event manager
//	bool done = false;              // Event handler flips it to true
//	if (argc > 1) s_url = argv[1];  // Use URL provided in the command line
//	mg_log_set(atoi(log_level));    // Set to 0 to disable debug
//	mg_mgr_init(&mgr);              // Initialise event manager
//	mg_http_connect(&mgr, s_url, fn, &done);  // Create client connection
//	while (!done) mg_mgr_poll(&mgr, 50);      // Event manager loops until 'done'
//	mg_mgr_free(&mgr);                        // Free resources
//	return 0;
//}