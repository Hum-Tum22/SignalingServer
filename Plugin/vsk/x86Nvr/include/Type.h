#ifndef NVR_TYPE_H
#define NVR_TYPE_H

typedef void         				*LPVOID;
typedef void         				*PVOID;
typedef unsigned char				U8;
typedef unsigned short				U16;
typedef unsigned int				U32;
typedef unsigned long long			U64;
typedef char  						S8;

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif
#ifndef F_OK
#define F_OK 0
#endif


// new types supported by VS2013
#ifdef _WIN32
#include <intsafe.h>
typedef int  BOOL;

typedef __int64		int64;
typedef int			int32;
typedef short int	int16;
typedef char		int8;
typedef unsigned __int64 uint64;
typedef unsigned int uint32;
typedef unsigned short int uint16;
typedef unsigned char  uint8;
typedef uint64 uint64_t;
#else
#include<inttypes.h>
#include<stdint.h>
typedef int64_t  int64;
typedef int32_t  int32;
typedef int16_t  int16;
typedef int8_t   int8;
typedef uint64_t uint64;
typedef uint32_t uint32;
typedef uint16_t uint16;
typedef uint8_t  uint8;
//typedef  long         LONG;
#ifndef LONG
#define LONG int
#endif

typedef unsigned long ULONG;
typedef long long LONGLONG;
//typedef int			SOCKET;

//typedef int64_t LONGLONG;

#endif


#ifndef _WIN32
#ifndef INVALID_SOCKET
#define INVALID_SOCKET  (int)(0xffffffff)
#endif
#define FAR
#define __stdcall

typedef unsigned char		UCHAR;
typedef char				CHAR;

//typedef int				BOOL;
typedef long				BOOL;		// NetStructCom.h
typedef unsigned short      WORD;
//typedef unsigned long		DWORD;
#ifndef DWORD
#define DWORD uint32_t
#endif

typedef unsigned char       BYTE;
typedef unsigned short      USHORT;
typedef short				SHORT;
typedef float               FLOAT;

typedef int                 INT;
typedef unsigned int        UINT;
typedef unsigned int        *PUINT;
typedef void*				HWND;

typedef void                VOID;
typedef void *				HANDLE;
//typedef BYTE				BOOLEAN;




#define WSAEVENT                HANDLE

#define MAKEWORD(a, b)      ((uint16)(((uint8)(((uint32)(a)) & 0xff)) | ((uint16)((uint8)(((uint32)(b)) & 0xff))) << 8))
#define MAKELONG(a, b)      ((uint32)(((uint16)(((uint32)(a)) & 0xffff)) | ((uint32)((uint16)(((uint32)(b)) & 0xffff))) << 16))
#define LOWORD(l)           ((uint16)(((uint32)(l)) & 0xffff))
#define HIWORD(l)           ((uint16)((((uint32)(l)) >> 16) & 0xffff))
#define LOBYTE(w)           ((uint8)(((uint32)(w)) & 0xff))
#define HIBYTE(w)           ((uint8)((((uint32)(w)) >> 8) & 0xff))
#define sprintf_s			snprintf

#include <pthread.h>
//#define  uintptr_t          pthread_t

#endif
#define CONST const

#define SafeDelete(x)		{ delete(x);  x = NULL; }
#define SafeDeleteArray(x)		{ delete [] (x);  x = NULL; }
#define RealocCharArray(x, size)		{ delete [] (x);  x = NULL; x = new char[size];}




/// 定义printf的整数输出格式
//#ifndef PRIu64
//#ifdef WIN32
//#define PRIu64 "l64d"
//#else
//#define PRIu64 "lld"
//#endif
//#endif

//------------------------------------------------------------------
#include <string>
#include <vector>
#include <map>
#include <list>
#include <sstream>

//------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#if _MSC_VER
#define snprintf _snprintf
#endif

#define my_max(a,b)            (((a) > (b)) ? (a) : (b))
#define my_min(a,b)            (((a) < (b)) ? (a) : (b))

#endif // NVR_TYPE_H
