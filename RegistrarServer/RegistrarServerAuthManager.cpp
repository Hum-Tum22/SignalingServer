#include "RegistrarServerAuthManager.hxx"
#include "rutil/Logger.hxx"
#include "rutil/ResipAssert.h"

#include "resip/dum/DialogUsageManager.hxx"
#include "resip/dum/ServerAuthManager.hxx"
#include "resip/dum/UserAuthInfo.hxx"
#include "rutil/Random.hxx"


#include <fstream>
#include <sstream>
#include "../SipExtensionInfo.h"

#define RESIPROCATE_SUBSYSTEM Subsystem::DUM

using namespace resip;
using namespace std;
SipServerAuthManager::SipServerAuthManager(DialogUsageManager& dum,
	Dispatcher* authRequestDispatcher,
	bool useAuthInt,
	bool rejectBadNonces,
	bool challengeThirdParties,
	int authtype,
	Data serverid,
	const Data& staticRealm) :
	ServerAuthManager(dum, dum.dumIncomingTarget(), challengeThirdParties, staticRealm), IServerID(serverid), authmethod(authtype),
	mDum(dum),
	mAuthRequestDispatcher(authRequestDispatcher),
	mUseAuthInt(useAuthInt),
	mRejectBadNonces(rejectBadNonces)
{
}
SipServerAuthManager::SipServerAuthManager(DialogUsageManager& dum,
	Dispatcher* authRequestDispatcher,
	bool useAuthInt,
	bool rejectBadNonces,
	bool challengeThirdParties,
	const Data& staticRealm) :
	ServerAuthManager(dum, dum.dumIncomingTarget(), challengeThirdParties, staticRealm), IServerID(""), authmethod(0),
	mDum(dum),
	mAuthRequestDispatcher(authRequestDispatcher),
	mUseAuthInt(useAuthInt),
	mRejectBadNonces(rejectBadNonces)
{
}
SipServerAuthManager::~SipServerAuthManager()
{
}
SipMessage* SipServerAuthManager::handleUserAuthInfo(UserAuthInfo* userAuth)
{
	resip_assert(userAuth);

	MessageMap::iterator it = mMessages.find(userAuth->getTransactionId());
	resip_assert(it != mMessages.end());
	SipMessage* requestWithAuth = it->second;
	mMessages.erase(it);

	InfoLog(<< "Checking for auth result in realm=" << userAuth->getRealm()
		<< " A1=" << userAuth->getA1());

	if (userAuth->getMode() == UserAuthInfo::UserUnknown ||
		(userAuth->getMode() == UserAuthInfo::RetrievedA1 && userAuth->getA1().empty()))
	{
		InfoLog(<< "User unknown " << userAuth->getUser() << " in " << userAuth->getRealm());
		std::shared_ptr<SipMessage> response(new SipMessage);
		Helper::makeResponse(*response, *requestWithAuth, 404, "User unknown.");
		mDum.send(std::move(response));
		onAuthFailure(BadCredentials, *requestWithAuth);
		delete requestWithAuth;
		return nullptr;
	}

	if (userAuth->getMode() == UserAuthInfo::Error)
	{
		InfoLog(<< "Error in auth procedure for " << userAuth->getUser() << " in " << userAuth->getRealm());
		std::shared_ptr<SipMessage> response(new SipMessage);
		Helper::makeResponse(*response, *requestWithAuth, 503, "Server Error.");
		mDum.send(std::move(response));
		onAuthFailure(Error, *requestWithAuth);
		delete requestWithAuth;
		return 0;
	}

	bool stale = false;
	bool digestAccepted = (userAuth->getMode() == UserAuthInfo::DigestAccepted);
	if (userAuth->getMode() == UserAuthInfo::RetrievedA1)
	{
		//!dcm! -- need to handle stale/unit test advancedAuthenticateRequest
		//!dcm! -- delta? deal with.

		std::pair<Helper::AuthResult, Data> resPair;
		if (authmethod > 0)
		{
			resPair = advancedAuthenticateRequestSm(*requestWithAuth, 3000, proxyAuthenticationMode());
		}
		else
		{
			resPair = Helper::advancedAuthenticateRequest(*requestWithAuth,
				userAuth->getRealm(),
				userAuth->getA1(),
				3000,
				proxyAuthenticationMode());
		}
		switch (resPair.first)
		{
		case Helper::Authenticated:
			digestAccepted = true;
			break;
		case Helper::Failed:
			// digestAccepted = false;   // already false by default
			break;
		case Helper::BadlyFormed:
			if (rejectBadNonces())
			{
				InfoLog(<< "Authentication nonce badly formed for " << userAuth->getUser());

				std::shared_ptr<SipMessage> response(new SipMessage);
				Helper::makeResponse(*response, *requestWithAuth, 403, "Invalid nonce");
				mDum.send(std::move(response));
				onAuthFailure(InvalidRequest, *requestWithAuth);
				delete requestWithAuth;
				return 0;
			}
			else
			{
				stale = true;
			}
			break;
		case Helper::Expired:
			stale = true;
			break;
		default:
			break;
		}
	}

	if (stale || userAuth->getMode() == UserAuthInfo::Stale)
	{
		InfoLog(<< "Nonce expired for " << userAuth->getUser());

		issueChallenge(requestWithAuth);
		delete requestWithAuth;
		return 0;
	}

	if (digestAccepted)
	{
		//if (authorizedForThisIdentity(userAuth->getUser(), userAuth->getRealm(),requestWithAuth->header(h_From).uri()))
		bool isok = false;
		if (authmethod == 0)
		{
			isok = authorizedForThisIdentity(userAuth->getUser(), userAuth->getRealm(), requestWithAuth->header(h_From).uri());
		}
		else
		{
			isok = authorizedForThisServerID(userAuth->getRealm());
		}
		if (isok)
		{
			InfoLog(<< "Authorized request for " << userAuth->getRealm());
			onAuthSuccess(*requestWithAuth);
			return requestWithAuth;
		}
		else
		{
			// !rwm! The user is trying to forge a request.  Respond with a 403
			InfoLog(<< "User: " << userAuth->getUser() << " at realm: " << userAuth->getRealm() <<
				" trying to forge request from: " << requestWithAuth->header(h_From).uri());

			std::shared_ptr<SipMessage> response(new SipMessage);
			Helper::makeResponse(*response, *requestWithAuth, 403, "Invalid user name provided");
			mDum.send(std::move(response));
			onAuthFailure(InvalidRequest, *requestWithAuth);
			delete requestWithAuth;
			return 0;
		}
	}
	else
	{
		// Handles digestAccepted == false, DigestNotAccepted and any other
		// case that is not recognised by the foregoing logic

		InfoLog(<< "Invalid password provided for " << userAuth->getUser() << " in " << userAuth->getRealm());
		InfoLog(<< "  a1 hash of password from db was " << userAuth->getA1());

		std::shared_ptr<SipMessage> response(new SipMessage);
		Helper::makeResponse(*response, *requestWithAuth, 403, "Invalid password provided");
		mDum.send(std::move(response));
		onAuthFailure(BadCredentials, *requestWithAuth);
		delete requestWithAuth;
		return 0;
	}
}
ServerAuthManager::Result SipServerAuthManager::handle(SipMessage* sipMsg)
{
	//InfoLog( << "trying to do auth" );
	if (sipMsg->isRequest())
	{
		if (sipMsg->method() == CANCEL)
		{
			// If we receive a cancel - check to see if we have the matching INVITE in our message map.
			// If we do, then we haven't created a DUM dialog for it yet, since we are still waiting
			// for the credential information to arrive.  We need to properly respond to the CANCEL here
			// since it won't be handled externally.
			MessageMap::iterator it = mMessages.find(sipMsg->getTransactionId());
			if (it != mMessages.end())
			{
				// Ensure message is an INVITE - if not then something fishy is going on.  Either
				// someone has cancelled a non-INVITE transaction or we have a tid collision.
				if (it->second->isRequest() && it->second->method() == INVITE)
				{
					std::unique_ptr<SipMessage> inviteMsg(it->second);
					mMessages.erase(it);  // Remove the INVITE from the message map and respond to it

					InfoLog(<< "Received a CANCEL for an INVITE request that we are still waiting on auth "
						<< "info for, responding appropriately, tid="
						<< sipMsg->getTransactionId());

					// Send 487/Inv
					std::shared_ptr<SipMessage> inviteResponse(new SipMessage);
					Helper::makeResponse(*inviteResponse, *inviteMsg, 487);  // Request Cancelled
					mDum.send(std::move(inviteResponse));

					// Send 200/Cancel
					std::shared_ptr<SipMessage> cancelResponse(new SipMessage);
					Helper::makeResponse(*cancelResponse, *sipMsg, 200);
					mDum.send(std::move(cancelResponse));

					return Rejected; // Use rejected since handling is what we want - stop DUM from processing the cancel any further
				}
			}
		}
		else if (sipMsg->method() != ACK)  // Do not challenge ACKs or CANCELs (picked off above)
		{
			if (sipMsg->method() == REGISTER)
			{
				ParserContainer<Auth>* auths;
				if (proxyAuthenticationMode())
				{
					if (!sipMsg->exists(h_ProxyAuthorizations))
					{
						return issueChallengeIfRequiredsm(sipMsg);
					}
					auths = &sipMsg->header(h_ProxyAuthorizations);
				}
				else
				{
					std::cout << "method:" << sipMsg->method() << "\n" << *sipMsg << std::endl;
					if (!sipMsg->exists(h_Authorizations))
					{
						return issueChallengeIfRequiredsm(sipMsg);
					}
					auths = &sipMsg->header(h_Authorizations);
				}

				try
				{
					for (Auths::iterator it = auths->begin(); it != auths->end(); it++)
					{
						if (authmethod == 0)
						{
							if (isMyRealm(it->param(p_realm)))
							{
								InfoLog(<< "Requesting credential for "
									<< it->param(p_username) << " @ " << it->param(p_realm));

								requestCredential(it->param(p_username),
									it->param(p_realm),
									*sipMsg,
									*it,
									sipMsg->getTransactionId());
								mMessages[sipMsg->getTransactionId()] = sipMsg;
								return RequestedCredentials;
							}
						}
						else
						{
							if (it->exists(p_serverid))
							{
								Data random1 = it->param(p_random1);
								Data random2 = it->param(p_random2);
								Data sign1 = it->param(p_sign1);
								Data serverid = it->param(p_serverid);

								requestCredentialsm(random1, random2, serverid, *sipMsg, *it, sipMsg->getTransactionId());
								mMessages[sipMsg->getTransactionId()] = sipMsg;

								return RequestedCredentials;
							}
						}
					}

					InfoLog(<< "Didn't find matching realm ");
					return issueChallengeIfRequiredsm(sipMsg);
				}
				catch (BaseException& e)
				{
					InfoLog(<< "Invalid auth header provided " << e);
					std::shared_ptr<SipMessage> response(new SipMessage);
					Helper::makeResponse(*response, *sipMsg, 400, "Invalid auth header");
					mDum.send(std::move(response));
					onAuthFailure(InvalidRequest, *sipMsg);
					return Rejected;
				}
			}
		}
	}
	return Skipped;
}
bool
SipServerAuthManager::proxyAuthenticationMode() const
{
	return mProxyAuthenticationMode;
}
void SipServerAuthManager::SetAuthenticationMode(bool isproxy)
{
	mProxyAuthenticationMode = isproxy;
}


