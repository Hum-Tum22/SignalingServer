#ifndef SIP_SERVER_CONFIG_H_
#define SIP_SERVER_CONFIG_H_
#include "RegistrarServer/ProxyConfig.hxx"
#include "RegistrarServer/AbstractDb.hxx"

class MyServerConfig : public regist::ProxyConfig
{
    regist::AbstractDb* mAbstractDb;
public:
    MyServerConfig();
    ~MyServerConfig();
    regist::AbstractDb* getDatabase(int configIndex);
    regist::AbstractDb* CreateDatabase();
};

MyServerConfig& GetSipServerConfig();
regist::AbstractDb* getCurDatabase();
#endif