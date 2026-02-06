#include "http.h"

#include "device/DeviceManager.h"
#include <iostream>
#include <sstream>
#include "UserAgent/UaClientCall.h"
#include "SipServer.h"
#include "tools/CodeConversion.h"
#include "tools/m_Time.h"
#include "media/mediaIn/JsonStream.h"
#include "deviceMng/JsonDevice.h"

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
	if (url.empty())
		return "";
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
		*(uint64_t*)c->data = mg_millis() + pThis->mTimeout_ms;
	}
	else if (ev == MG_EV_POLL)
	{
		if (mg_millis() > *(uint64_t*)c->data && (c->is_connecting || c->is_resolving))
		{
			mg_error(c, "Connect timeout");
			c->is_draining = 1;
			pThis->bExitFlag = true;
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
		c->is_draining = 1;
		pThis->bExitFlag = true;
	}
	else if (ev == MG_EV_ERROR)
	{
		c->is_draining = 1;
		pThis->bExitFlag = true;
	}
	else if(ev == MG_EV_CLOSE)
	{
		c->is_draining = 1;
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

struct thread_data {
	struct mg_queue queue;  // Worker -> Connection queue
	struct mg_str body;     // Copy of message body
};

static void start_thread(void* (*f)(void*), void* p)
{
#ifdef _WIN32
#define usleep(x) Sleep((x) / 1000)
	_beginthread((void(__cdecl*)(void*)) f, 0, p);
#else
#include <pthread.h>
	pthread_t thread_id = (pthread_t)0;
	pthread_attr_t attr;
	(void)pthread_attr_init(&attr);
	(void)pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_create(&thread_id, &attr, f, p);
	pthread_attr_destroy(&attr);
#endif
}

static void* worker_thread(void* param)
{
	struct thread_data* d = (struct thread_data*)param;
	char buf[100];  // On-stack buffer for the message queue

	mg_queue_init(&d->queue, buf, sizeof(buf));  // Init queue
	usleep(1 * 1000 * 1000);                     // Simulate long execution time

	// Send a response to the connection
	if (d->body.len == 0)
	{
		mg_queue_printf(&d->queue, "Send me POST data");
	}
	else
	{
		uint32_t crc = mg_crc32(0, d->body.ptr, d->body.len);
		mg_queue_printf(&d->queue, "crc32: %#x", crc);
		free((char*)d->body.ptr);
	}

	// Wait until connection reads our message, then it is safe to quit
	while (d->queue.tail != d->queue.head)
	{
		usleep(1000);
	}
	MG_INFO(("done, cleaning up..."));
	free(d);
	return NULL;
}
HttpServer::HttpServer(int port):m_run_status(0)
{
	m_address = std::string("http://0.0.0.0:") + to_string(port);
	mg_mgr_init(&m_mgr);
	m_run_status = 1;
	mTLoop = std::thread(std::bind(&HttpServer::loop, this));
}
HttpServer::~HttpServer()
{
	m_run_status = false;
	if (mTLoop.joinable())
		mTLoop.join();
	mg_mgr_free(&m_mgr);
}
void HttpServer::loop()
{
	mg_http_listen(&m_mgr, m_address.c_str(), HandleDefault, this);  // Setup listener
	while (m_run_status)
	{
		mg_mgr_poll(&m_mgr, 50);
	}
	return ;
}

void WebAddUserAction(rapidjson_sip::Document& document, rapidjson_sip::StringBuffer& response)
{
	int errcode = 0;
	if (document.HasMember("data"))
	{
		rapidjson_sip::Value& msbody = document["data"];
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
	rapidjson_sip::Writer<rapidjson_sip::StringBuffer> writer(response);
	writer.StartObject();
	writer.Key("code"); writer.Int(errcode);
	writer.Key("msg"); writer.String("");
	writer.EndObject();
	return;
}
void WebDelUserAction(rapidjson_sip::Document& document, rapidjson_sip::StringBuffer& response)
{
	int errcode = 0;
	if (document.HasMember("data"))
	{
		rapidjson_sip::Value& msbody = document["data"];
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
	rapidjson_sip::Writer<rapidjson_sip::StringBuffer> writer(response);
	writer.StartObject();
	writer.Key("code"); writer.Int(errcode);
	writer.Key("msg"); writer.String("");
	writer.EndObject();
	return;
}
void WebChangeUserAction(rapidjson_sip::Document& document, rapidjson_sip::StringBuffer& response)
{
	int errcode = 0;
	if (document.HasMember("data"))
	{
		rapidjson_sip::Value& msbody = document["data"];
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
	rapidjson_sip::Writer<rapidjson_sip::StringBuffer> writer(response);
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
		struct mg_str* Hupgrade = mg_http_get_header(hm, "Upgrade");
		if (Hupgrade && strncmp(Hupgrade->ptr, "websocket", Hupgrade->len) == 0)
		{
			mg_ws_upgrade(c, hm, NULL);
			return;
		}
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
	else if (ev == MG_EV_WS_OPEN)
	{
		std::cout << "ws open:" << c << std::endl;
	}
	else if (ev == MG_EV_WS_MSG)
	{
		// Got websocket frame. Received data is wm->data. Echo it back!
		struct mg_ws_message* wm = (struct mg_ws_message*)ev_data;

		//struct thread_data* d = (struct thread_data*)calloc(1, sizeof(*d));
		//d->body = mg_strdup(wm->body);   // Pass received body to the worker
		//start_thread(worker_thread, d);  // Start a thread
		//*(void**)c->data = d;          // Memorise data pointer in c->data

		/*if (wm)
			WebsocketMessage(c, wm->data.ptr, wm->data.len);*/

		//mg_ws_send(c, wm->data.ptr, wm->data.len, WEBSOCKET_OP_TEXT);
	}
	else if (ev == MG_EV_WS_CTL)
	{
		struct mg_ws_message* wm = (struct mg_ws_message*)ev_data;
		auto op = wm->flags & 15;
		if (op == WEBSOCKET_OP_CLOSE)
		{
			printf("------------- ws close-------fn_data:%p\n", c->fn_data);
			//关闭消息 释放内存数据
			//WsStreamInfo* sinfo = (WsStreamInfo*)(*(void**)c->data);
			//if (sinfo)
			//{
			//	//m_run_status = false;
			//	if (sinfo->ms)
			//	{
			//		sinfo->ms->removeReader(sinfo->readhandle);
			//	}
			//	sinfo->readhandle = 0;
			//	if (sinfo->t.joinable())
			//		sinfo->t.join();
			//	CloseStreamByStreamId(sinfo->ms);
			//	free(sinfo);
			//	*(void**)c->data = NULL;
			//}
			
		}
		else if (op == WEBSOCKET_OP_PONG)
		{
			printf("------------- ws pong-------\n");
			//if (c->fn_data) ((mg_per_session_data*)c->fn_data)->tmLastRecvHeart = tmNow;
		}
		std::cout << "ws ctl:" << c << std::endl;
	}
}
void HttpServer::DeviceOnLineState(const struct mg_str& body, std::string& strOut)
{
	std::string jsonStr(body.ptr, body.len);
	rapidjson_sip::Document document;
	document.Parse((char*)jsonStr.c_str());
	if (document.HasParseError())
	{
		strOut = g_msg200jsonerror;
		return;
	}

	std::string cmd = json_check_string(document, "cmd");
	rapidjson_sip::StringBuffer response;
	strOut = std::string(response.GetString(), response.GetSize());
	return;
}
void HttpServer::DeviceChannelList(const struct mg_str& body, std::string& strOut)
{
	std::string jsonStr(body.ptr, body.len);
	rapidjson_sip::Document document;
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
	rapidjson_sip::StringBuffer response;
	rapidjson_sip::Writer<rapidjson_sip::StringBuffer> writer(response);
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
		rapidjson_sip::Document document;
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
		rapidjson_sip::Document document;
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
					rapidjson_sip::Value& msbody = document["data"][0];
					rapidjson_sip::StringBuffer sbBuf;
					rapidjson_sip::Writer<rapidjson_sip::StringBuffer> jWriter(sbBuf);
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
	rapidjson_sip::Document document;
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

	rapidjson_sip::StringBuffer response;
	rapidjson_sip::Writer<rapidjson_sip::StringBuffer> writer(response);
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
				//IDeviceMngrSvr& DevMgr = GetIDeviceMngr();
				//SipServerDeviceInfo devuinfo;
				//list<GBDeviceChannel> chlist = DevMgr.GetGBDeviceMapper().GetGBDeviceChannelMapper().queryChannelByChannelId(channel);
				//for (auto& it : chlist)
				//{
				//	if (it.getStatus())
				//	{
				//		devuinfo = DevMgr.GetGBDeviceMapper().getDeviceByDeviceId(it.getDeviceId());
				//		break;
				//	}
				//}
				//if (!devuinfo.getDeviceId().empty())
				//{
				//	deviceId = devuinfo.getDeviceId();
				//	if (stream_Id.empty())
				//	{
				//		//stream_Id = std::str_format("%s_%s", deviceId.c_str(), channel.c_str());
				//	}
				//	//判断流存在
				//	ownThreadPool::myThreadPool& tPool = ownThreadPool::GetThreadPool();
				//	tPool.submitTask(std::make_shared<resip::RequestStreamTask>(devuinfo.getDeviceId(), 
				//		devuinfo.getIp(), devuinfo.getPort(), channel, *pUaMgr, pUaMgr->GetAvailableRtpPort(), 0
				//		, (UaClientCall*)NULL, "Play"));
				//}
				//else
				//{
				//	std::cout << "******************* channel:" << channel << " not found" << std::endl;
				//}
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
			//IDeviceMngrSvr& DevMgr = GetIDeviceMngr();
			//SipServerDeviceInfo devuinfo;
			//list<GBDeviceChannel> chlist = DevMgr.GetGBDeviceMapper().GetGBDeviceChannelMapper().queryChannelByChannelId(channel);
			//for (auto& it : chlist)
			//{
			//	if (it.getStatus())
			//	{
			//		devuinfo = DevMgr.GetGBDeviceMapper().getDeviceByDeviceId(it.getDeviceId());
			//		break;
			//	}
			//}
			//if (!devuinfo.getDeviceId().empty())
			//{
			//	deviceId = devuinfo.getDeviceId();
			//	if (stream_Id.empty())
			//	{
			//		//stream_Id = std::str_format("%s_%s", deviceId.c_str(), channel.c_str());
			//	}
			//}
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
			std::string url;// = std::str_format("http://%s:%d/rtp/%s.live.mp4", pSvr->zlmHost.c_str(), pSvr->zlmHttpPort, stream_Id.c_str());
			writer.Key("fmp4"); writer.String(url.c_str());
			std::string rtcUrl;// =
				//std::str_format("https://%s:443/index/api/webrtc?app=rtp&stream=%s&type=play",
					//pSvr->zlmHost.c_str(), stream_Id.c_str());
			writer.Key("rtc"); writer.String(rtcUrl.c_str());
			//writer.Key("mediainfo"); writer.String(GetMediaConfigInfo().c_str());
			std::string mediaInfo = GetMediaConfigInfo();
			writer.Key("mediainfo"); writer.RawValue(mediaInfo.c_str(), mediaInfo.size(), rapidjson_sip::kObjectType);
			
				
			writer.EndObject();
		writer.EndObject();
	}
	else
	{
		writer.StartObject();
		writer.Key("code"); writer.Int(1);
		writer.Key("msg"); writer.String(""/*std::GbkToUtf8("播放失败").c_str()*/);
		writer.EndObject();
	}
	strOut = std::string(response.GetString(), response.GetSize());
	return;
}
void HttpServer::StopLive(const struct mg_str& body, std::string& strOut)
{
	std::string jsonStr(body.ptr, body.len);
	rapidjson_sip::Document document;
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
	rapidjson_sip::StringBuffer response;
	rapidjson_sip::Writer<rapidjson_sip::StringBuffer> writer(response);
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
	rapidjson_sip::Document document;
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

	rapidjson_sip::StringBuffer response;
	rapidjson_sip::Writer<rapidjson_sip::StringBuffer> writer(response);
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
				//IDeviceMngrSvr& DevMgr = GetIDeviceMngr();
				//SipServerDeviceInfo devuinfo;
				//list<GBDeviceChannel> chlist = DevMgr.GetGBDeviceMapper().GetGBDeviceChannelMapper().queryChannelByChannelId(channel);
				//for (auto& it : chlist)
				//{
				//	if (it.getStatus())
				//	{
				//		devuinfo = DevMgr.GetGBDeviceMapper().getDeviceByDeviceId(it.getDeviceId());
				//		break;
				//	}
				//}
				//if (!devuinfo.getDeviceId().empty())
				//{
				//	deviceId = devuinfo.getDeviceId();
				//	if (stream_Id.empty())
				//	{
				//		CDateTime staTime(stime);
				//		CDateTime endTime(etime);
				//		//stream_Id = std::str_format("%s_%s_%s_%s", deviceId.c_str(), channel.c_str(), staTime.tmFormat("%Y-%m-%dT%H:%M:%S").c_str(), endTime.tmFormat("%Y-%m-%dT%H:%M:%S").c_str());
				//	}
				//	//判断流存在
				//	ownThreadPool::myThreadPool& tPool = ownThreadPool::GetThreadPool();
				//	tPool.submitTask(
				//		std::make_shared<resip::RequestStreamTask>(devuinfo.getDeviceId(),devuinfo.getIp(), devuinfo.getPort(), channel,
				//			*pUaMgr, pUaMgr->GetAvailableRtpPort(), 0, (UaClientCall*)NULL, std::string("Playback"), stime, etime)
				//	);
				//}
				//else
				//{
				//	std::cout << "******************* channel:" << channel << " not found" << std::endl;
				//}
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
			//IDeviceMngrSvr& DevMgr = GetIDeviceMngr();
			//SipServerDeviceInfo devuinfo;
			//list<GBDeviceChannel> chlist = DevMgr.GetGBDeviceMapper().GetGBDeviceChannelMapper().queryChannelByChannelId(channel);
			//for (auto& it : chlist)
			//{
			//	if (it.getStatus())
			//	{
			//		devuinfo = DevMgr.GetGBDeviceMapper().getDeviceByDeviceId(it.getDeviceId());
			//		break;
			//	}
			//}
			//if (!devuinfo.getDeviceId().empty())
			//{
			//	deviceId = devuinfo.getDeviceId();
			//	if (stream_Id.empty())
			//	{
			//		//stream_Id = std::str_format("%s_%s", deviceId.c_str(), channel.c_str());
			//	}
			//}
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
		std::string url;// = std::str_format("http://%s:%d/rtp/%s.live.mp4", pSvr->zlmHost.c_str(), pSvr->zlmHttpPort, stream_Id.c_str());
		writer.Key("fmp4"); writer.String(url.c_str());
		std::string rtcUrl;// =
			//std::str_format("https://%s:443/index/api/webrtc?app=rtp&stream=%s&type=play",
				//pSvr->zlmHost.c_str(), stream_Id.c_str());
		writer.Key("rtc"); writer.String(rtcUrl.c_str());
		//writer.Key("mediainfo"); writer.String(GetMediaConfigInfo().c_str());
		std::string mediaInfo = GetMediaConfigInfo();
		writer.Key("mediainfo"); writer.RawValue(mediaInfo.c_str(), mediaInfo.size(), rapidjson_sip::kObjectType);

		writer.EndObject();
		writer.EndObject();
	}
	else
	{
		writer.StartObject();
		writer.Key("code"); writer.Int(1);
		writer.Key("msg"); writer.String(""/*std::GbkToUtf8("播放失败").c_str()*/);
		writer.EndObject();
	}
	strOut = std::string(response.GetString(), response.GetSize());
	return;
}
void HttpServer::StopVod(const struct mg_str& body, std::string& strOut)
{
	std::string jsonStr(body.ptr, body.len);
	rapidjson_sip::Document document;
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
	rapidjson_sip::StringBuffer response;
	rapidjson_sip::Writer<rapidjson_sip::StringBuffer> writer(response);
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
	rapidjson_sip::Document document;
	document.Parse((char*)jsonStr.c_str());
	if (document.HasParseError())
	{
		strOut = g_msg200jsonerror;
		return;
	}

	std::string cmd = json_check_string(document, "cmd");
	rapidjson_sip::StringBuffer response;
	if (strcmp("addUser", cmd.c_str()) == 0)
		WebAddUserAction(document, response);
	else if(strcmp("delUser", cmd.c_str()) == 0)
		WebDelUserAction(document, response);
	else if(strcmp("changeUser", cmd.c_str()) == 0)
		WebChangeUserAction(document, response);
	else
	{
		rapidjson_sip::Writer<rapidjson_sip::StringBuffer> writer(response);
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
	rapidjson_sip::Document document;
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
		
	rapidjson_sip::StringBuffer response;
	rapidjson_sip::Writer<rapidjson_sip::StringBuffer> writer(response);
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
	rapidjson_sip::Document document;
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

	rapidjson_sip::StringBuffer response;
	rapidjson_sip::Writer<rapidjson_sip::StringBuffer> writer(response);
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
	rapidjson_sip::Document document;
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
		rapidjson_sip::Value& msbody = document["originSock"];
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
		rapidjson_sip::Value& msbody = document["tracks"];
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
	rapidjson_sip::StringBuffer response;
	rapidjson_sip::Writer<rapidjson_sip::StringBuffer> writer(response);
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
	rapidjson_sip::Document document;
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
	rapidjson_sip::Document document;
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

//int HttpServer::WebsocketMessage(struct mg_connection* c, const char* message, size_t len)
//{
//	rapidjson_sip::Document document;
//	rapidjson_sip::ParseResult res = document.Parse(message, len);
//	if (document.HasParseError())
//	{
//		rapidjson_sip::StringBuffer response;
//		CommonResponseParamToJson("CommonError", -1, 6, response);
//		mg_ws_send(c, response.GetString(), response.GetSize(), WEBSOCKET_OP_TEXT);
//		return 0;
//	}
//	printf("ws msg:%s\n", message);
//
//	std::string classe = json_check_string(document, "classe");
//	std::string strCmd = json_check_string(document, "cmd");
//
//	//pWsTaskInfo->nCmd = 0;
//	if (strcmp(webs_Play_class, classe.c_str()) == 0)
//	{
//		if (strcmp(web_RealTime_method, strCmd.c_str()) == 0)
//		{
//			rapidjson_sip::StringBuffer response;
//			rapidjson_sip::Writer<rapidjson_sip::StringBuffer> writer(response);
//			auto sinfo = new WsStreamInfo();
//			sinfo->IsLive = 0;
//			sinfo->c = c;
//			int nRet = RealTimePlayAction(sinfo, document, writer);
//			*(void**)c->data = sinfo;
//
//			mg_ws_send(c, response.GetString(), response.GetSize(), WEBSOCKET_OP_TEXT);
//			if(nRet == 0)
//				sinfo->t = std::move(std::thread(&HttpServer::RtPreviewThread, sinfo));
//
//			/*SendBufferNoPre(wsi_in, response.GetString(), response.GetSize(), 1);
//			lws_callback_on_writable(wsi_in);
//			if (nRet == 0)
//			{
//				std::thread t(std::bind(&WsStreamMng::RtPreviewThread, this, cnPtr));
//				t.detach();
//			}*/
//
//		}
//		else if (strcmp(webs_PlayBack_method, strCmd.c_str()) == 0)
//		{
//			rapidjson_sip::StringBuffer response;
//			rapidjson_sip::Writer<rapidjson_sip::StringBuffer> writer(response);
//			auto cnPtr = new WsStreamInfo();
//			cnPtr->IsLive = 1;
//			cnPtr->c = c;
//			int nRet = PlayBackAction(cnPtr, document, writer);
//			*(void**)c->data = cnPtr;
//
//			/*SendBufferNoPre(wsi_in, response.GetString(), response.GetSize(), 1);
//			lws_callback_on_writable(wsi_in);
//			if (nRet == 0)
//			{
//				std::thread t(std::bind(&WsStreamMng::PlayBackThread, this, cnPtr));
//				t.detach();
//			}*/
//			mg_ws_send(c, response.GetString(), response.GetSize(), WEBSOCKET_OP_TEXT);
//		}
//		else if (strcmp(webs_PBControl_method, strCmd.c_str()) == 0)
//		{
//			rapidjson_sip::StringBuffer response;
//			rapidjson_sip::Writer<rapidjson_sip::StringBuffer> writer(response);
//			auto cnPtr = (WsStreamInfo*)(*(void**)c->data);
//			if(cnPtr)
//				PlayBackControlAction(cnPtr, document, writer);
//			//SendBufferNoPre(wsi_in, response.GetString(), response.GetSize(), 1);
//			//lws_callback_on_writable(wsi_in);
//			mg_ws_send(c, response.GetString(), response.GetSize(), WEBSOCKET_OP_TEXT);
//		}
//		else
//		{
//			rapidjson_sip::StringBuffer response;
//			rapidjson_sip::Writer<rapidjson_sip::StringBuffer> writer(response);
//			writer.StartObject();
//			writer.Key("classe"); writer.String(webs_Play_class);
//			writer.Key("method"); writer.String(strCmd.c_str());
//			writer.Key("errorcode"); writer.Int(-1);
//			//writer.Key("msg");writer.String(ErrStr(_ER_INVALID_CMD));
//			writer.EndObject();
//			mg_ws_send(c, response.GetString(), response.GetSize(), WEBSOCKET_OP_TEXT);
//		}
//	}
//	else if (strcmp(webs_Download_class, classe.c_str()) == 0)
//	{
//		if (strcmp(webs_Download_Raw_method, strCmd.c_str()) == 0)
//		{
//			rapidjson_sip::StringBuffer response;
//			rapidjson_sip::Writer<rapidjson_sip::StringBuffer> writer(response);
//			//DownloadAction(connectinfo, document, writer, NULL);
//			mg_ws_send(c, response.GetString(), response.GetSize(), WEBSOCKET_OP_TEXT);
//		}
//		else
//		{
//			rapidjson_sip::StringBuffer response;
//			rapidjson_sip::Writer<rapidjson_sip::StringBuffer> writer(response);
//			writer.StartObject();
//			writer.Key("classe"); writer.String(webs_Play_class);
//			writer.Key("method"); writer.String(strCmd.c_str());
//			writer.Key("errorcode"); writer.Int(-1);
//			//writer.Key("msg");writer.String(ErrStr(_ER_INVALID_CMD));
//			writer.EndObject();
//			mg_ws_send(c, response.GetString(), response.GetSize(), WEBSOCKET_OP_TEXT);
//		}
//	}
//	else
//	{
//		const static char* pResult = "Does not support class";
//		rapidjson_sip::StringBuffer response;
//		rapidjson_sip::Writer<rapidjson_sip::StringBuffer> writer(response);
//		writer.StartObject();
//		writer.Key("errorcode"); writer.Int(-1);
//		writer.Key("msg"); writer.String(pResult);
//		writer.EndObject();
//		mg_ws_send(c, response.GetString(), response.GetSize(), WEBSOCKET_OP_TEXT);
//	}
//	return 0;
//}
//int HttpServer::RealTimePlayAction(WsStreamInfo* sinfo, rapidjson_sip::Document& document, rapidjson_sip::Writer<rapidjson_sip::StringBuffer>& writer)
//{
//	int errcode = 0;
//	if (document.HasMember("params"))
//	{
//		//{"streamType":0,"streamNo":0,"handle":5,}
//		rapidjson_sip::Value& msbody = document["params"];
//		unsigned int chl_handle = json_check_int32(msbody, "handle");
//		int streamtype = json_check_uint32(msbody, "streamType");
//		int streamno = json_check_int32(msbody, "streamno");
//
//
//		MediaStream::Ptr s = MediaMng::GetInstance().findStream("37028806251320111520");
//		if (s)
//		{
//			s->increasing();
//			sinfo->ms = s;
//			sinfo->readhandle = s->createReader();
//
//			errcode = 0;
//			writer.StartObject();
//			writer.Key("classe"); writer.String(webs_Play_class);
//			writer.Key("method"); writer.String(web_RealTime_method);
//			writer.Key("errorcode"); writer.Int(errcode);
//			writer.Key("msg"); writer.String("");
//			writer.EndObject();
//			return errcode;
//		}
//		else
//		{
//			s = StartLiveStream("37028806251320111520", 0);
//			if (s)
//			{
//				s->increasing();
//				sinfo->ms = s;
//				sinfo->readhandle = s->createReader();
//
//				errcode = 0;
//				writer.StartObject();
//				writer.Key("classe"); writer.String(webs_Play_class);
//				writer.Key("method"); writer.String(web_RealTime_method);
//				writer.Key("errorcode"); writer.Int(errcode);
//				writer.Key("msg"); writer.String("");
//				writer.EndObject();
//				return errcode;
//			}
//			else
//			{
//				errcode = -2;
//				writer.StartObject();
//				writer.Key("classe"); writer.String(webs_Play_class);
//				writer.Key("method"); writer.String(web_RealTime_method);
//				writer.Key("errorcode"); writer.Int(errcode);
//				writer.Key("playhandle"); writer.Int(0);
//				//writer.Key("msg");writer.String(ErrStr(errcode));
//				//writer.Key("fps");writer.Int(videoparam.codecMap[0].fps);
//				writer.EndObject();
//				return errcode;
//			}
//		}
//	}
//	else
//	{
//		errcode = 6;
//		writer.StartObject();
//		writer.Key("classe"); writer.String(webs_Play_class);
//		writer.Key("method"); writer.String(web_RealTime_method);
//		writer.Key("errorcode"); writer.Int(errcode);
//		writer.Key("msg"); writer.String("Parser Json error!");
//		//writer.Key("fps");writer.Int(videoparam.codecMap[0].fps);
//		writer.EndObject();
//	}
//	return errcode;
//}
//int HttpServer::PlayBackAction(WsStreamInfo* sinfo, rapidjson_sip::Document& document, rapidjson_sip::Writer<rapidjson_sip::StringBuffer>& writer)
//{
//	int errcode = 0;
//	unsigned int readhandle = 0;
//	bool playok = false;
//	if (document.HasMember("params"))
//	{
//		//{"starttime":1,"endtime":0,"handle":0}}
//		rapidjson_sip::Value& msbody = document["params"];
//		unsigned int chl_handle = json_check_uint32(msbody, "handle");
//		uint64_t starttime = json_check_uint64(msbody, "starttime");
//		uint64_t endtime = json_check_uint64(msbody, "endtime");
//		int filetype = json_check_int32(msbody, "fileType");
//		uint64_t pointtime = json_check_uint64(msbody, "pointTime");
//		std::string uuid = json_check_string(msbody, "uuid");
//		if (pointtime == 0)
//			pointtime = starttime;
//
//		//smInfo->IsLive = 1;
//		//int fPlayFindStatus = C_PlayByTime(starttime, endtime, chl_handle, 0, smInfo->fPlayHandle);
//		////int fPlayFindStatus = playbackmrg::Instance()->PlayByTime(starttime, endtime, chl_handle, 0, smInfo->fPlayHandle, 0);
//		//if (fPlayFindStatus != _PB_OK)
//		//{
//		//	errcode = ERR_PLAY_BACK_FAILED;
//		//}
//		//else
//		//{
//		//	//int res = C_SetTimePos(smInfo->fPlayHandle, pointtime);
//		//	playok = true;
//		//}
//	}
//	else
//	{
//		errcode = 6;
//	}
//	if (playok)
//	{
//		writer.StartObject();
//		writer.Key("classe"); writer.String(webs_Play_class);
//		writer.Key("method"); writer.String(webs_PlayBack_method);
//		writer.Key("errorcode"); writer.Int(errcode);
//		writer.Key("playhandle"); writer.Int(0);
//		//writer.Key("msg");writer.String(ErrStr(errcode));
//		writer.EndObject();
//		return 0;
//	}
//	else
//	{
//		writer.StartObject();
//		writer.Key("classe"); writer.String(webs_Play_class);
//		writer.Key("method"); writer.String(webs_PlayBack_method);
//		writer.Key("errorcode"); writer.Int(errcode);
//		//writer.Key("msg");writer.String(ErrStr(errcode));
//		writer.EndObject();
//	}
//	return errcode;
//}
//int HttpServer::PlayBackControlAction(WsStreamInfo* sinfo, rapidjson_sip::Document& document, rapidjson_sip::Writer<rapidjson_sip::StringBuffer>& writer)
//{
//	int errcode = 0;
//	unsigned int readhandle;
//	int streamno = 0;
//	bool playok = false;
//	if (document.HasMember("params"))
//	{
//		//回放控制类型：ctrlType 0：开始1：暂停2：倍速3：拖动
//		//"params":{"ctrlType":1,"speed":0.5,"pointTime":1631868217,"handle":2}
//		rapidjson_sip::Value& msbody = document["params"];
//		int ctrlType = json_check_int32(msbody, "ctrlType");
//		float speed = json_check_float(msbody, "speed");
//		uint64_t pointTime = json_check_uint32(msbody, "pointTime");
//		std::string uuid = json_check_string(msbody, "uuid");
//		uint32_t handle = json_check_uint32(msbody, "handle");
//
//		if (sinfo->IsLive != 1)
//		{
//		}
//		else
//		{
//			if (ctrlType == 0)
//			{
//				sinfo->speed = 1;
//			}
//			else if (ctrlType == 1)
//			{
//				//connectinfo->speed = 0xffffffff;
//				sinfo->speed = 0;
//				//connectinfo->prevTime = 0xFFFFFFFFFFFFF
//			}
//			else if (ctrlType == 2)
//			{
//				sinfo->speed = speed;
//			}
//			else if (ctrlType == 3)
//			{
//				//C_SetTimePos(connectinfo->fPlayHandle, pointTime);
//			}
//			else
//			{
//				errcode = -1;
//			}
//		}
//		writer.StartObject();
//		writer.Key("classe"); writer.String(webs_Play_class);
//		writer.Key("method"); writer.String(webs_PBControl_method);
//		writer.Key("errorcode"); writer.Int(errcode);
//		//writer.Key("msg");writer.String(ErrStr(errcode));
//		writer.EndObject();
//	}
//	else
//	{
//		errcode = 6;
//		writer.StartObject();
//		writer.Key("classe"); writer.String(webs_Play_class);
//		writer.Key("method"); writer.String(webs_PBControl_method);
//		writer.Key("errorcode"); writer.Int(errcode);
//		//writer.Key("msg");writer.String(ErrStr(errcode));
//		writer.EndObject();
//	}
//	return errcode;
//}
//MediaStream::Ptr HttpServer::StartLiveStream(std::string channelId, int streamId)
//{
//	BaseDevice::Ptr parentDev = NULL;
//	BaseChildDevice* childDev = NULL;
//	if (!channelId.empty())
//	{
//		childDev = DeviceMng::Instance().findChildDevice(channelId);
//		if (childDev)
//		{
//			parentDev = childDev->getParentDev();
//		}
//		else
//		{
//			printf("%s child device not found\n", channelId.c_str());
//			return NULL;
//		}
//	}
//	else
//	{
//		printf("channelId is null\n");
//		return NULL;
//	}
//
//	if (parentDev)
//	{
//		if (parentDev->devType == BaseDevice::JSON_NVR)
//		{
//			JsonNvrDevic::Ptr Nvr = std::dynamic_pointer_cast<JsonNvrDevic>(parentDev);
//			if (Nvr)
//			{
//				int err = 0, chl = -1;
//				uint32_t msgSize = 1024 * 8 * 3;
//				char* Buffer = new char[msgSize];
//				Nvr->Dev_ListIPC(Buffer, msgSize, err);
//				if (err == 0)
//				{
//					rapidjson_sip::Document document;
//					document.Parse(Buffer);
//					if (!document.HasParseError())
//					{
//						if (document.HasMember("ipc_list") && document["ipc_list"].IsArray())
//						{
//							rapidjson_sip::Value& body = document["ipc_list"];
//							for (uint32_t i = 0; i < body.Size(); i++)
//							{
//								std::string devNum = json_check_string(body[i], "device_number");
//								JsonChildDevic* pChild = dynamic_cast<JsonChildDevic*>(childDev);
//								if (devNum == pChild->getName())
//								{
//									int enable_flag = json_check_int32(body[i], "enable_flag");
//									if (enable_flag == 2)
//									{
//										int child = json_check_int32(body[i], "chid");
//										int online = json_check_int32(body[i], "online_status");
//										if (online == 1)
//										{
//											std::string Id = json_check_string(body[i], "device_id");
//											chl = child;
//											pChild->setStatus(1);
//										}
//										else
//										{
//											pChild->setStatus(0);
//											printf("%s %s child device offline\n", channelId.c_str(), devNum.c_str());
//										}
//									}
//									else
//									{
//										printf("%s %s child device enable_flag :%d\n", channelId.c_str(), devNum.c_str(), enable_flag);
//									}
//									break;
//								}
//							}
//						}
//					}
//				}
//				else
//				{
//					printf("%s get channel info err: %d\n", channelId.c_str(), err);
//				}
//				delete Buffer; Buffer = NULL;
//				if (chl >= 0)
//				{
//					JsonStream::Ptr streamIn = std::make_shared<JsonStream>(channelId.c_str(), channelId.c_str());
//					ULHandle playhandle = Nvr->Dev_Preview(chl, streamId, (void*)JsonStream::DataPlayCallBack, (void*)streamIn.get(), err);
//					if (err == 0)
//					{
//						printf("%s child device pull stream ok\n", channelId.c_str());
//						streamIn->setStreamHandle(playhandle);
//						MediaMng::GetInstance().addStream(streamIn);
//						return streamIn;
//					}
//					else
//					{
//						printf("%s child device pull stream failed err:%d\n", channelId.c_str(), err);
//						return NULL;
//					}
//				}
//				else
//				{
//					printf("%s child device channel not found\n", channelId.c_str());
//				}
//			}
//		}
//	}
//	return NULL;
//}
//void HttpServer::RtPreviewThread(WsStreamInfo* smInfo)
//{
//	if (smInfo == NULL)
//		return;
//
//	uint64_t m_clock = 0;
//	int mGap = 40;
//	uint64_t frameNum = 0;
//
//	uint8_t* pBuffer = (uint8_t*)malloc(1024 * 1024);
//	int offset = 0;
//	pthread_setname_np(pthread_self(), "PreviewThread");
//	while (smInfo->readhandle > 0)
//	{
//		int64_t clock = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
//		if (0 == m_clock || m_clock < clock)
//		{
//			vframe_t frame;
//			if (smInfo->ms && 0 == smInfo->ms->GetNextFrame(smInfo->readhandle, frame))
//			{
//				if (!frame.nalu)
//					continue;
//				if (0 == m_clock)
//					m_clock = clock;
//				if (frame.nalu && frame.nalu[0] != 0x00)
//				{
//					printf("data err %x,%x,%x,%x,nalu:%d\n", frame.nalu[0], frame.nalu[1], frame.nalu[2], frame.nalu[3], (frame.nalu[4] & 0x1F));
//					continue;
//				}
//				if (frame.frameType == GB_CODEC_H264 || frame.frameType == GB_CODEC_H265)
//				{
//					frameNum++;
//				}
//				if (frame.idr)
//				{
//					//mGap = frame.gap;
//					printf("--------------- idr frame time:%ld, gap:%d, frameNum:%ju\n", time(0), mGap, frameNum);
//				}
//
//				int type = 0, nfps = 1000 / mGap;
//				short auChl = 0, auRate = 0;
//				uint64_t pts = clock;
//				if (frame.frameType == GB_CODEC_H264)
//				{
//					type = 0;
//				}
//				else if (frame.frameType == GB_CODEC_H265)
//				{
//					type = 1;
//				}
//				else
//				{
//					/*if (frame.frameType == PT_AAC)
//					{
//						type = 37;
//					}
//					else if (frame.frameType == PT_PCMA)
//					{
//						type = 19;
//					}
//					else if (frame.frameType == PT_PCMU)
//					{
//						type = 20;
//					}
//					else
//					{
//						type = frame.frameType;
//					}*/
//					auChl = 1;// pFrame->GetChls();
//					//auRate = pFrame->GetFramRate() / 1000;
//					auRate = auRate == 0 ? 8000 : auRate;
//				}
//				type = htonl(type);
//				pts = (((uint64_t)htonl(pts)) << 32) + htonl(pts >> 32);;
//				auChl = htons(auChl);
//				auRate = htons(auRate);
//				nfps = htonl(nfps);
//
//				offset = 0;
//				memcpy(pBuffer + offset, (unsigned char*)&type, sizeof(type));
//				offset += sizeof(type);
//
//				memcpy(pBuffer + offset, (unsigned char*)&pts, sizeof(pts));
//				offset += sizeof(pts);
//
//				memcpy(pBuffer + offset, (unsigned char*)&auChl, sizeof(auChl));
//				offset += sizeof(auChl);
//
//				memcpy(pBuffer + offset, (unsigned char*)&auRate, sizeof(auRate));
//				offset += sizeof(auRate);
//
//				memcpy(pBuffer + offset, frame.nalu, frame.bytes);
//				offset += frame.bytes;
//
//				size_t len = mg_ws_send(smInfo->c, pBuffer, offset, WEBSOCKET_OP_BINARY);
//				if (len < offset)
//				{
//					std::this_thread::sleep_for(std::chrono::milliseconds(2));
//					mg_ws_send(smInfo->c, pBuffer + len, offset - len, WEBSOCKET_OP_BINARY);
//				}
//				m_clock += mGap;
//
//			}
//		}
//		std::this_thread::sleep_for(std::chrono::milliseconds(5));
//	}
//	free(pBuffer);
//	printf("------------------- Preview exit -------ms info :%p\n", smInfo);
//}
//bool HttpServer::CloseStreamByStreamId(MediaStream::Ptr& ms)
//{
//	if (ms)
//	{
//		ms->reduction();
//		printf("xxxxxxxxxxxxxxxxx close stream:%s ref:%d\n", ms->getStreamId().c_str(), ms->refNum());
//		if (ms->refNum() == 0)
//		{
//			BaseChildDevice* childDev = DeviceMng::Instance().findChildDevice(ms->getDeviceId());
//			if (childDev)
//			{
//				BaseDevice::Ptr parentDev = childDev->getParentDev();
//				if (parentDev && parentDev->devType == BaseDevice::JSON_NVR)
//				{
//					int err = 0;
//					parentDev->Dev_StopPreview(ms->getStreamHandle(), err);
//					if (err == 0)
//					{
//						MediaMng::GetInstance().removeStream(ms->getStreamId());
//						printf("remove stream:%s\n", ms->getStreamId().c_str());
//					}
//					else
//					{
//						printf("stop stream:%s, failed:%d\n", ms->getStreamId().c_str(), err);
//					}
//				}
//			}
//		}
//	}
//	return true;
//}