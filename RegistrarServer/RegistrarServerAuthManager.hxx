#if !defined(REPRO_SERVERAUTHMANAGER_HXX)
#define REPRO_SERVERAUTHMANAGER_HXX

#include <map>

#include "rutil/AsyncBool.hxx"
#include "resip/stack/Auth.hxx"
#include "resip/stack/Message.hxx"
#include "resip/dum/UserProfile.hxx"
#include "resip/dum/ServerAuthManager.hxx"
#include "resip/stack/Dispatcher.hxx"

namespace resip
{
    class Profile;
    class DialogUsageManager;
}

namespace repro
{
    class AclStore;

    class RegistServerAuthManager : public resip::ServerAuthManager
    {
    public:
        RegistServerAuthManager(resip::DialogUsageManager& dum,
            resip::Dispatcher* authRequestDispatcher,
            AclStore& aclDb,
            bool useAuthInt,
            bool rejectBadNonces,
            bool challengeThirdParties,
            const resip::Data& staticRealm = resip::Data::Empty);

        ~RegistServerAuthManager();

    protected:
        // this call back should async cause a post of UserAuthInfo
        virtual void requestCredential(const resip::Data& user,
            const resip::Data& realm,
            const resip::SipMessage& msg,
            const resip::Auth& auth,
            const resip::Data& transactionId);

        virtual bool useAuthInt() const;
        virtual bool rejectBadNonces() const;
        virtual resip::AsyncBool requiresChallenge(const resip::SipMessage& msg);

    private:
        resip::DialogUsageManager& mDum;
        resip::Dispatcher* mAuthRequestDispatcher;
        AclStore& mAclDb;
        bool mUseAuthInt;
        bool mRejectBadNonces;
    };


}

#endif