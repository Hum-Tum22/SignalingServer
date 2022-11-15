#ifndef HTTP_H_
#define HTTP_H_
#include <iostream>
#include <mutex>
#include <thread>

extern "C"
{
#include "mongoose.h"
#include "curl/curl.h"
}
using namespace std;

string GetRequest(const string& url);
string PostRequest(const string& url, const string& data);
class CHttpClient
{
public:
    CHttpClient(uint64_t timeout = 15000);
    ~CHttpClient();
    std::string HttpGetRequest(const std::string& url);
    std::string HttpPostRequest(const std::string& url, const std::string& PostData);
    static void OnHttpEvent(struct mg_connection* c, int ev, void* ev_data, void* user_data);

public:
    bool bExitFlag;
    std::string mUrl;// = "http://info.cern.ch/";
    std::string mPostData;// = NULL;
    uint64_t mTimeout_ms;
    std::string mResponse;
};
class HttpServer
{
private:
    static void HandleDefault(struct mg_connection* c, int ev, void* ev_data, void* fn_data);
    static void WebsocketMessageProcess(struct mg_connection* websocketC, char* message, int len);

    static void DeviceOnLineState(const struct mg_str & body, std::string &strOut);
    static void DeviceChannelList(const struct mg_str& body, std::string& strOut);
    static void StartLive(const struct mg_str& body, std::string& strOut);
    static void StopLive(const struct mg_str& body, std::string& strOut);
    static void StartVod(const struct mg_str& body, std::string& strOut);
    static void StopVod(const struct mg_str& body, std::string& strOut);
    static void UserManager(const struct mg_str& body, std::string& strOut);
    static void zlmHookPublish(const struct mg_str& body, std::string& strOut);
    static void zlmHookStreamNoneReader(const struct mg_str& body, std::string& strOut);
    static void zlmHookStreamChanged(const struct mg_str& body, std::string& strOut);
    static void zlmHookStreamNotFound(const struct mg_str& body, std::string& strOut);
    static void zlmHookSendRtpStopped(const struct mg_str& body, std::string& strOut);

    bool init(const string& rootpath = "./", const string& indexfile = "index");
private:
    std::string m_address;
    std::string m_doc_root;
    int m_run_status;

    struct mg_mgr m_mgr;

    thread mTLoop;
public:
	HttpServer(int port = 80);
	~HttpServer();

    void loop();
    static void StartFun(void *data);
};
#endif