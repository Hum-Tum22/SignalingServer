#ifndef MYMESSAGEDECORATOR_HXX
#define MYMESSAGEDECORATOR_HXX

#if defined(HAVE_CONFIG_H)
  #include "config.h"
#endif

#include "rutil/Data.hxx"
#include "resip/stack/MessageDecorator.hxx"

namespace sipserver
{

class MyMessageDecorator : public resip::MessageDecorator
{
   public:
      MyMessageDecorator();
      virtual ~MyMessageDecorator() {}

      virtual void decorateMessage(resip::SipMessage &msg,
                                  const resip::Tuple &source,
                                  const resip::Tuple &destination,
                                  const resip::Data& sigcompId);
      virtual void rollbackMessage(resip::SipMessage& msg) {};
      virtual resip::MessageDecorator* clone() const { return new MyMessageDecorator(); };

};

}

#endif