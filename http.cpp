#include "http.h"

#include "myjsondef.h"
#include "writer.h"
#include "stringbuffer.h"
#include "document.h"

#include "device/DeviceManager.h"
#include <iostream>

using namespace std;
#define HTTP_TIMEOUT 10
size_t getUrlResponse(char* buffer, size_t size, size_t count, string* response)
{
	size_t recv_size = size * count;
	response->clear();
	response->append(buffer);
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
size_t curlWriteCallback(char* buff, size_t size, size_t nmemb, string* response) {
	response->clear();
	response->append(buff);
	return size * nmemb;
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
		rapidjson::StringBuffer response;
		rapidjson::Writer<rapidjson::StringBuffer> writer(response);
		writer.StartObject();
		writer.Key("code"); writer.Int(0);
		writer.Key("msg"); writer.String("");
		writer.Key("data");
		writer.StartArray();

		writer.Key("channel"); writer.String("34020000001320000001");
		writer.Key("description"); writer.String("");
		writer.Key("name"); writer.String("ipc1");

		/*writer.Key("channel"); writer.String("");
		writer.Key("description"); writer.String("");
		writer.Key("name"); writer.String("");*/

		writer.EndArray();
		writer.EndObject();
		
		mg_printf(nc, g_msg200Ok_msg, response.GetSize(), response.GetString());
		nc->flags |= MG_F_SEND_AND_CLOSE;
	}
	return;
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
		//rapidjson::Document document;
		//document.Parse((char*)jsonStr.c_str());
		//if (document.HasParseError())
		//{
		//	LogOut("Web", L_ERROR, "Json: parse error", jsonStr.c_str());
		//	mg_printf(nc, g_msg200jsonerror);
		//	nc->flags |= MG_F_SEND_AND_CLOSE;
		//	return;
		//}

		//std::string cmd = json_check_string(document, "cmd");
		//LogOut("Web", L_INFO, "cmd:%s", cmd.c_str());
		//rapidjson::StringBuffer response;
		//if (strcmp(cmdGetPreviewUrl, cmd.c_str()) == 0)
		//	WebGetPreviewUrlAction(nc, document, response, userInfo);
		////else if(strcmp(cmdGetPlaybackUrl, cmd.c_str()) == 0)
		////    WebIpcPresetCtrlAction(nc, document, response, userInfo);
		////else if(strcmp(cmdPlaybackCtrl, cmd.c_str()) == 0)
		////    WebIpcCruiseCtrlAction(nc, document, response, userInfo);
		//else
		//	CommonResponseParamToJson(cmd.c_str(), -1, _ER_INVALID_CMD, response);
		//LogOut("Web", L_INFO, "Json response:%s", response.GetString());
		//mg_printf(nc, g_msg200Ok_msg, response.GetSize(), response.GetString());
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
		//rapidjson::Document document;
		//document.Parse((char*)jsonStr.c_str());
		//if (document.HasParseError())
		//{
		//	LogOut("Web", L_ERROR, "Json: parse error", jsonStr.c_str());
		//	mg_printf(nc, g_msg200jsonerror);
		//	nc->flags |= MG_F_SEND_AND_CLOSE;
		//	return;
		//}

		//std::string cmd = json_check_string(document, "cmd");
		//LogOut("Web", L_INFO, "cmd:%s", cmd.c_str());
		//rapidjson::StringBuffer response;
		//if (strcmp(cmdGetPreviewUrl, cmd.c_str()) == 0)
		//	WebGetPreviewUrlAction(nc, document, response, userInfo);
		////else if(strcmp(cmdGetPlaybackUrl, cmd.c_str()) == 0)
		////    WebIpcPresetCtrlAction(nc, document, response, userInfo);
		////else if(strcmp(cmdPlaybackCtrl, cmd.c_str()) == 0)
		////    WebIpcCruiseCtrlAction(nc, document, response, userInfo);
		//else
		//	CommonResponseParamToJson(cmd.c_str(), -1, _ER_INVALID_CMD, response);
		//LogOut("Web", L_INFO, "Json response:%s", response.GetString());
		//mg_printf(nc, g_msg200Ok_msg, response.GetSize(), response.GetString());
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