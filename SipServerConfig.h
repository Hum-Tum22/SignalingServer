#ifndef SIP_SERVER_CONFIG_H_
#define SIP_SERVER_CONFIG_H_
#include "repro/ProxyConfig.hxx"
#include "repro/AbstractDb.hxx"

class MyServerConfig : public repro::ProxyConfig
{
    repro::AbstractDb* mAbstractDb;
public:
    MyServerConfig();
    ~MyServerConfig();
    repro::AbstractDb* getDatabase(int configIndex);
    repro::AbstractDb* CreateDatabase();
};

MyServerConfig& GetSipServerConfig();
repro::AbstractDb* getCurDatabase();
#endif