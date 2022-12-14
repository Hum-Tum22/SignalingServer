#if !defined(RESIP_TuSelector_HXX)
#define RESIP_TuSelector_HXX 

#include "resip/stack/SipMessage.hxx"
#include "resip/stack/StatisticsMessage.hxx"
#include "resip/stack/TransactionUserMessage.hxx"
#include "rutil/TimeLimitFifo.hxx"
#include "rutil/Fifo.hxx"

namespace resip
{

class ConnectionTerminated;
class KeepAlivePong;
class Message;
class TransactionUser;
class AsyncProcessHandler;

class TuSelector
{
   public:
      TuSelector(TimeLimitFifo<Message>& fallBackFifo);
      ~TuSelector();
      
      void add(Message* msg, TimeLimitFifo<Message>::DepthUsage usage);
      void add(ConnectionTerminated* term);
      void add(KeepAlivePong* pong);
      
      unsigned int size() const;      
      bool wouldAccept(TimeLimitFifo<Message>::DepthUsage usage) const;
  
      TransactionUser* selectTransactionUser(const SipMessage& msg);
      bool haveTransactionUsers() const { return mTuSelectorMode; }
      void registerTransactionUser(TransactionUser&, const bool front = false);
      void requestTransactionUserShutdown(TransactionUser&);
      void unregisterTransactionUser(TransactionUser&);
      void process();
      unsigned int getTimeTillNextProcessMS();
      bool isTransactionUserStillRegistered(const TransactionUser* ) const;

      // Handler is notified when a message is posted
      // to the default application receive queue.
      void setFallbackPostNotify(AsyncProcessHandler *handler);
      
      void setCongestionManager(CongestionManager* manager);
      CongestionManager::RejectionBehavior getRejectionBehavior(TransactionUser* tu) const;
      uint32_t getExpectedWait(TransactionUser* tu) const;

   private:
      void remove(TransactionUser* tu);
      void markShuttingDown(TransactionUser* tu);
      bool exists(TransactionUser* tu);

   private:
      struct Item
      {
            Item(TransactionUser* ptu) : tu(ptu), shuttingDown(false) {}
            TransactionUser* tu;
            bool shuttingDown;
            bool operator==(const Item& rhs) { return tu == rhs.tu; }
      };
      
      typedef std::vector<Item> TuList;
      TuList mTuList;
      TimeLimitFifo<Message>& mFallBackFifo;
      CongestionManager* mCongestionManager;
      AsyncProcessHandler *mFallbackPostNotify;
      Fifo<TransactionUserMessage> mShutdownFifo;
      bool mTuSelectorMode;
      StatisticsMessage::Payload mStatsPayload;
};
}

#endif

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
 * 
 * This software consists of voluntary contributions made by Vovida
 * Networks, Inc. and many individuals on behalf of Vovida Networks,
 * Inc.  For more information on Vovida Networks, Inc., please see
 * <http://www.vovida.org/>.
 *
 */
