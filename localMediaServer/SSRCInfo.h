#ifndef SSRC_INFO_H_
#define SSRC_INFO_H_
#include <iostream>
class SSRCInfo
{

    int port;
    std::string ssrc;
    std::string Stream;
public:
    SSRCInfo(int port, std::string ssrc, std::string stream) {
        port = port;
        ssrc = ssrc;
        Stream = stream;
    }

    int getPort() {
        return port;
    }

    void setPort(int iPort) {
        port = iPort;
    }

    std::string getSsrc() {
        return ssrc;
    }

    void setSsrc(std::string strSsrc) {
        ssrc = strSsrc;
    }

    std::string getStream() {
        return Stream;
    }

    void setStream(std::string strStream) {
        Stream = strStream;
    }
};
#endif