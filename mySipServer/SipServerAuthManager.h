#pragma once
#include "resip/stack/SipMessage.hxx"
#include "../SipServer/ReproServerAuthManager.hxx"
#include "resip/stack/Dispatcher.hxx"
#include "resip/stack/Helper.hxx"

using namespace resip;
namespace repro
{
class SipServerAuthManager : public resip::ServerAuthManager
{
public:
	SipServerAuthManager(resip::DialogUsageManager& dum,
		resip::Dispatcher* authRequestDispatcher,
		AclStore& aclDb,
		bool useAuthInt,
		bool rejectBadNonces,
		bool challengeThirdParties,
		int authtype,
		Data serverid = resip::Data::Empty,
		const resip::Data& staticRealm = resip::Data::Empty);

	SipServerAuthManager(resip::DialogUsageManager& dum,
		resip::Dispatcher* authRequestDispatcher,
		AclStore& aclDb,
		bool useAuthInt,
		bool rejectBadNonces,
		bool challengeThirdParties,
		const resip::Data& staticRealm = resip::Data::Empty);

	~SipServerAuthManager();

	std::pair<Helper::AuthResult, Data> advancedAuthenticateRequestSm(SipMessage& request, int expiresDelta = 0, bool proxyAuthorization = true);

protected:
	// this call back should async cause a post of UserAuthInfo
	virtual void requestCredential(const resip::Data& user,
		const resip::Data& realm,
		const resip::SipMessage& msg,
		const resip::Auth& auth,
		const resip::Data& transactionId);
	virtual void requestCredentialsm(const resip::Data& random1,
		const resip::Data& random2,
		const resip::Data& serverid,
		const resip::SipMessage& msg,
		const resip::Auth& auth,
		const resip::Data& transactionId);

	virtual bool useAuthInt() const;
	virtual bool rejectBadNonces() const;
	virtual resip::AsyncBool requiresChallenge(const resip::SipMessage& msg);

	virtual void SetAuthenticationMode(bool isproxy);

	// returns the SipMessage that was authorized if succeeded or returns 0 if
	// rejected. 
	virtual SipMessage* handleUserAuthInfo(UserAuthInfo* auth);

	virtual Result handle(resip::SipMessage* sipMsg);
protected:
	virtual bool proxyAuthenticationMode() const; 
	virtual bool authorizedForThisIdentity(const resip::Data &user, const resip::Data &realm, resip::Uri &fromUri);
	virtual bool authorizedForThisServerID(const resip::Data &user);
	
	Result issueChallengeIfRequiredsm(SipMessage *sipMsg);
	// sends a 407 challenge to the UAC who sent sipMsg
	void issueChallengesm(SipMessage *sipMsg);

	virtual void onAuthSuccess(const SipMessage& msg);
	virtual void onAuthFailure(AuthFailureReason reason, const SipMessage& msg);
private:
	resip::DialogUsageManager& mDum;
	resip::Dispatcher* mAuthRequestDispatcher;
	AclStore&  mAclDb;
	bool mUseAuthInt;
	bool mRejectBadNonces;

	bool mProxyAuthenticationMode;

	Data IServerID;
	int authmethod;
};
}
