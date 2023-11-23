#ifndef SIP_SERVER_H_
#define SIP_SERVER_H_

#include "rutil/Data.hxx"
#include "rutil/ServerProcess.hxx"
#include "resip/dum/TlsPeerAuthManager.hxx"
#include "resip/stack/StatisticsHandler.hxx"
#include "resip/stack/DomainMatcher.hxx"
#include <memory>

#include "RegistrarServer/AuthenticatorFactory.hxx"
//#include "repro/Plugin.hxx"
#include "RegistrarServer/Registrar.hxx"
#include "UserAgent/UaMessageMgr.h"


using namespace regist;
namespace resip
{
	class TransactionUser;
	class SipStack;
	class Dispatcher;
	class RegistrationPersistenceManager;
	class PublicationPersistenceManager;
	class FdPollGrp;
	class AsyncProcessHandler;
	class ThreadIf;
	class DialogUsageManager;
	class CongestionManager;
	class UaMgr;
}
namespace regist
{
class ProxyConfig;
class ProcessorChain;
class AbstractDb;
class ProcessorChain;
class Proxy;
class WebAdmin;
class WebAdminThread;
class Registrar;
class CertServer;
class RegSyncClient;
class RegSyncServer;
class RegSyncServerThread;
//class CommandServer;
//class CommandServerThread;
class Processor;
class PresenceServer;
class CRegistServer;
}
namespace sipserver
{

class SipServer : public resip::ServerProcess,
	public resip::ExternalStatsHandler

{
public:
	SipServer();
	virtual ~SipServer();

	static SipServer *Instance();

	virtual bool run(int argc, char** argv);
	virtual void shutdown();
	virtual void restart();  // brings everydown and then backup again - leaves InMemoryRegistrationDb intact
	virtual void onReload();

	//virtual Proxy* getProxy() { return mProxy; }

	// External Stats handler
	virtual bool operator()(resip::StatisticsMessage& statsMessage);

protected:
	virtual void cleanupObjects();

	virtual bool loadPlugins();
	virtual void setOpenSSLCTXOptionsFromConfig(const resip::Data& configVar, long& opts);
	virtual bool createSipStack();
	virtual bool createDatastore();
	virtual bool createProxy();
	virtual void populateRegistrations();
	virtual bool createWebAdmin();
	virtual void createAuthenticatorFactory();
	virtual void createDialogUsageManager();//¡ä¡ä?¡§¡Á¡é2¨¢server dum  SipServerAuthManager
	virtual void createRegSync();
	virtual void createCommandServer();

	virtual bool createRegistServer();

	virtual void initDomainMatcher();
	virtual void addDomains(resip::TransactionUser& tu);
	virtual bool addTransports(bool& allTransportsSpecifyRecordRoute);
	// Override this and examine the processor name to selectively add custom processors before or after the standard ones
	//virtual void addProcessor(repro::ProcessorChain& chain, std::unique_ptr<repro::Processor> processor);
	//virtual void makeRequestProcessorChain(repro::ProcessorChain& chain);
	//virtual void makeResponseProcessorChain(repro::ProcessorChain& chain);
	//virtual void makeTargetProcessorChain(repro::ProcessorChain& chain);

	bool mRunning;
	bool mRestarting;
	int mArgc;
	char** mArgv;
	bool mThreadedStack;
	resip::Data mHttpRealm;
	bool mUseV4;
	bool mUseV6;
	int mRegSyncPort;
	ProxyConfig* mProxyConfig;
	resip::FdPollGrp* mFdPollGrp;
	resip::AsyncProcessHandler* mAsyncProcessHandler;
	resip::SipStack* mSipStack;
	resip::ThreadIf* mStackThread;
	AbstractDb* mAbstractDb;
	AbstractDb* mRuntimeAbstractDb;
	resip::RegistrationPersistenceManager* mRegistrationPersistenceManager;
	resip::PublicationPersistenceManager* mPublicationPersistenceManager;
	AuthenticatorFactory* mAuthFactory;
	resip::Dispatcher* mAsyncProcessorDispatcher;
	ProcessorChain* mMonkeys;
	ProcessorChain* mLemurs;
	ProcessorChain* mBaboons;
	Proxy* mProxy;
	std::list<WebAdmin*> mWebAdminList;
	WebAdminThread* mWebAdminThread;
	Registrar* mRegistrar;
	PresenceServer* mPresenceServer;
	resip::DialogUsageManager* mDum;
	resip::ThreadIf* mDumThread;
	CertServer* mCertServer;
	RegSyncClient* mRegSyncClient;
	RegSyncServer* mRegSyncServerV4;
	RegSyncServer* mRegSyncServerV6;
	RegSyncServer* mRegSyncServerAMQP;
	RegSyncServerThread* mRegSyncServerThread;
	//std::list<CommandServer*> mCommandServerList;
	//CommandServerThread* mCommandServerThread;
	resip::CongestionManager* mCongestionManager;
	//std::vector<Plugin*> mPlugins;
	typedef std::map<unsigned int, resip::NameAddr> TransportRecordRouteMap;
	TransportRecordRouteMap mStartupTransportRecordRoutes;
	std::shared_ptr<resip::DomainMatcher> mDomainMatcher;
	resip::Data mDefaultRealm;

	resip::UaMgr* mUserAgent;
	CRegistServer* mRegistSv;

	CUserMessageMrg* mMessageMgr;
private:
	static SipServer* g_server;
public:
	resip::UaMgr* GetUaManager() { return mUserAgent; };
	CUserMessageMrg* GetMsgManager() { return mMessageMgr; };

	int getQDCCTVNodeInfo(std::string& upID, std::string& upHost, int& upPort, std::string& upPassword);
public:

	std::string zlmHost;
	int zlmHttpPort;
	int gbHttpPort;

	Data localHost;
	int localPort;
};
}
sipserver::SipServer* GetServer();
#endif
