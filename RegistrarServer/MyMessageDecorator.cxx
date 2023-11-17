#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <rutil/Log.hxx>
#include <rutil/Logger.hxx>
#include <resip/stack/SdpContents.hxx>
#include <resip/stack/SipMessage.hxx>
#include <resip/stack/Tuple.hxx>
//#include "AppSubsystem.hxx"

#include "MyMessageDecorator.hxx"

#include "../SipServer.h"

#define RESIPROCATE_SUBSYSTEM Subsystem::TEST

using namespace resip;
using namespace sipserver;

MyMessageDecorator::MyMessageDecorator()
{
}

void
MyMessageDecorator::decorateMessage(SipMessage &msg,
                                  const Tuple &source,
                                  const Tuple &destination,
                                  const Data& sigcompId)
{
   StackLog(<<"Got a message for decoration");
   if (msg.method() == INVITE)
   {
       SdpContents* sdp = dynamic_cast<SdpContents*>(msg.getContents());
       if (sdp)
       {
           const std::list<Data>& iceOpts = sdp->session().getValues("ice-options");
           if (std::find(iceOpts.begin(), iceOpts.end(), "trickle") != iceOpts.end())
           {
               StackLog(<< "Not modifying the SDP as ICE is in use");
           }
           else
           {
               if (!sdp->session().isConnection())
               {
                   ErrLog(<< "SDP is missing the connection parameter");
                   return;
               }
               SdpContents::Session::Connection& c = sdp->session().connection();
               StackLog(<< "session connection address = " << c.getAddress());
               if (c.getAddress() == "0.0.0.0")
               {
                   Data newAddr = Tuple::inet_ntop(source);
                   StackLog(<< "replacing session connection address with " << newAddr);
                   c.setAddress(newAddr);
               }
               std::list<SdpContents::Session::Connection>& mc = sdp->session().media().front().getMediumConnections();
               std::list<SdpContents::Session::Connection>::iterator it = mc.begin();
               for (; it != mc.end(); it++)
               {
                   SdpContents::Session::Connection& _mc = *it;
                   if (_mc.getAddress() == "0.0.0.0")
                   {
                       Data newAddr = Tuple::inet_ntop(source);
                       StackLog(<< "replacing media stream connection address with " << newAddr);
                       _mc.setAddress(newAddr);
                   }
               }
           }
           // Fill in IP and Port from source
           //sdp->session().connection().setAddress(Tuple::inet_ntop(source), source.ipVersion() == V6 ? SdpContents::IP6 : SdpContents::IP4);
           //sdp->session().origin().setAddress(Tuple::inet_ntop(source), source.ipVersion() == V6 ? SdpContents::IP6 : SdpContents::IP4);
           if (msg.isRequest())
           {
               sipserver::SipServer* pSvr = GetServer();
               if (pSvr)
               {
                   sdp->session().connection().setAddress(pSvr->zlmHost.c_str());
                   sdp->session().origin().setAddress(pSvr->zlmHost.c_str());
               }
           }
           else if (msg.isResponse())
           {
               //sdp->session().connection().setAddress("192.168.1.232");
               //sdp->session().origin().setAddress("192.168.1.232");
           }
           Data sourceip = Tuple::inet_ntop(source);
           Data destip = Tuple::inet_ntop(destination);
       }
   }
   else if (msg.method() == REGISTER)
   {
       Data strMsg;
       {
           oDataStream ds(strMsg);
           ds << msg;
           ds.flush();
       }
       if (msg.isRequest())
       {
           Data destip = Tuple::inet_ntop(destination);
           cout << destip << endl;
       }
       else
       {
           Data destip = Tuple::inet_ntop(source);
           cout << destip << endl;
       }
   }
   else if (msg.method() == MESSAGE)
   {
       Data destip = Tuple::inet_ntop(destination);
       if (destip == "192.168.1.230")
       {
           cout << destip << endl;
       }
   }
   else
   {
       Data destip = Tuple::inet_ntop(destination);
   }
}