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
#include "tools/m_Time.h"

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
	CHttpClient cHttpClient;
	return cHttpClient.HttpGetRequest(url);
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
	CHttpClient cHttpClient;
	return cHttpClient.HttpPostRequest(url, data);
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
CHttpClient::CHttpClient(uint64_t timeout)
{
	bExitFlag = 0;
	mTimeout_ms = timeout;
}

CHttpClient::~CHttpClient()
{
}

std::string CHttpClient::HttpGetRequest(const std::string& url)
{
	mUrl = url;
	struct mg_mgr mgr;
	mg_mgr_init(&mgr);
	mg_http_connect(&mgr, mUrl.c_str(), OnHttpEvent, this);
	while (!bExitFlag) mg_mgr_poll(&mgr, 50);
	mg_mgr_free(&mgr);
	return  mResponse;
}
std::string CHttpClient::HttpPostRequest(const std::string& url, const std::string& PostData)
{
	mUrl = url;
	mPostData = PostData;
	struct mg_mgr mgr;
	mg_mgr_init(&mgr);
	mg_http_connect(&mgr, mUrl.c_str(), OnHttpEvent, this);
	while (!bExitFlag) mg_mgr_poll(&mgr, 50);
	mg_mgr_free(&mgr);
	return  mResponse;
}
// Print HTTP response and signal that we're done
void CHttpClient::OnHttpEvent(struct mg_connection* c, int ev, void* ev_data, void* user_data)
{
	CHttpClient* pThis = (CHttpClient*)user_data;
	if (pThis == NULL)
	{
		return;
	}
	if (ev == MG_EV_OPEN)
	{
		// Connection created. Store connect expiration time in c->label
		*(uint64_t*)c->label = mg_millis() + pThis->mTimeout_ms;
	}
	else if (ev == MG_EV_POLL)
	{
		if (mg_millis() > *(uint64_t*)c->label && (c->is_connecting || c->is_resolving))
		{
			mg_error(c, "Connect timeout");
		}
	}
	else if (ev == MG_EV_CONNECT)
	{
		// Connected to server. Extract host name from URL
		struct mg_str host = mg_url_host(pThis->mUrl.c_str());
		// If s_url is https://, tell client connection to use TLS
		if (mg_url_is_ssl(pThis->mUrl.c_str()))
		{
			struct mg_tls_opts opts;// = {.ca = "ca.pem", .srvname = host};
			opts.ca = "ca.pem";
			opts.srvname = host;
			mg_tls_init(c, &opts);
		}
		// Send request
		mg_printf(c,
			"%s %s HTTP/1.0\r\n"
			"Host: %.*s\r\n"
			"Content-Type: application/json;charset=utf-8\r\n"
			"Content-Length: %d\r\n"
			"\r\n",
			!pThis->mPostData.empty() ? "POST" : "GET", mg_url_uri(pThis->mUrl.c_str()), (int)host.len,
			host.ptr, pThis->mPostData.size());
		mg_send(c, pThis->mPostData.c_str(), pThis->mPostData.size());
	}
	else if (ev == MG_EV_HTTP_MSG)
	{
		// Response is received. Print it
		struct mg_http_message* hm = (struct mg_http_message*)ev_data;
		//printf("%.*s", (int) hm->body.len, hm->body.ptr);
		pThis->mResponse = std::string(hm->body.ptr, hm->body.len);
		c->is_closing = 1;
		pThis->bExitFlag = true;
	}
	else if (ev == MG_EV_ERROR)
	{
		pThis->bExitFlag = true;
	}
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

HttpServer::HttpServer(int port):m_run_status(0)
{
	m_address = std::string("http://0.0.0.0:") + to_string(port);
	mg_mgr_init(&m_mgr);
	m_run_status = 1;
	mTLoop = thread(std::bind(&HttpServer::loop, this));
}
HttpServer::~HttpServer()
{
	mg_mgr_free(&m_mgr);
}
bool HttpServer::init(const string& rootpath, const string& indexfile)
{
	/*g_webOpts.document_root = "./web";
	g_webOpts.index_files = "index.html";
	g_webOpts.enable_directory_listing = "yes";*/

	//struct mg_connection* httpcon = mg_bind(&m_mgr, m_address.c_str(), HandleDefault);
	//if (httpcon == NULL)
	//{
	//	return false;
	//}
	////websocket请求接口
	////mg_register_http_endpoint(httpcon, "/ws", WebsocketMessageProcess);

	//mg_register_http_endpoint(httpcon, "/device/channels", DeviceChannelList);
	//mg_register_http_endpoint(httpcon, "/device/open_live", StartLive);
	//mg_register_http_endpoint(httpcon, "/device/close_live", StopLive);
	//mg_register_http_endpoint(httpcon, "/device/open_vod", StartVod);
	//mg_register_http_endpoint(httpcon, "/device/close_vod", StopVod);
	//mg_register_http_endpoint(httpcon, "/userAction", UserManager);
	//mg_register_http_endpoint(httpcon, "/index/hook/on_stream_none_reader", zlmHookStreamNoneReader);
	//mg_register_http_endpoint(httpcon, "/index/hook/on_publish", zlmHookPublish);
	////mg_register_http_endpoint(httpcon, "//index/hook/on_play", UserManager);
	//mg_register_http_endpoint(httpcon, "/index/hook/on_stream_changed", zlmHookStreamChanged);
	//mg_register_http_endpoint(httpcon, "/index/hook/on_stream_not_found", zlmHookStreamNotFound);
	//mg_register_http_endpoint(httpcon, "/index/hook/on_send_rtp_stopped", zlmHookSendRtpStopped);
	//
	//mg_set_protocol_http_websocket(httpcon);

	
	return true;
}
void HttpServer::loop()
{
	mg_http_listen(&m_mgr, m_address.c_str(), HandleDefault, this);  // Setup listener
	while (m_run_status)
		mg_mgr_poll(&m_mgr, 1000);                         // Event loop
	mg_mgr_free(&m_mgr);

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

void WebAddUserAction(rapidjson::Document& document, rapidjson::StringBuffer& response)
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
void WebDelUserAction(rapidjson::Document& document, rapidjson::StringBuffer& response)
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
void WebChangeUserAction(rapidjson::Document& document, rapidjson::StringBuffer& response)
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
void HttpServer::HandleDefault(struct mg_connection* c, int ev, void* ev_data, void* fn_data)
{
	//mg_register_http_endpoint(httpcon, "/device/channels", DeviceChannelList);
	//mg_register_http_endpoint(httpcon, "/device/open_live", StartLive);
	//mg_register_http_endpoint(httpcon, "/device/close_live", StopLive);
	//mg_register_http_endpoint(httpcon, "/device/open_vod", StartVod);
	//mg_register_http_endpoint(httpcon, "/device/close_vod", StopVod);
	//mg_register_http_endpoint(httpcon, "/userAction", UserManager);
	//mg_register_http_endpoint(httpcon, "/index/hook/on_stream_none_reader", zlmHookStreamNoneReader);
	//mg_register_http_endpoint(httpcon, "/index/hook/on_publish", zlmHookPublish);
	////mg_register_http_endpoint(httpcon, "//index/hook/on_play", UserManager);
	//mg_register_http_endpoint(httpcon, "/index/hook/on_stream_changed", zlmHookStreamChanged);
	//mg_register_http_endpoint(httpcon, "/index/hook/on_stream_not_found", zlmHookStreamNotFound);
	//mg_register_http_endpoint(httpcon, "/index/hook/on_send_rtp_stopped", zlmHookSendRtpStopped);
	if (ev == MG_EV_HTTP_MSG)
	{
		struct mg_http_message* hm = (struct mg_http_message*)ev_data;
		if (mg_http_match_uri(hm, "/device/channels"))
		{
			// Attempt to fetch parameters from the body, hm->body
			std::string strOut;
			DeviceChannelList(hm->body, strOut);
			struct mg_str params = hm->body;
			double num1, num2;
			if (mg_json_get_num(params, "$[0]", &num1) &&
				mg_json_get_num(params, "$[1]", &num2)) {
				// Success! create a JSON response
				mg_http_reply(c, 200, "Content-Type: application/json;charset=utf-8\r\n", "{%Q:%g}\n",
					"result", num1 + num2);
			}
			else {
				mg_http_reply(c, 500, NULL, "%s", "Parameters missing");
			}
		}
		else if (mg_http_match_uri(hm, "/device/open_live"))
		{
			std::string strOut;
			StartLive(hm->body, strOut);
			mg_http_reply(c, 200, "Content-Type: application/json;charset=utf-8\r\n", "%s\n", strOut.c_str());
		}
		else if (mg_http_match_uri(hm, "/device/close_live"))
		{
			std::string strOut;
			StopLive(hm->body, strOut);
			mg_http_reply(c, 200, "Content-Type: application/json;charset=utf-8\r\n", "%s\n", strOut.c_str());
		}
		else if (mg_http_match_uri(hm, "/device/open_vod"))
		{
			std::string strOut;
			StartVod(hm->body, strOut);
			mg_http_reply(c, 200, "Content-Type: application/json;charset=utf-8\r\n", "%s\n", strOut.c_str());
		}
		else if (mg_http_match_uri(hm, "/device/close_vod"))
		{
			std::string strOut;
			StopVod(hm->body, strOut);
			mg_http_reply(c, 200, "Content-Type: application/json;charset=utf-8\r\n", "%s\n", strOut.c_str());
		}
		else if (mg_http_match_uri(hm, "/userAction"))
		{
			std::string strOut;
			UserManager(hm->body, strOut);
			mg_http_reply(c, 200, "Content-Type: application/json;charset=utf-8\r\n", "%s\n", strOut.c_str());
		}
		else if (mg_http_match_uri(hm, "/index/hook/on_stream_none_reader"))
		{
			std::string strOut;
			zlmHookStreamNoneReader(hm->body, strOut);
			mg_http_reply(c, 200, "Content-Type: application/json;charset=utf-8\r\n", "%s\n", strOut.c_str());
		}
		else if (mg_http_match_uri(hm, "/index/hook/on_publish"))
		{
			std::string strOut;
			zlmHookPublish(hm->body, strOut);
			mg_http_reply(c, 200, "Content-Type: application/json;charset=utf-8\r\n", "%s\n", strOut.c_str());
		}
		else if (mg_http_match_uri(hm, "/index/hook/on_stream_changed"))
		{
			std::string strOut;
			zlmHookStreamChanged(hm->body, strOut);
			mg_http_reply(c, 200, "Content-Type: application/json;charset=utf-8\r\n", "%s\n", strOut.c_str());
		}
		else if (mg_http_match_uri(hm, "/index/hook/on_stream_not_found"))
		{
			std::string strOut;
			zlmHookStreamNotFound(hm->body, strOut);
			mg_http_reply(c, 200, "Content-Type: application/json;charset=utf-8\r\n", "%s\n", strOut.c_str());
		}
		else if (mg_http_match_uri(hm, "/index/hook/on_send_rtp_stopped"))
		{
			std::string strOut;
			zlmHookSendRtpStopped(hm->body, strOut);
			mg_http_reply(c, 200, "Content-Type: application/json;charset=utf-8\r\n", "%s\n", strOut.c_str());
		}
		else
		{
			mg_http_reply(c, 500, NULL, "%s", "Invalid URI");
		}
	}
}
void HttpServer::WebsocketMessageProcess(struct mg_connection* websocketC, char* message, int len)
{
	return ;
}
void HttpServer::DeviceOnLineState(const struct mg_str& body, std::string& strOut)
{
	std::string jsonStr(body.ptr, body.len);
	rapidjson::Document document;
	document.Parse((char*)jsonStr.c_str());
	if (document.HasParseError())
	{
		strOut = g_msg200jsonerror;
		return;
	}

	std::string cmd = json_check_string(document, "cmd");
	rapidjson::StringBuffer response;
	strOut = std::string(response.GetString(), response.GetSize());
	return;
}
void HttpServer::DeviceChannelList(const struct mg_str& body, std::string& strOut)
{
	std::string jsonStr(body.ptr, body.len);
	rapidjson::Document document;
	document.Parse((char*)jsonStr.c_str());
	if (document.HasParseError())
	{
		strOut = g_msg200jsonerror;
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
	strOut = std::string(response.GetString(), response.GetSize());
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
void HttpServer::StartLive(const struct mg_str& body, std::string& strOut)
{
	std::string jsonStr(body.ptr, body.len);
	rapidjson::Document document;
	document.Parse((char*)jsonStr.c_str());
	if (document.HasParseError())
	{
		strOut = g_msg200jsonerror;
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
			resip::UaMgr::streamStatus smStatus = pUaMgr->getStreamStatus(stream_Id);
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
					tPool.submitTask(std::make_shared<resip::RequestStreamTask>(devuinfo.getDeviceId(), 
						devuinfo.getIp(), devuinfo.getPort(), channel, *pUaMgr, pUaMgr->GetAvailableRtpPort(), 0
						, (UaClientCall*)NULL, "Play"));
				}
				else
				{
					std::cout << "******************* channel:" << channel << " not found" << std::endl;
				}
			}
			else if (smStatus == resip::UaMgr::_UERAGERNT_STREAM_OK)
			{
				//UaClientCall* pCall = pUaMgr->reTurnCallByStreamId(stream_Id);
				//if (pCall)
				//{
				//	if (stream_Id.empty())
				//	{
				//		//stream_Id = std::str_format("%s_%s", pCall->devId.c_str(), channel.c_str());
				//	}
				//}
				isStream = true;
					
			}
			if (smStatus != resip::UaMgr::_UERAGERNT_STREAM_OK)
			{
				for (int i = 0; i < 2 * 10; i++)
				{
					smStatus = pUaMgr->getStreamStatus(stream_Id);
					if (smStatus == resip::UaMgr::_UERAGERNT_STREAM_OK)
					{
						std::this_thread::sleep_for(std::chrono::milliseconds(500));
						//if (HttpIsStreamExist("rtsp", "rtp", stream_Id))
						{
							isStream = true;
							break;
						}
					}
					else if (smStatus == resip::UaMgr::_UERAGERNT_NOT_STREAM)
					{
						pUaMgr->CloseStreamStreamId(stream_Id);
						break;
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
	strOut = std::string(response.GetString(), response.GetSize());
	return;
}
void HttpServer::StopLive(const struct mg_str& body, std::string& strOut)
{
	std::string jsonStr(body.ptr, body.len);
	rapidjson::Document document;
	document.Parse((char*)jsonStr.c_str());
	if (document.HasParseError())
	{
		strOut = g_msg200jsonerror;
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
			pUaMgr->CloseStreamStreamId(stream_Id);
		}
	}
	rapidjson::StringBuffer response;
	rapidjson::Writer<rapidjson::StringBuffer> writer(response);
	writer.StartObject();
	writer.Key("code"); writer.Int(0);
	writer.Key("msg"); writer.String("");
	writer.EndObject();
	strOut = std::string(response.GetString(), response.GetSize());
	return;
}
void HttpServer::StartVod(const struct mg_str& body, std::string& strOut)
{
	std::string jsonStr(body.ptr, body.len);
	rapidjson::Document document;
	document.Parse((char*)jsonStr.c_str());
	if (document.HasParseError())
	{
		strOut = g_msg200jsonerror;
		return;
	}

	std::string wid = json_check_string(document, "wid");
	std::string channel = json_check_string(document, "channel");
	if (channel.empty())
		channel = wid;
	std::string app = json_check_string(document, "app");
	std::string stream_Id = json_check_string(document, "stream");
	std::string deviceId;
	unsigned long stime = (unsigned long)json_check_uint64(document, "startTime");
	unsigned long etime = (unsigned long)json_check_uint64(document, "stopTime");

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
			resip::UaMgr::streamStatus smStatus = pUaMgr->getStreamStatus(stream_Id);
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
						CDateTime staTime(stime);
						CDateTime endTime(etime);
						stream_Id = std::str_format("%s_%s_%s_%s", deviceId.c_str(), channel.c_str(), staTime.tmFormat("%Y-%m-%dT%H:%M:%S").c_str(), endTime.tmFormat("%Y-%m-%dT%H:%M:%S").c_str());
					}
					//判断流存在
					ownThreadPool::myThreadPool& tPool = ownThreadPool::GetThreadPool();
					tPool.submitTask(
						std::make_shared<resip::RequestStreamTask>(devuinfo.getDeviceId(),devuinfo.getIp(), devuinfo.getPort(), channel,
							*pUaMgr, pUaMgr->GetAvailableRtpPort(), 0, (UaClientCall*)NULL, std::string("Playback"), stime, etime)
					);
				}
				else
				{
					std::cout << "******************* channel:" << channel << " not found" << std::endl;
				}
			}
			else if (smStatus == resip::UaMgr::_UERAGERNT_STREAM_OK)
			{
				//UaClientCall* pCall = pUaMgr->reTurnCallByStreamId(stream_Id);
				//if (pCall)
				//{
				//	if (stream_Id.empty())
				//	{
				//		//不可能走到这
				//		//stream_Id = std::str_format("%s_%s", pCall->devId.c_str(), channel.c_str());
				//	}
				//}
				isStream = true;

			}
			if (smStatus != resip::UaMgr::_UERAGERNT_STREAM_OK)
			{
				for (int i = 0; i < 2 * 10; i++)
				{
					smStatus = pUaMgr->getStreamStatus(stream_Id);
					if (smStatus == resip::UaMgr::_UERAGERNT_STREAM_OK)
					{
						std::this_thread::sleep_for(std::chrono::milliseconds(500));
						//if (HttpIsStreamExist("rtsp", "rtp", stream_Id))
						{
							isStream = true;
							break;
						}
					}
					else if (smStatus == resip::UaMgr::_UERAGERNT_NOT_STREAM)
					{
						pUaMgr->CloseStreamStreamId(stream_Id);
						break;
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
	strOut = std::string(response.GetString(), response.GetSize());
	return;
}
void HttpServer::StopVod(const struct mg_str& body, std::string& strOut)
{
	std::string jsonStr(body.ptr, body.len);
	rapidjson::Document document;
	document.Parse((char*)jsonStr.c_str());
	if (document.HasParseError())
	{
		strOut = g_msg200jsonerror;
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
			pUaMgr->CloseStreamStreamId(stream_Id);
		}
	}
	rapidjson::StringBuffer response;
	rapidjson::Writer<rapidjson::StringBuffer> writer(response);
	writer.StartObject();
	writer.Key("code"); writer.Int(0);
	writer.Key("msg"); writer.String("");
	writer.EndObject();
	strOut = std::string(response.GetString(), response.GetSize());
	return;
}
void HttpServer::UserManager(const struct mg_str& body, std::string& strOut)
{
	std::string jsonStr(body.ptr, body.len);
	rapidjson::Document document;
	document.Parse((char*)jsonStr.c_str());
	if (document.HasParseError())
	{
		strOut = g_msg200jsonerror;
		return;
	}

	std::string cmd = json_check_string(document, "cmd");
	rapidjson::StringBuffer response;
	if (strcmp("addUser", cmd.c_str()) == 0)
		WebAddUserAction(document, response);
	else if(strcmp("delUser", cmd.c_str()) == 0)
		WebDelUserAction(document, response);
	else if(strcmp("changeUser", cmd.c_str()) == 0)
		WebChangeUserAction(document, response);
	else
	{
		rapidjson::Writer<rapidjson::StringBuffer> writer(response);
		writer.StartObject();
		writer.Key("code"); writer.Int(-2);
		writer.Key("msg"); writer.String("");
		writer.EndObject();
	}
	strOut = std::string(response.GetString(), response.GetSize());
	return;
}
void HttpServer::zlmHookPublish(const struct mg_str& body, std::string& strOut)
{
	std::string jsonStr(body.ptr, body.len);
	cout << "*******************************\n"
		<< jsonStr << "\n"
		<< "*******************************\n" << endl;
	rapidjson::Document document;
	document.Parse((char*)jsonStr.c_str());
	if (document.HasParseError())
	{
		strOut = g_msg200jsonerror;
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
	strOut = std::string(response.GetString(), response.GetSize());
	return;
}
void HttpServer::zlmHookStreamNoneReader(const struct mg_str& body, std::string& strOut)
{
	std::string jsonStr(body.ptr, body.len);
	cout << "*******************************\n"
		<< jsonStr << "\n"
		<< "*******************************\n" << endl;
	rapidjson::Document document;
	document.Parse((char*)jsonStr.c_str());
	if (document.HasParseError())
	{
		strOut = g_msg200jsonerror;
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
				resip::UaMgr::streamStatus state = pUaMgr->getStreamStatus(strStream);
				if (state == resip::UaMgr::_UERAGERNT_STREAM_OK)
				{
					pUaMgr->CloseStreamStreamId(strStream);
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
	strOut = std::string(response.GetString(), response.GetSize());
	return;
}
void HttpServer::zlmHookStreamChanged(const struct mg_str& body, std::string& strOut)
{
	std::string jsonStr(body.ptr, body.len);
	cout << "*******************************\n"
		<< jsonStr << "\n"
		<< "*******************************\n" << endl;
	rapidjson::Document document;
	document.Parse((char*)jsonStr.c_str());
	if (document.HasParseError())
	{
		strOut = g_msg200jsonerror;
		return;
	}
	std::string strVhost = json_check_string(document, "vhost");
	int iAliveSecond = json_check_int32(document, "aliveSecond");
	std::string strApp = json_check_string(document, "app");
	int iBytesSpeed = json_check_int32(document, "bytesSpeed");
	uint64_t llCreateStamp = json_check_int64(document, "createStamp");
	bool bIsRecordingHLS = json_check_int64(document, "isRecordingHLS");
	bool bIsRecordingMP4 = json_check_int64(document, "isRecordingMP4");
	std::string strMediaServerId = json_check_string(document, "mediaServerId");
	if (document.HasMember("originSock") && !document["originSock"].IsNull())
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
	if (strApp == "rtp")
	{
		sipserver::SipServer* pSvr = GetServer();
		if (pSvr)
		{
			resip::UaMgr* pUaMgr = pSvr->GetUaManager();
			if (pUaMgr)
			{
				UaClientCall::CALL_STATE state = (UaClientCall::CALL_STATE)pUaMgr->getCallStatus(strStream);
				if (bRegist)
				{
					if (UaClientCall::CALL_UAC_CONNECTED == state || UaClientCall::CALL_MEDIA_READY == state)
					{
						pUaMgr->setCallStatus(strStream, UaClientCall::CALL_MY_MEDIA_OK);
					}
				}
				else
				{
						pUaMgr->CloseStreamStreamId(strStream);
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
	strOut = std::string(response.GetString(), response.GetSize());
	return;
}
void HttpServer::zlmHookStreamNotFound(const struct mg_str& body, std::string& strOut)
{
	std::string jsonStr(body.ptr, body.len);
	cout << "*******************************\n"
		<< jsonStr << "\n"
		<< "*******************************\n" << endl;
	rapidjson::Document document;
	document.Parse((char*)jsonStr.c_str());
	if (document.HasParseError())
	{
		strOut = g_msg200jsonerror;
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
	return;
}
void HttpServer::zlmHookSendRtpStopped(const struct mg_str& body, std::string& strOut)
{
	std::string jsonStr(body.ptr, body.len);
	cout << "*******************************\n"
		<< jsonStr << "\n"
		<< "*******************************\n" << endl;
	rapidjson::Document document;
	document.Parse((char*)jsonStr.c_str());
	if (document.HasParseError())
	{
		strOut = g_msg200jsonerror;
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
	//nc->flags |= MG_F_SEND_AND_CLOSE;
	return;
}
