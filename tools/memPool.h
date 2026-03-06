#pragma once

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <queue>
#include "SelfLog.h"

//#include "unLockQueue.h"
//#include "lineBuf.h"


//template <typename T>
class avMemPool
{
public:
    avMemPool(size_t size = 0) : buf(NULL), bm_ptr(NULL), curBuf(NULL)
        , bufSize(size), readPtr(NULL), readSize(0)
    {
        //6M Kbps  =>11M 缓存15s
        if(bufSize == 0)
        {
            bufSize = 1024 * 1024 * 1;
        }
        
        try {
            buf = new uint8_t[bufSize];
            bm_ptr = new uint8_t[bufSize];
            curBuf = buf;
            rPos = wPos = frPos = fwPos = 0;
            LogOut(BLL, L_DEBUG, "new avMemPool buf:%p, bm_ptr:%p, curBuf:%p, bufSize:%lu", buf, bm_ptr, curBuf, bufSize);
        } catch (std::bad_alloc& e) {
            LogOut(BLL, L_ERROR, "avMemPool allocation failed: %s, bufSize:%lu", e.what(), bufSize);
            // 内存分配失败，保持 buf、bm_ptr、curBuf 为 NULL
        }
    };
    ~avMemPool()
    {
        if(buf)
        {
            delete[] buf; buf = NULL;
        }
        if(bm_ptr)
        {
            delete[] bm_ptr; bm_ptr = NULL;
        }
        curBuf = NULL;
        readPtr = NULL;
        LogOut(BLL, L_DEBUG, "delete avMemPool");
    }
    int wirteData(void* data, size_t size)
    {
        // 检查数据大小是否超过缓冲区容量
        if(size > bufSize)
        {
            LogOut(BLL, L_ERROR, "wirteData error: size(%lu) > bufSize(%lu)", size, bufSize);
            return -1;
        }

        // 检查指针有效性
        if(!curBuf || !data)
        {
            LogOut(BLL, L_ERROR, "wirteData error: curBuf=%p, data=%p", curBuf, data);
            return -1;
        }

        if(bufSize - wPos >= size)
        {
            readPtr = curBuf + wPos;
            readSize = size;
            memcpy(curBuf + wPos, data, size);
            wPos += size;
            return Init();
        }
        else
        {
            if(wPos != rPos)
            {
                frPos = rPos;
                fwPos = wPos;
                LogOut(BLL, L_DEBUG, "------- wPos:%lu rPos:%lu error", wPos, rPos);
            }
            wPos = rPos = 0;
            if(curBuf == buf)
            {
                curBuf = bm_ptr;
            }
            else if(curBuf == bm_ptr)
            {
                curBuf = buf;
            }
            else
            {
                LogOut(BLL, L_ERROR, "------- curBuf error: curBuf=%p, buf=%p, bm_ptr=%p", curBuf, buf, bm_ptr);
                return -1;
            }

            // 再次检查切换后的缓冲区是否有效
            if(!curBuf)
            {
                LogOut(BLL, L_ERROR, "wirteData error: curBuf is NULL after switch");
                return -1;
            }

            return wirteData(data, size);
        }
        return 0;
    };

protected:
    uint8_t* buf, * bm_ptr, * curBuf;
    size_t bufSize;
    size_t rPos, wPos, frPos, fwPos;

    //可读数据开始指针、有效数据长度
    const uint8_t* readPtr;
    size_t readSize;

    virtual int Init() = 0;
};