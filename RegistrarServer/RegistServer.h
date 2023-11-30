#ifndef REGIST_SERVER_H_
#define REGIST_SERVER_H_


#include "repro/Registrar.hxx"
#include "resip/stack/Dispatcher.hxx"
#include "repro/UserAuthGrabber.hxx"
#include "resip/dum/ServerRegistration.hxx"
#include "resip/stack/DomainMatcher.hxx"

#include "repro/AuthenticatorFactory.hxx"

#include "repro/Store.hxx"
#include "repro/AsyncProcessor.hxx"
#include <list>
#include <iostream>
#include <regex>

using namespace resip;
using namespace std;

namespace repro
{
class ProxyConfig;
class CertServer;
class PresenceServer;

class MyRegistrarHandler :	public AsyncProcessor,
							public RegistrarHandler
{
	resip::Uri GetSrcUri(const resip::SipMessage& reg);
	int authenmethod;
	AbstractDb* mAbstractDb;
public:
	MyRegistrarHandler(ProxyConfig& config, resip::Dispatcher* asyncDispatcher);
	~MyRegistrarHandler(void);

	virtual processor_action_t process(RequestContext&);
	virtual bool asyncProcess(AsyncProcessorMessage* msg);

	void SetAbstractDb(AbstractDb* db) { mAbstractDb = db; };
	virtual bool onRefresh(resip::ServerRegistrationHandle, const resip::SipMessage& reg);
	virtual bool onRemove(resip::ServerRegistrationHandle, const resip::SipMessage& reg);
	virtual bool onRemoveAll(resip::ServerRegistrationHandle, const resip::SipMessage& reg);
	virtual bool onAdd(resip::ServerRegistrationHandle, const resip::SipMessage& reg);
	virtual bool onQuery(resip::ServerRegistrationHandle, const resip::SipMessage& reg) { return true; };
	void RequestStreamTask(std::string Id, std::string ip, int port);
private:
	SiloStore& mSiloStore;
	std::regex* mDestFilterRegex;
	std::regex* mMimeTypeFilterRegex;
	unsigned long mExpirationTime;
	bool mAddDateHeader;
	unsigned long mMaxContentLength;
	unsigned short mSuccessStatusCode;
	unsigned short mFilteredMimeTypeStatusCode;
	unsigned short mFailureStatusCode;
	time_t mLastSiloCleanupTime;
};

class CRegistServer
{
public:
	CRegistServer(ProxyConfig* config, resip::SipStack& tsipstack);
	//CRegistServer(resip::SipStack& tsipstack, resip::DialogUsageManager* rgdum, int authtype = 0, Data serverid = "");
	~CRegistServer();
	bool Setup();
	void Shutdown();
	bool InitRegistServer();
	DialogUsageManager* Dum() { return m_RegistDum; }
	virtual void cleanupRegistServerObjects();
	virtual bool CreateRegistDum();
	virtual void createAuthenticatorFactory();
	virtual bool createDatastore();
	virtual void initDomainMatcher();
	virtual void addDomains(resip::TransactionUser& tu);
	virtual void populateRegistrations();
protected:

	//bool InitRegistDum();
	std::list<Data> MyDomainList;

private:
	SipStack& m_SipStack;

	//MyRegistrarHandler mMyRisgtHandler;
	int mRegSyncPort;
	ProxyConfig *SvConfig;
	AbstractDb* mAbstractDb;
	AbstractDb* mRuntimeAbstractDb;

	Registrar* mRegistrar;
	DialogUsageManager* m_RegistDum;
	ThreadIf* mRegistDumThread;
	AuthenticatorFactory* mAuthFactory;
	resip::RegistrationPersistenceManager* mRegistrationPersistenceManager;
	resip::PublicationPersistenceManager* mPublicationPersistenceManager;
	shared_ptr<resip::DomainMatcher> mDomainMatcher;
	resip::Dispatcher* mAsyncProcessorDispatcher;

	Data mDefaultRealm;

	CertServer* mCertServer;
	PresenceServer* mPresenceServer;

	MyRegistrarHandler *mMyRisgtHandler;
};
}

#endif