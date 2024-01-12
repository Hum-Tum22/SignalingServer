#pragma once
#define ASIO_STANDALONE
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <websocketpp/extensions/permessage_deflate/enabled.hpp>
#include <iostream>
#include <mutex>

#include "../media/MediaMng.h"
#include "myjsondef.h"
#include "writer.h"
#include "stringbuffer.h"
#include "document.h"


struct ws_config : public websocketpp::config::asio
{
    // pull default settings from our core config
    typedef websocketpp::config::asio core;

    typedef core::concurrency_type concurrency_type;
    typedef core::request_type request_type;
    typedef core::response_type response_type;
    typedef core::message_type message_type;
    typedef core::con_msg_manager_type con_msg_manager_type;
    typedef core::endpoint_msg_manager_type endpoint_msg_manager_type;

    typedef core::alog_type alog_type;
    typedef core::elog_type elog_type;
    typedef core::rng_type rng_type;
    typedef core::endpoint_base endpoint_base;

    static bool const enable_multithreading = true;

    struct transport_config : public core::transport_config {
        typedef core::concurrency_type concurrency_type;
        typedef core::elog_type elog_type;
        typedef core::alog_type alog_type;
        typedef core::request_type request_type;
        typedef core::response_type response_type;

        static bool const enable_multithreading = true;
    };

    typedef websocketpp::transport::asio::endpoint<transport_config>
        transport_type;

    static const websocketpp::log::level elog_level =
        websocketpp::log::elevel::none;
    static const websocketpp::log::level alog_level =
        websocketpp::log::alevel::none;

    /// permessage_compress extension
    struct permessage_deflate_config {};

    typedef websocketpp::extensions::permessage_deflate::enabled
        <permessage_deflate_config> permessage_deflate_type;
};

typedef websocketpp::server<ws_config> server;


typedef struct _WsStreamInfo
{
    uint32_t readhandle;
    std::thread t;
    websocketpp::connection_hdl hdl;
    server* s;

    MediaStream::Ptr ms;
    int flags;
    bool haveKeyFrame;
    int IsLive;
    //send
    char* pBuffer;
    int nBufLen;
    int nDtLen;
    //回放
    uint64_t prevTime;
    uint64_t currTime;
    unsigned int fPlayHandle;
    float speed;
    char* pRdBuffer;
    int mRdBufLen;
    bool isFirst;
    uint64_t nFrameTime;

    ~_WsStreamInfo()
    {
        if (pBuffer)
        {
            delete[] pBuffer;
            pBuffer = NULL;
        }
        if (pRdBuffer)
        {
            delete[] pRdBuffer;
            pRdBuffer = NULL;
        }
    }
}WsStreamInfo;
class WsServer
{
public:
    WsServer(short port = 80);
    ~WsServer();

    void on_open(websocketpp::connection_hdl hdl);
    void on_close(websocketpp::connection_hdl hdl);
    void on_message(server* s, websocketpp::connection_hdl hdl, server::message_ptr msg);
    void on_socket_init(websocketpp::connection_hdl, asio::ip::tcp::socket& s);

public:
    static void RtPreviewThread(WsStreamInfo* smInfo);
    static void PlayBackThread(WsStreamInfo* smInfo);

    int WebsocketMessage(server* s, websocketpp::connection_hdl hdl, const char* message, size_t len);
    static int RealTimePlayAction(WsStreamInfo* sinfo, rapidjson_sip::Document& document, rapidjson_sip::Writer<rapidjson_sip::StringBuffer>& writer);
    static int PlayBackAction(WsStreamInfo* sinfo, rapidjson_sip::Document& document, rapidjson_sip::Writer<rapidjson_sip::StringBuffer>& writer);
    static int PlayBackControlAction(WsStreamInfo* sinfo, rapidjson_sip::Document& document, rapidjson_sip::Writer<rapidjson_sip::StringBuffer>& writer);
private:
    server m_endpoint;
    size_t num_threads;
    typedef websocketpp::lib::shared_ptr<websocketpp::lib::thread> thread_ptr;
    std::vector<thread_ptr> ts;

    typedef std::map<websocketpp::connection_hdl, WsStreamInfo*, std::owner_less<websocketpp::connection_hdl>> ConMap;
    std::mutex conMtx;
    ConMap m_connections;
};