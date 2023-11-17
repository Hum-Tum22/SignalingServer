#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

//#include <fcntl.h>
//#include "rutil/ResipAssert.h"
//#include <cstdlib>

#include "rutil/Data.hxx"
#include "rutil/FileSystem.hxx"
#include "rutil/Logger.hxx"

#include "PersistentMessageQueue.hxx"
#include "rutil/WinLeakCheck.hxx"

using namespace resip;
using namespace regist;
using namespace std;

#define RESIPROCATE_SUBSYSTEM Subsystem::REPRO



PersistentMessageQueue::PersistentMessageQueue(const Data& baseDir) : 
   //mDb(0),
   mBaseDir(baseDir),
   mRecoveryNeeded(false)
{
//    int defaultDatabaseIndex = mProxyConfig->getConfigInt("DefaultDatabase", -1);
//    if (defaultDatabaseIndex >= 0)
//    {
//        mDb = mProxyConfig->getDatabase(defaultDatabaseIndex);
//        if (!mDb)
//        {
//            CritLog(<< "Failed to get configuration database");
//            cleanupObjects();
//            return false;
//        }
//    }
//    else     // Try legacy configuration parameter names
//    {
//#ifdef USE_MYSQL
//        Data mySQLServer;
//        mProxyConfig->getConfigValue("MySQLServer", mySQLServer);
//        if (!mySQLServer.empty())
//        {
//            WarningLog(<< "Using deprecated parameter MySQLServer, please update to indexed Database definitions.");
//            mDb = new MySqlDb(*mProxyConfig, mySQLServer,
//                mProxyConfig->getConfigData("MySQLUser", Data::Empty),
//                mProxyConfig->getConfigData("MySQLPassword", Data::Empty),
//                mProxyConfig->getConfigData("MySQLDatabaseName", Data::Empty),
//                mProxyConfig->getConfigUnsignedLong("MySQLPort", 0),
//                mProxyConfig->getConfigData("MySQLCustomUserAuthQuery", Data::Empty));
//        }
//#endif
//        if (!mAbstractDb)
//        {
//            //mAbstractDb = new BerkeleyDb(mProxyConfig->getConfigData("DatabasePath", "./", true));
//            mDb = mProxyConfig->getDatabase(defaultDatabaseIndex);
//        }
//    }
}

PersistentMessageQueue::~PersistentMessageQueue()
{

}

bool 
PersistentMessageQueue::init(bool sync, const resip::Data& queueName)
{

   return false;
}

bool 
PersistentMessageQueue::isRecoveryNeeded()
{
   return mRecoveryNeeded;
}

bool 
PersistentMessageEnqueue::push(const resip::Data& data)
{

   return false;
}

// returns true for success, false for failure - can return true and 0 records if none available
// Note:  if autoCommit is used then it is safe to allow multiple consumers
bool 
PersistentMessageDequeue::pop(size_t numRecords, std::vector<resip::Data>& records, bool autoCommit)  
{

   return false;
}

bool 
PersistentMessageDequeue::commit()
{

   return false;
}

void 
PersistentMessageDequeue::abort()
{
   mNumRecords = 0;
}


/* ====================================================================
 * The Vovida Software License, Version 1.0 
 * 
 * Copyright (c) 2000 Vovida Networks, Inc.  All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 
 * 3. The names "VOCAL", "Vovida Open Communication Application Library",
 *    and "Vovida Open Communication Application Library (VOCAL)" must
 *    not be used to endorse or promote products derived from this
 *    software without prior written permission. For written
 *    permission, please contact vocal@vovida.org.
 *
 * 4. Products derived from this software may not be called "VOCAL", nor
 *    may "VOCAL" appear in their name, without prior written
 *    permission of Vovida Networks, Inc.
 * 
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, TITLE AND
 * NON-INFRINGEMENT ARE DISCLAIMED.  IN NO EVENT SHALL VOVIDA
 * NETWORKS, INC. OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT DAMAGES
 * IN EXCESS OF $1,000, NOR FOR ANY INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 * 
 * ====================================================================
 */
