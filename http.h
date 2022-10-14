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

class HttpServer
{
private:
    static void HandleDefault(struct mg_connection* nc, int ev, void* ev_data);
    static void WebsocketMessageProcess(struct mg_connection* websocketC, char* message, int len);

    static void DeviceOnLineState(struct mg_connection* nc, int ev, void* ev_data);
    static void DeviceChannelList(struct mg_connection* nc, int ev, void* ev_data);
    static void StartLive(struct mg_connection* nc, int ev, void* ev_data);
    static void StopLive(struct mg_connection* nc, int ev, void* ev_data);
    static void UserManager(struct mg_connection* nc, int ev, void* ev_data);
    static void zlmHookPublish(struct mg_connection* nc, int ev, void* ev_data);
    static void zlmHookStreamNoneReader(struct mg_connection* nc, int ev, void* ev_data);
    static void zlmHookStreamChanged(struct mg_connection* nc, int ev, void* ev_data);
    static void zlmHookStreamNotFound(struct mg_connection* nc, int ev, void* ev_data);
    static void zlmHookSendRtpStopped(struct mg_connection* nc, int ev, void* ev_data);

    bool init(const string& rootpath = "./", const string& indexfile = "index");
private:
    std::string m_address;
    std::string m_doc_root;
    int m_run_status;
    mutex m_mutex;

    struct mg_mgr m_mgr;
    struct mg_connection* httpcon;

    thread mTLoop;
public:
	HttpServer(int port = 80);
	~HttpServer();

    void loop();
    static void StartFun(void *data);
};
#endif