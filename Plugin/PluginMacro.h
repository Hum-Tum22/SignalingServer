#pragma once


#ifdef _WIN32
#include <Windows.h>
#include <iostream>

typedef unsigned short ushort;
// DESCRIPTION
char* GetErrorDescriptionString(DWORD ErrorCode)
{
    HLOCAL LocalAddress = NULL;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, ErrorCode, 0, (PTSTR)&LocalAddress, 0, NULL);

    // int cch = WideCharToMultiByte(cp, 0, pwsz, -1, 0, 0, NULL, NULL);
    // char* psz = new char[cch];
    // WideCharToMultiByte(cp, 0, pwsz, -1, psz, cch, NULL, NULL);

    //CString sChar = CString(LocalAddress);
    // USES_CONVERSION;
    // std::string sddddd = T2A(sChar);
    std::string sddddd;
    return (char*)sddddd.c_str();
}
#define PlHandle HMODULE
#define dlopenFun(x)  LoadLibrary(x)
#define LoadSharedLibFun(handle, name) GetProcAddress(handle, name)
#define dlcloseFun(h) FreeLibrary(h)
#define dlerrorFun() GetErrorDescriptionString(GetLastError())

#ifdef VSK_PLUGIN_WIN_EXPORTS
#define VSK_PLUGIN_API extern "C"  __declspec(dllexport)
#else
#define VSK_PLUGIN_API extern "C"  __declspec(dllimport)
#endif
#define CALL_METHOD __stdcall

#else
#include <dlfcn.h>
#define CALLBACK

#define dlopenFun(x)  dlopen(x, RTLD_NOW)
#define LoadSharedLibFun(handle, name) dlsym(handle, name)
#define dlcloseFun(h) dlclose(h)
#define dlerrorFun() dlerror()
#define PlHandle void*

#define VSK_PLUGIN_API extern "C" 
#define CALL_METHOD 

#endif
