#pragma once

#ifdef _WIN32
#include <Windows.h>

#define LoadSharedLibFun(handle, name) GetProcAddress(handle, name)
#define CALLBACK __stdcall
#else
#include <dlfcn.h>
#define CALLBACK

#define LoadSharedLibFun(handle, name) dlsym(handle, name)
#endif
class PluginInter
{
public:
	typedef enum
	{
		JSON_SDK,
	}InterProtocal;
	PluginInter(InterProtocal type) :pluginType(type) {};
	InterProtocal pluginType;
};