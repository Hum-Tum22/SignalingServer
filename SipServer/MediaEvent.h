#ifndef MediaEvent_h_
#define MediaEvent_h_

#include <resip/dum/DumCommand.hxx>
#include "HandleTypes.h"

namespace B2BSvr
{

    class ConversationManager;
    class Message;

    /**
      This class represents an event that happens to a media participant.

      Author: Scott Godin (sgodin AT SipSpectrum DOT com)
    */

    class MediaEvent : public resip::DumCommand
    {
    public:
        typedef enum
        {
            RESOURCE_DONE,
            RESOURCE_FAILED,
            VOICE_STARTED,
            VOICE_STOPPED
        } MediaEventType;

        typedef enum
        {
            DIRECTION_IN,
            DIRECTION_OUT
        } MediaDirection;

        MediaEvent(ConversationManager& conversationManager, ParticipantHandle partHandle, MediaEventType eventType, MediaDirection direction);
        virtual void executeCommand();

        Message* clone() const;
        EncodeStream& encode(EncodeStream& strm) const;
        EncodeStream& encodeBrief(EncodeStream& strm) const;

    private:
        ConversationManager& mConversationManager;
        ParticipantHandle mParticipantHandle;
        MediaEventType mEventType;
        MediaDirection mDirection;
    };


}

#endif