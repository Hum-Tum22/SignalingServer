#include "rutil/ResipAssert.h"

#include "rutil/AsyncBool.hxx"
#include "resip/dum/DialogUsageManager.hxx"
#include "RegistrarServerAuthManager.hxx"
#include "resip/dum/ServerAuthManager.hxx"
#include "resip/dum/UserAuthInfo.hxx"
#include "repro/UserStore.hxx"
#include "repro/AclStore.hxx"

#define RESIPROCATE_SUBSYSTEM Subsystem::REPRO

using namespace resip;
using namespace repro;


RegistServerAuthManager::RegistServerAuthManager(DialogUsageManager& dum,
    Dispatcher* authRequestDispatcher,
    AclStore& aclDb,
    bool useAuthInt,
    bool rejectBadNonces,
    bool challengeThirdParties,
    const Data& staticRealm) :
    ServerAuthManager(dum, dum.dumIncomingTarget(), challengeThirdParties, staticRealm),
    mDum(dum),
    mAuthRequestDispatcher(authRequestDispatcher),
    mAclDb(aclDb),
    mUseAuthInt(useAuthInt),
    mRejectBadNonces(rejectBadNonces)
{
}

RegistServerAuthManager::~RegistServerAuthManager()
{
}

bool
RegistServerAuthManager::useAuthInt() const
{
    return mUseAuthInt;
}

bool
RegistServerAuthManager::rejectBadNonces() const
{
    return mRejectBadNonces;
}

AsyncBool
RegistServerAuthManager::requiresChallenge(const SipMessage& msg)
{
    resip_assert(msg.isRequest());
    if (!mAclDb.isRequestTrusted(msg))
    {
        return ServerAuthManager::requiresChallenge(msg);
    }
    else
    {
        return False;
    }
}

void
RegistServerAuthManager::requestCredential(const Data& user,
    const Data& realm,
    const SipMessage& msg,
    const Auth& auth,
    const Data& transactionId)
{
    // Build a UserAuthInfo object and pass to UserAuthGrabber to have a1 password filled in
    UserAuthInfo* async = new UserAuthInfo(user, realm, transactionId, &mDum);
    std::unique_ptr<ApplicationMessage> app(async);
    mAuthRequestDispatcher->post(app);
}
