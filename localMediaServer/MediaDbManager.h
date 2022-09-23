#pragma once

#include "MediaServerItem.h"
#include "../SqliteDb.h"

#include <iostream>
#include <list>

class MediaServerDbManagr
{
public:
    virtual void IniTable();
    int add(MediaServerItem mediaServerItem);
    int update(MediaServerItem mediaServerItem);
    int updateByHostAndPort(MediaServerItem mediaServerItem);
    MediaServerItem queryOne(std::string id);
    std::list<MediaServerItem> queryAll();
    void delOne(std::string id);
    void delOneByIPAndPort(std::string host, int port);
    int delDefault();
    MediaServerItem queryOneByHostAndPort(std::string host, int port);
    MediaServerItem queryDefault();
private:
    repro::SqliteDb* pDb;
};