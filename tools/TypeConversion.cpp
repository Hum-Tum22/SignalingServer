#include "TypeConversion.h"
#include "SelfLog.h"


double String2Double(const std::string strValue)
{
    try
    {
        return std::stod(strValue);
    }
    catch(const std::invalid_argument& e)
    {
        LogOut(TOOL, L_ERROR, "stod invalid argument %s %s", e.what(), strValue.c_str());
        return 0;
    }
    catch(const std::out_of_range& e)
    {
        LogOut(TOOL, L_ERROR, "stod out of range %s", e.what());
        return 0;
    }
    catch(...)
    {
        LogOut(TOOL, L_ERROR, "stod error unknow");
        return 0;
    }
    LogOut(TOOL, L_ERROR, "stod error end");
    return -1;
}
float String2float(const std::string strValue)
{
    try
    {
        return std::stof(strValue);
    }
    catch(const std::invalid_argument& e)
    {
        LogOut(TOOL, L_ERROR, "stof invalid argument %s %s", e.what(), strValue.c_str());
        return 0;
    }
    catch(const std::out_of_range& e)
    {
        LogOut(TOOL, L_ERROR, "stof out of range %s", e.what());
        return 0;
    }
    catch(...)
    {
        LogOut(TOOL, L_ERROR, "stof error unknow");
        return 0;
    }
    LogOut(TOOL, L_ERROR, "stof error end");
    return -1;
}
int String2Int(const std::string strValue)
{
    try
    {
        return std::stoi(strValue);
    }
    catch(const std::invalid_argument& e)
    {
        LogOut(TOOL, L_ERROR, "stoi invalid argument %s %s", e.what(), strValue.c_str());
        return 0;
    }
    catch(const std::out_of_range& e)
    {
        LogOut(TOOL, L_ERROR, "stoi out of range %s", e.what());
        return 0;
    }
    catch(...)
    {
        LogOut(TOOL, L_ERROR, "stoi error unknow");
        return 0;
    }
    LogOut(TOOL, L_ERROR, "stoi error end");
    return -1;
}
long String2Long(const std::string strValue)
{
    try
    {
        return std::stol(strValue);
    }
    catch(const std::invalid_argument& e)
    {
        LogOut(TOOL, L_ERROR, "stol invalid argument %s %s", e.what(), strValue.c_str());
        return 0;
    }
    catch(const std::out_of_range& e)
    {
        LogOut(TOOL, L_ERROR, "stol out of range %s", e.what());
        return 0;
    }
    catch(...)
    {
        LogOut(TOOL, L_ERROR, "stol error unknow");
        return 0;
    }
    LogOut(TOOL, L_ERROR, "stol error end");
    return -1;
}

long long String2ll(const std::string strValue)
{
    try
    {
        return stoll(strValue);
    }
    catch(const std::invalid_argument& e)
    {
        LogOut(TOOL, L_ERROR, "stoll invalid argument %s %s", e.what(), strValue.c_str());
        return 0;
    }
    catch(const std::out_of_range& e)
    {
        LogOut(TOOL, L_ERROR, "stoll out of range %s", e.what());
        return 0;
    }
    catch(...)
    {
        LogOut(TOOL, L_ERROR, "stoll error unknow");
        return 0;
    }
    LogOut(TOOL, L_ERROR, "stoll error end");
    return -1;
}
unsigned long String2Ul(const std::string strValue)
{
    try
    {
        return std::stoul(strValue);
    }
    catch(const std::invalid_argument& e)
    {
        LogOut(TOOL, L_ERROR, "stoul invalid argument %s %s", e.what(), strValue.c_str());
        return 0;
    }
    catch(const std::out_of_range& e)
    {
        LogOut(TOOL, L_ERROR, "stoul out of range %s", e.what());
        return 0;
    }
    catch(...)
    {
        LogOut(TOOL, L_ERROR, "stoul error unknow");
        return 0;
    }
    LogOut(TOOL, L_ERROR, "stoul error end");
    return -1;
}

unsigned long long String2Ull(const std::string strValue)
{
    try
    {
        return std::stoull(strValue);
    }
    catch(const std::invalid_argument& e)
    {
        LogOut(TOOL, L_ERROR, "stoull invalid argument %s %s", e.what(), strValue.c_str());
        return 0;
    }
    catch(const std::out_of_range& e)
    {
        LogOut(TOOL, L_ERROR, "stoull out of range %s", e.what());
        return 0;
    }
    catch(...)
    {
        LogOut(TOOL, L_ERROR, "stoull error unknow");
        return 0;
    }
    LogOut(TOOL, L_ERROR, "stoull error end");
    return -1;
}