bool
SipServerAuthManager::useAuthInt() const
{
	return mUseAuthInt;
}

bool
SipServerAuthManager::rejectBadNonces() const
{
	return mRejectBadNonces;
}

AsyncBool
SipServerAuthManager::requiresChallenge(const SipMessage& msg)
{
	resip_assert(msg.isRequest());
	if (1/*!mAclDb.isRequestTrusted(msg)*/)
	{
		return ServerAuthManager::requiresChallenge(msg);
	}
	else
	{
		return False;
	}
}

void
SipServerAuthManager::requestCredential(const Data& user,
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
void
SipServerAuthManager::requestCredentialsm(const resip::Data& random1, const resip::Data& random2, const resip::Data& serverid,
	const resip::SipMessage& msg, const resip::Auth& auth, const resip::Data& transactionId)
{
	// Build a UserAuthInfo object and pass to UserAuthGrabber to have a1 password filled in
	UserAuthInfo* async = new UserAuthInfo(random2, serverid, transactionId, &mDum);
	std::unique_ptr<ApplicationMessage> app(async);
	mAuthRequestDispatcher->post(app);
}
bool SipServerAuthManager::authorizedForThisIdentity(const resip::Data& user, const resip::Data& realm, resip::Uri& fromUri)
{
	// !rwm! good enough for now.  TODO eventually consult a database to see what
	// combinations of user/realm combos are authorized for an identity

	// First try the form where the username parameter in the auth
	// header is just the username component of the fromUri
	//
	if ((fromUri.user() == user) && (fromUri.host() == realm))
		return true;

	// Now try the form where the username parameter in the auth
	// header is the full fromUri, e.g.
	//    Proxy-Authorization: Digest username="user@domain" ...
	//
	if (fromUri.getAorNoPort() == user)
		return true;

	if (fromUri.user() == user)
		return true;

	// catch-all: access denied
	return false;
}
bool SipServerAuthManager::authorizedForThisServerID(const resip::Data& user)
{
	if (user == IServerID)
		return true;
	return false;
}
void SipServerAuthManager::onAuthSuccess(const SipMessage& msg)
{
	// sub class may want to create a log entry

}

void SipServerAuthManager::onAuthFailure(AuthFailureReason reason, const SipMessage& msg)
{
	// sub class may want to create a log entry
}
ServerAuthManager::Result
SipServerAuthManager::issueChallengeIfRequiredsm(SipMessage* sipMsg)
{
	// Is challenge required for this message
	AsyncBool required = requiresChallenge(*sipMsg);
	switch (required)
	{
	case False:
		return Skipped;
	case Async:
		mMessages[sipMsg->getTransactionId()] = sipMsg;
		return RequestedInfo;
	case True:
	default:
		issueChallengesm(sipMsg);
		return Challenged;
	}
}
void
SipServerAuthManager::issueChallengesm(SipMessage* sipMsg)
{
	//assume TransactionUser has matched/repaired a realm
	std::shared_ptr<SipMessage> challenge(Helper::makeChallenge(*sipMsg,
		getChallengeRealm(*sipMsg),
		useAuthInt(),
		false /*stale*/,
		proxyAuthenticationMode()));
	//35114
	if (authmethod > 0)
	{
		challenge->header(h_WWWAuthenticates).clear();

		Auth auth;
		if (authmethod == 1)
			auth.scheme() = AuthScheme;// Symbols::Digest;
		else if (authmethod == 2)
			auth.scheme() = AuthScheme_b;
		auth.param(p_algorithm) = "\"A:SM2;H:SM3;S:SM4/OFB/PKCS5;SI:SM3-SM2\"";// "MD5";
		Data rdm1 = "\"" + Random::getRandomHex(8) + "\"";
		auth.param(p_random1) = rdm1;

		challenge->header(h_WWWAuthenticates).push_back(auth);
	}
	InfoLog(<< "Sending challenge to " << sipMsg->brief());
	std::cout << "issueChallengesm:" << *challenge << std::endl;
	mDum.send(std::move(challenge));
}


std::pair<Helper::AuthResult, Data>
SipServerAuthManager::advancedAuthenticateRequestSm(SipMessage& request, int expiresDelta, bool proxyAuthorization)
{
	Data username;
	DebugLog(<< "Authenticating: realm=" << "" << " expires=" << expiresDelta);
	//DebugLog(<< request);
	std::cout << "advancedAuthenticateRequestSm\n" << request << std::endl;
	const ParserContainer<Auth>* auths = 0;
	if (proxyAuthorization)
	{
		if (request.exists(h_ProxyAuthorizations))
		{
			auths = &request.header(h_ProxyAuthorizations);
		}
	}
	else
	{
		if (request.exists(h_Authorizations))
		{
			auths = &request.header(h_Authorizations);
		}
	}

	if (auths)
	{
		for (ParserContainer<Auth>::const_iterator i = auths->begin(); i != auths->end(); i++)
		{
			if (i->exists(p_random1) &&
				i->exists(p_random2) &&
				i->exists(p_serverid)
				&& i->exists(p_sign1))
			{
				Data Scheme;
				if (authmethod == 1)
				{
					Scheme = AuthScheme;
				}
				else if (authmethod == 2)
				{
					Scheme = AuthScheme_b;
				}
				if (!isEqualNoCase(i->scheme(), Scheme))
				{
					DebugLog(<< "Scheme must be Digest");
					continue;
				}
				username = request.header(h_From).uri().user();
				Data random1 = i->param(p_random1);
				Data random2 = i->param(p_random2);
				Data serverid = i->param(p_serverid);
				Data sign1 = i->param(p_sign1);

				//auto devinfo = GetDevInfoPtrByGBID(username.c_str());
				//if(devinfo == NULL)
				//{
				//	return make_pair(Helper::Failed, username);
				//}
				////char downdevpblcbuf[256] = { 0 };
				////snprintf(downdevpblcbuf, 256, "/data/app/%s", devinfo->strPubKey.c_str());
				//FILE *fstrm = fopen(devinfo->strPubKey.c_str(), "rb");
				//if (fstrm == NULL)
				//{
				//	return make_pair(Helper::Failed, username);
				//}
				//fseek(fstrm, 0, SEEK_END);
				//int filelen = ftell(fstrm);
				//fseek(fstrm, 0, SEEK_SET);
				//char *pPbKeyBuf = new char[filelen+1];
				//fread(pPbKeyBuf, 1, filelen, fstrm);
				//fclose(fstrm);
				//pPbKeyBuf[filelen] = 0;

				//Data srcData = random2 + random1 + serverid;
				//LogOut("GB", L_DEBUG, "srcdat:%s, sign1:%s", srcData.c_str(), sign1.c_str());
				//std::string strsign1(sign1.base64decode().c_str(), sign1.base64decode().size());//用设备证书校验sign1签名结果
				//int nRet = Sm2Verify(pPbKeyBuf, strsign1, srcData.c_str());
				//if (nRet != 0 || serverid != IServerID)
				//{
				//	return make_pair(Helper::Failed, username);
				//}
				//delete[]pPbKeyBuf; pPbKeyBuf = NULL;
				////ok
				return make_pair(Helper::Authenticated, username);
			}
		}
		return make_pair(Helper::BadlyFormed, username);
	}
	DebugLog(<< "No authentication headers. Failing request.");
	return make_pair(Helper::Failed, username);
}