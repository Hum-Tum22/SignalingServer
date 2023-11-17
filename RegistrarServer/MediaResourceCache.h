#ifndef MediaResourceCache_h_
#define MediaResourceCache_h_

#include <map>
#include <rutil/Mutex.hxx>
#include <rutil/Data.hxx>

namespace B2BSvr
{

    /**
      This class is responsible for caching media resource buffers.  It use a Mutex
      for locking, so that additions can happen from other threads.

      Author: Scott Godin (sgodin AT SipSpectrum DOT com)
    */

    class MediaResourceCache
    {
    public:
        MediaResourceCache();
        virtual ~MediaResourceCache();
        void addToCache(const resip::Data& name, const resip::Data& buffer, int type);
        bool getFromCache(const resip::Data& name, resip::Data** buffer, int* type);

    private:
        class CacheItem
        {
        public:
            CacheItem(const resip::Data& buffer, int type) :
                mBuffer(buffer), mType(type) {}
            resip::Data mBuffer;
            int mType;
        };

        typedef std::map<resip::Data, CacheItem*> CacheMap;
        CacheMap mCacheMap;
        resip::Mutex mMutex;
    };

}

#endif