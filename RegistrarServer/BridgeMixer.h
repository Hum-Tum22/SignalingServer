#ifndef BridgeMixer_h_
#define BridgeMixer_h_

#if (_MSC_VER >= 1600)
#include <stdint.h>       // Use Visual Studio's stdint.h
#define _MSC_STDINT_H_    // This define will ensure that stdint.h in sipXport tree is not used
#endif

namespace B2BSvr
{
    class ConversationManager;
    class Participant;

    /**
      This class is used to control the sipX Bridge Mixer mix matrix.

      If there is ever a change required in the mixer bridge, the
      application should call calculateMixWeightsForParticipant
      in order to have the changes detected and applied.

      Author: Scott Godin (sgodin AT SipSpectrum DOT com)
    */

    class BridgeMixer
    {
    public:
        /**
          Constructor
        */
        BridgeMixer();
        virtual ~BridgeMixer();

        /**
          Calculates all of the current mixer settings required
          for the passed in participant and applies them.
          Calculations are based off of Participants membership
          into Conversations and the input/output gain settings.

          @param participant Participant to calculate mixer weights for
        */
        virtual void calculateMixWeightsForParticipant(Participant* participant) = 0;

        /**
          Logs a multiline representation of the current state
          of the mixing matrix.
        */
        virtual void outputBridgeMixWeights() = 0;
    };

}
#endif