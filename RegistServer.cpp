
#include "RegistServer.h"
#include "SipServer/ReproAuthenticatorFactory.hxx"

#include "resip/stack/InteropHelper.hxx"
#include "rutil/ResipAssert.h"
#include "resip/dum/DumThread.hxx"
#include "resip/stack/ExtendedDomainMatcher.hxx"
#include "rutil/DnsUtil.hxx"
#include "rutil/Log.hxx"
#include "rutil/Logger.hxx"
#include "resip/dum/InMemorySyncRegDb.hxx"
#include "resip/dum/InMemorySyncPubDb.hxx"
#include "resip/dum/WsCookieAuthManager.hxx"
#include "resip/stack/WsCookieContextFactory.hxx"
#include "resip/stack/DateCategory.hxx"
#include "resip/stack/PlainContents.hxx"
#include "resip/stack/Helper.hxx"

#include <list>
#include <iostream>

#if defined(USE_MYSQL)
#include "../MySqlDb.hxx"
#endif

#include "SqliteDb.h"
#include "device/DeviceInfo.h"
#include "SipServer/AsyncProcessorMessage.hxx"
#include "SipServer/RequestContext.hxx"
#include "SipServer/AsyncProcessorWorker.hxx"
#include "SipExtensionInfo.h"
#include "SipServerConfig.h"
#include "device/DeviceManager.h"
#include "tools/m_Time.h"
#include "tools/ownString.h"


#define RESIPROCATE_SUBSYSTEM resip::Subsystem::REPRO

#define SILO_CLEANUP_PERIOD 86400

using namespace resip;
using namespace repro;
using namespace std;

CRegistServer::CRegistServer(ProxyConfig* config, resip::SipStack& tsipstack):SvConfig(config)
, m_SipStack(tsipstack)
, mAbstractDb(NULL)
, mRuntimeAbstractDb(NULL)
, mRegistrar(NULL)
, m_RegistDum(NULL)
, mRegistDumThread(NULL)
, mAuthFactory(NULL)
, mRegistrationPersistenceManager(NULL)
, mPublicationPersistenceManager(NULL)
, mCertServer(NULL)
, mPresenceServer(NULL)
, mMyRisgtHandler(NULL)
, mAsyncProcessorDispatcher(NULL)
{
    int numAsyncProcessorWorkerThreads = SvConfig->getConfigInt("NumAsyncProcessorWorkerThreads", 2);
    if (numAsyncProcessorWorkerThreads > 0)
    {
        resip_assert(!mAsyncProcessorDispatcher);
        mAsyncProcessorDispatcher = new Dispatcher(std::unique_ptr<Worker>(new AsyncProcessorWorker),
            &m_SipStack,
            numAsyncProcessorWorkerThreads);
    }
}
CRegistServer::~CRegistServer()
{
    mRegistDumThread->join();
    cleanupRegistServerObjects();
}
bool CRegistServer::CreateRegistDum()
{
    shared_ptr<MasterProfile> profile(new MasterProfile);
    profile->setRportEnabled(InteropHelper::getRportEnabled());
    profile->clearSupportedMethods();
    profile->addSupportedMethod(resip::REGISTER);
#ifdef USE_SSL
    profile->addSupportedScheme(Symbols::Sips);
#endif
    if (InteropHelper::getOutboundSupported())
    {
        profile->addSupportedOptionTag(Token(Symbols::Outbound));
    }
    profile->addSupportedOptionTag(Token(Symbols::Path));
    if (SvConfig->getConfigBool("AllowBadReg", false))
    {
        profile->allowBadRegistrationEnabled() = true;
    }
#ifdef PACKAGE_VERSION
    Data serverText(SvConfig->getConfigData("ServerText", "repro " PACKAGE_VERSION));
#else
    Data serverText(SvConfig->getConfigData("ServerText", Data::Empty));
#endif
    if (!serverText.empty())
    {
        profile->setUserAgent(serverText);
    }
    // Create DialogeUsageManager if Registrar or Certificate Server are enabled
    resip_assert(!mRegistrar);
    resip_assert(!m_RegistDum);
    resip_assert(!mRegistDumThread);
    mRegistrar = new Registrar;
    resip::MessageFilterRuleList ruleList;
    bool registrarEnabled = !SvConfig->getConfigBool("DisableRegistrar", false);
    bool certServerEnabled = SvConfig->getConfigBool("EnableCertServer", false);
    bool presenceEnabled = SvConfig->getConfigBool("EnablePresenceServer", false);
    if (registrarEnabled || certServerEnabled || presenceEnabled)
    {
        m_RegistDum = new DialogUsageManager(m_SipStack);
        m_RegistDum->setMasterProfile(profile);
        addDomains(*m_RegistDum);
    }

    // If registrar is enabled, configure DUM to handle REGISTER requests
    if (registrarEnabled)
    {
        resip_assert(m_RegistDum);
        resip_assert(mRegistrationPersistenceManager);
        mMyRisgtHandler = new MyRegistrarHandler(*SvConfig, mAsyncProcessorDispatcher);
        mRegistrar->addRegistrarHandler(mMyRisgtHandler);
        m_RegistDum->setServerRegistrationHandler(mRegistrar);
        m_RegistDum->setRegistrationPersistenceManager(mRegistrationPersistenceManager);

        // Install rules so that the registrar only gets REGISTERs
        resip::MessageFilterRule::MethodList methodList;
        methodList.push_back(resip::REGISTER);
        ruleList.push_back(MessageFilterRule(resip::MessageFilterRule::SchemeList(),
            resip::MessageFilterRule::DomainIsMe,
            methodList));
    }

    // If Certificate Server is enabled, configure DUM to handle SUBSCRIBE and 
    // PUBLISH requests for events: credential and certificate
    resip_assert(!mCertServer);
    if (certServerEnabled)
    {
#if defined(USE_SSL)
        mCertServer = new CertServer(*m_RegistDum);

        // Install rules so that the cert server receives SUBSCRIBEs and PUBLISHs
        resip::MessageFilterRule::MethodList methodList;
        resip::MessageFilterRule::EventList eventList;
        methodList.push_back(resip::SUBSCRIBE);
        methodList.push_back(resip::PUBLISH);
        eventList.push_back(resip::Symbols::Credential);
        eventList.push_back(resip::Symbols::Certificate);
        ruleList.push_back(MessageFilterRule(resip::MessageFilterRule::SchemeList(),
            resip::MessageFilterRule::DomainIsMe,
            methodList,
            eventList));
#endif
    }

    if (presenceEnabled)
    {
        resip_assert(m_RegistDum);
        resip_assert(mPublicationPersistenceManager);

        // Set the publication persistence manager in dum
        m_RegistDum->setPublicationPersistenceManager(mPublicationPersistenceManager);

        // Configure DUM to handle SUBSCRIBE and PUBLISH requests for presence
        /*mPresenceServer = new PresenceServer(*m_RegistDum, mAuthFactory->getDispatcher(),
            SvConfig->getConfigBool("PresenceUsesRegistrationState", true),
            SvConfig->getConfigBool("PresenceNotifyClosedStateForNonPublishedUsers", true));*/

            // Install rules so that the cert server receives SUBSCRIBEs and PUBLISHs
        MessageFilterRule::MethodList methodList;
        MessageFilterRule::EventList eventList;
        methodList.push_back(SUBSCRIBE);
        methodList.push_back(PUBLISH);
        eventList.push_back(Symbols::Presence);
        ruleList.push_back(MessageFilterRule(MessageFilterRule::SchemeList(),
            MessageFilterRule::DomainIsMe,
            methodList,
            eventList));
    }

    if (m_RegistDum)
    {
        resip_assert(mAuthFactory);
        mAuthFactory->setDum(m_RegistDum);

        if (mAuthFactory->certificateAuthEnabled())
        {
            // TODO: perhaps this should be initialised from the trusted node
            // monkey?  Or should the list of trusted TLS peers be independent
            // from the trusted node list?
            m_RegistDum->addIncomingFeature(mAuthFactory->getCertificateAuthManager());
        }

        Data wsCookieAuthSharedSecret = SvConfig->getConfigData("WSCookieAuthSharedSecret", Data::Empty);
        if (!mAuthFactory->digestAuthEnabled() && !wsCookieAuthSharedSecret.empty())
        {
            shared_ptr<WsCookieAuthManager> cookieAuth(new WsCookieAuthManager(*m_RegistDum, m_RegistDum->dumIncomingTarget()));
            m_RegistDum->addIncomingFeature(cookieAuth);
        }

        // If Authentication is enabled, then configure DUM to authenticate requests
        if (mAuthFactory->digestAuthEnabled())
        {
            //SipServerAuthManager
            m_RegistDum->setServerAuthManager(mAuthFactory->getServerAuthManager());
        }

        // Set the MessageFilterRuleList on DUM and create a thread to run DUM in
        m_RegistDum->setMessageFilterRuleList(ruleList);
        mRegistDumThread = new DumThread(*m_RegistDum);
    }
    return !!m_RegistDum;
}
void CRegistServer::createAuthenticatorFactory()
{
    // TODO: let a plugin supply an instance of AuthenticatorFactory
    // instead of our builtin ReproAuthenticatorFactory
    mAuthFactory = new ReproAuthenticatorFactory(*SvConfig, m_SipStack, NULL);
}
bool CRegistServer::createDatastore()
{
    // Create Database access objects
    resip_assert(!mAbstractDb);
    resip_assert(!mRuntimeAbstractDb);
    int defaultDatabaseIndex = SvConfig->getConfigInt("DefaultDatabase", -1);
    if (defaultDatabaseIndex >= 0)
    {
        mAbstractDb = SvConfig->getDatabase(defaultDatabaseIndex);
        if (!mAbstractDb)
        {
            CritLog(<< "Failed to get configuration database");
            cleanupRegistServerObjects();
            return false;
        }
        SqliteDb* pdb = dynamic_cast<SqliteDb*>(mAbstractDb);
        MyServerConfig m = GetSipServerConfig();
        printf("****************\n");
    }
    else     // Try legacy configuration parameter names
    {
#ifdef USE_MYSQL
        Data mySQLServer;
        SvConfig->getConfigValue("MySQLServer", mySQLServer);
        if (!mySQLServer.empty())
        {
            WarningLog(<< "Using deprecated parameter MySQLServer, please update to indexed Database definitions.");
            mAbstractDb = new MySqlDb(*SvConfig, mySQLServer,
                SvConfig->getConfigData("MySQLUser", Data::Empty),
                SvConfig->getConfigData("MySQLPassword", Data::Empty),
                SvConfig->getConfigData("MySQLDatabaseName", Data::Empty),
                SvConfig->getConfigUnsignedLong("MySQLPort", 0),
                SvConfig->getConfigData("MySQLCustomUserAuthQuery", Data::Empty));
        }
#endif
        if (!mAbstractDb)
        {
            //mAbstractDb = new BerkeleyDb(mServerConfig->getConfigData("DatabasePath", "./", true));
            mAbstractDb = SvConfig->getDatabase(1);
        }
    }
    int runtimeDatabaseIndex = SvConfig->getConfigInt("RuntimeDatabase", -1);
    if (runtimeDatabaseIndex >= 0)
    {
        mRuntimeAbstractDb = SvConfig->getDatabase(runtimeDatabaseIndex);
        if (!mRuntimeAbstractDb || !mRuntimeAbstractDb->isSane())
        {
            CritLog(<< "Failed to get runtime database");
            cleanupRegistServerObjects();
            return false;
        }
    }
#ifdef USE_MYSQL
    else     // Try legacy configuration parameter names
    {
        Data runtimeMySQLServer;
        SvConfig->getConfigValue("RuntimeMySQLServer", runtimeMySQLServer);
        if (!runtimeMySQLServer.empty())
        {
            WarningLog(<< "Using deprecated parameter RuntimeMySQLServer, please update to indexed Database definitions.");
            mRuntimeAbstractDb = new MySqlDb(*SvConfig, runtimeMySQLServer,
                SvConfig->getConfigData("RuntimeMySQLUser", Data::Empty),
                SvConfig->getConfigData("RuntimeMySQLPassword", Data::Empty),
                SvConfig->getConfigData("RuntimeMySQLDatabaseName", Data::Empty),
                SvConfig->getConfigUnsignedLong("RuntimeMySQLPort", 0),
                SvConfig->getConfigData("MySQLCustomUserAuthQuery", Data::Empty));
        }
    }
#endif
    resip_assert(mAbstractDb);
    if (!mAbstractDb->isSane())
    {
        CritLog(<< "Failed to open configuration database");
        cleanupRegistServerObjects();
        return false;
    }
    if (mRuntimeAbstractDb && !mRuntimeAbstractDb->isSane())
    {
        CritLog(<< "Failed to open runtime configuration database");
        cleanupRegistServerObjects();
        return false;
    }
    SvConfig->createDataStore(mAbstractDb, mRuntimeAbstractDb);

    // Create ImMemory Registration Database
    mRegSyncPort = SvConfig->getConfigInt("RegSyncPort", 0);
    // We only need removed records to linger if we have reg sync enabled
    //if (!mRestarting)  // If we are restarting then we left the InMemorySyncRegDb and InMemorySyncPubDb intact at restart - don't recreate
    {
        resip_assert(!mRegistrationPersistenceManager);
        mRegistrationPersistenceManager = new InMemorySyncRegDb(mRegSyncPort ? 86400 /* 24 hours */ : 0 /* removeLingerSecs */);  // !slg! could make linger time a setting
        resip_assert(!mPublicationPersistenceManager);
        mPublicationPersistenceManager = new InMemorySyncPubDb((mRegSyncPort && SvConfig->getConfigBool("EnablePublicationReplication", false)) ? true : false);
    }
    resip_assert(mRegistrationPersistenceManager);
    resip_assert(mPublicationPersistenceManager);

    // Copy contacts from the StaticRegStore to the RegistrationPersistanceManager
    populateRegistrations();

    return true;
}
bool CRegistServer::Setup()
{
    if (mRegistDumThread)
    {
        mRegistDumThread->run();
        return true;
    }
    return false;
}
void CRegistServer::initDomainMatcher()
{
    shared_ptr<ExtendedDomainMatcher> matcher(new ExtendedDomainMatcher());
    mDomainMatcher = matcher;

     std::vector<Data> configDomains;
     if (SvConfig->getConfigValue("Domains", configDomains))
     {
         for (std::vector<Data>::const_iterator i = configDomains.begin();
             i != configDomains.end(); ++i)
         {
             InfoLog(<< "Adding domain " << *i << " from command line");
             matcher->addDomain(*i);
             if (mDefaultRealm.empty())
             {
                 mDefaultRealm = *i;
             }
         }
     }

     std::vector<Data> configDomainSuffixes;
     if (SvConfig->getConfigValue("DomainSuffixes", configDomainSuffixes))
     {
         for (std::vector<Data>::const_iterator i = configDomainSuffixes.begin();
             i != configDomainSuffixes.end(); ++i)
         {
             InfoLog(<< "Adding domain suffix " << *i << " from command line");
             matcher->addDomainSuffix(*i);
             if (mDefaultRealm.empty())
             {
                 mDefaultRealm = *i;
             }
         }
     }

     const ConfigStore::ConfigData& dList = SvConfig->getDataStore()->mConfigStore.getConfigs();
     for (ConfigStore::ConfigData::const_iterator i = dList.begin();
         i != dList.end(); ++i)
     {
         InfoLog(<< "Adding domain " << i->second.mDomain << " from config");
         matcher->addDomain(i->second.mDomain);
         if (mDefaultRealm.empty())
         {
             mDefaultRealm = i->second.mDomain;
         }
     }

     /* All of this logic has been commented out - the sysadmin must explicitly
        add any of the items below to the Domains config option in repro.config*/

    /*Data localhostname(DnsUtil::getLocalHostName());
    InfoLog(<< "Adding local hostname domain " << localhostname);
    matcher->addDomain(localhostname);
    mDefaultRealm = SvConfig->getConfigData("DefaultRealm", mDefaultRealm);
    if (mDefaultRealm.empty())
    {
        mDefaultRealm = localhostname;
    }

    InfoLog(<< "Adding localhost domain.");
    matcher->addDomain("localhost");
    if (mDefaultRealm.empty())
    {
        mDefaultRealm = "localhost";
    }

    list<pair<Data, Data> > ips = DnsUtil::getInterfaces();
    for (list<pair<Data, Data> >::const_iterator i = ips.begin(); i != ips.end(); i++)
    {
        InfoLog(<< "Adding domain for IP " << i->second << " from interface " << i->first);
        matcher->addDomain(i->second);
    }

    InfoLog(<< "Adding 127.0.0.1 domain.");
    matcher->addDomain("127.0.0.1");*/

    if (mDefaultRealm.empty())
    {
        mDefaultRealm = "Unconfigured";
    }
}
void CRegistServer::addDomains(resip::TransactionUser& tu)
{
    if (mDomainMatcher.get() == 0)
    {
        initDomainMatcher();
    }
    tu.setDomainMatcher(mDomainMatcher);
}
void CRegistServer::cleanupRegistServerObjects()
{
    //delete mRegSyncServerThread; mRegSyncServerThread = 0;
    //delete mRegSyncServerAMQP; mRegSyncServerAMQP = 0;
    //delete mRegSyncServerV6; mRegSyncServerV6 = 0;
    //delete mRegSyncServerV4; mRegSyncServerV4 = 0;
    //delete mRegSyncClient; mRegSyncClient = 0;
#if defined(USE_SSL)
    delete mCertServer; mCertServer = 0;
#endif
    delete mRegistDumThread; mRegistDumThread = 0;
    delete m_RegistDum; m_RegistDum = 0;
    delete mRegistrar; mRegistrar = 0;
    //delete mPresenceServer; mPresenceServer = 0;
    

    
    delete mAuthFactory; mAuthFactory = 0;
    //delete mAsyncProcessorDispatcher; mAsyncProcessorDispatcher = 0;
    //if (!mRestarting)
    {
        // If we are restarting then leave the In Memory Registration and Publication database intact
        delete mRegistrationPersistenceManager; mRegistrationPersistenceManager = 0;
        delete mPublicationPersistenceManager; mPublicationPersistenceManager = 0;
    }
    delete mAbstractDb; mAbstractDb = 0;
    delete mRuntimeAbstractDb; mRuntimeAbstractDb = 0;
 
}
void CRegistServer::Shutdown()
{
    if (mRegistDumThread)
    {
        mRegistDumThread->shutdown();
    } 
}
bool CRegistServer::InitRegistServer()
{
    if (!createDatastore())
    {
        return false;
    }
    
    createAuthenticatorFactory();

    if (!CreateRegistDum())
    {
        return false;
    }
    mMyRisgtHandler->SetAbstractDb(mAbstractDb);
    return true;
}
void CRegistServer::populateRegistrations()
{
    resip_assert(mRegistrationPersistenceManager);
    resip_assert(SvConfig);
    resip_assert(SvConfig->getDataStore());

    // Copy contacts from the StaticRegStore to the RegistrationPersistanceManager
    StaticRegStore::StaticRegRecordMap& staticRegList = SvConfig->getDataStore()->mStaticRegStore.getStaticRegList();
    StaticRegStore::StaticRegRecordMap::iterator it = staticRegList.begin();
    for (; it != staticRegList.end(); it++)
    {
        try
        {
            Uri aor(it->second.mAor);

            ContactInstanceRecord rec;
            rec.mContact = NameAddr(it->second.mContact);
            rec.mSipPath = NameAddrs(it->second.mPath);
            rec.mRegExpires = NeverExpire;
            rec.mSyncContact = true;  // Tag this permanent contact as being a synchronized contact so that it will
                                      // not be synchronized to a paired server (this is actually configuration information)
            mRegistrationPersistenceManager->updateContact(aor, rec);
        }
        catch (resip::ParseBuffer::Exception& e)
        {
            // This should never happen, since the format should be verified before writing to DB
            ErrLog(<< "Failed to apply a static registration due to parse error: " << e);
        }
    }
}
class AsyncAddToSiloMessage : public AsyncProcessorMessage
{
public:
    AsyncAddToSiloMessage(AsyncProcessor& proc,
        const resip::Data& tid,
        resip::TransactionUser* passedtu) :
        AsyncProcessorMessage(proc, tid, passedtu)
    {
    }

    virtual EncodeStream& encode(EncodeStream& strm) const { strm << "AsyncAddToSiloMessage(tid=" << mTid << ", aor=" << mDestUri << ")"; return strm; }

    Data mDestUri;
    Data mSourceUri;
    time_t mOriginalSendTime;
    Data mMimeType;
    Data mMessageBody;
};

class AsyncDrainSiloMessage : public AsyncProcessorMessage
{
public:
    AsyncDrainSiloMessage(AsyncProcessor& proc,
        const resip::Data& tid,
        resip::TransactionUser* passedtu) :
        AsyncProcessorMessage(proc, tid, passedtu)
    {
    }

    virtual EncodeStream& encode(EncodeStream& strm) const { strm << "AsyncDrainSiloMessage(aor=" << mAor << ")"; return strm; }

    Data mAor;
    ContactList mRequestContacts;
};
MyRegistrarHandler::MyRegistrarHandler(ProxyConfig& config, Dispatcher* asyncDispatcher) :
    AsyncProcessor("MessageSilo", asyncDispatcher),
    mSiloStore(config.getDataStore()->mSiloStore),
    mDestFilterRegex(0),
    mMimeTypeFilterRegex(0),
    mExpirationTime(config.getConfigUnsignedLong("MessageSiloExpirationTime", 2592000 /* 30 days */)),
    mAddDateHeader(config.getConfigBool("MessageSiloAddDateHeader", true)),
    mMaxContentLength(config.getConfigUnsignedLong("MessageSiloMaxContentLength", 4096)),
    mSuccessStatusCode(config.getConfigUnsignedShort("MessageSiloSuccessStatusCode", 202)),
    mFilteredMimeTypeStatusCode(config.getConfigUnsignedShort("MessageSiloFilteredMimeTypeStatusCode", 200)),
    mFailureStatusCode(config.getConfigUnsignedShort("MessageSiloFailureStatusCode", 480)),
    mLastSiloCleanupTime(time(0))  // set to now
    , authenmethod(0), mAbstractDb(NULL)
{
    Data destFilterRegex = config.getConfigData("MessageSiloDestFilterRegex", "", false);
    Data mimeTypeFilterRegex = config.getConfigData("MessageSiloMimeTypeFilterRegex", "application\\/im\\-iscomposing\\+xml", false);
    if (!destFilterRegex.empty())
    {
        mDestFilterRegex = new regex_t;
        int ret = regcomp(mDestFilterRegex, destFilterRegex.c_str(), REG_EXTENDED | REG_NOSUB);
        if (ret != 0)
        {
            delete mDestFilterRegex;
            ErrLog(<< "MessageSilo has invalid destination filter regular expression: " << destFilterRegex);
            mDestFilterRegex = 0;
        }
    }
    if (!mimeTypeFilterRegex.empty())
    {
        mMimeTypeFilterRegex = new regex_t;
        int ret = regcomp(mMimeTypeFilterRegex, mimeTypeFilterRegex.c_str(), REG_EXTENDED | REG_NOSUB);
        if (ret != 0)
        {
            delete mMimeTypeFilterRegex;
            ErrLog(<< "MessageSilo has invalid mime-type filter regular expression: " << mimeTypeFilterRegex);
            mMimeTypeFilterRegex = 0;
        }
    }
}

MyRegistrarHandler::~MyRegistrarHandler()
{
    // Clean up pcre memory
    if (mDestFilterRegex)
    {
        regfree(mDestFilterRegex);
        delete mDestFilterRegex;
        mDestFilterRegex = 0;
    }
    if (mMimeTypeFilterRegex)
    {
        regfree(mMimeTypeFilterRegex);
        delete mMimeTypeFilterRegex;
        mMimeTypeFilterRegex = 0;
    }
}
Processor::processor_action_t
MyRegistrarHandler::process(RequestContext& context)
{
    DebugLog(<< "Monkey handling request: " << *this << "; reqcontext = " << context);
    SipMessage& originalRequest = context.getOriginalRequest();

    // Note:  A potential enhancement could be to also silo messages that fail to route due to a 
    // 408 or 503 error.  In order to do this, this processor needs to be part of the ResponseChain
    // as well.

    // Check if request is a MESSAGE request and if there were no targets found
    if (originalRequest.method() == MESSAGE &&
        !context.getResponseContext().hasTargets())
    {
        // There are no targets for this request - silo candidate

        // Only need to silo if there is a message body
        Contents* contents = originalRequest.getContents();
        if (contents)
        {
            // Create async message now, so we can use it's storage and avoid some copies
            AsyncAddToSiloMessage* async = NULL;// = new AsyncAddToSiloMessage(*this, context.getTransactionId(), &context.getProxy());
            std::unique_ptr<ApplicationMessage> async_ptr(async);

            // Check Max ContentLength setting
            async->mMessageBody = contents->getBodyData();
            if (async->mMessageBody.size() > mMaxContentLength)
            {
                InfoLog(<< " MESSAGE not silo'd due to content-length exceeding max: " << async->mMessageBody.size());
                SipMessage response;
                Helper::makeResponse(response, originalRequest, mFailureStatusCode);
                context.sendResponse(response);
                return SkipThisChain;
            }

            // Check if message passes Mime-type filter
            async->mMimeType = Data::from(contents->getType());
            if (mMimeTypeFilterRegex)
            {
                int ret = regexec(mMimeTypeFilterRegex, async->mMimeType.c_str(), 0, 0, 0/*eflags*/);
                if (ret == 0)
                {
                    // match 
                    DebugLog(<< " MESSAGE not silo'd due to Mime-Type filter: " << async->mMimeType);
                    if (mFilteredMimeTypeStatusCode == 0)
                    {
                        return Processor::Continue;
                    }
                    else
                    {
                        SipMessage response;
                        Helper::makeResponse(response, originalRequest, mFilteredMimeTypeStatusCode);
                        context.sendResponse(response);
                        return SkipThisChain;
                    }
                }
            }

            // Check if message passes Destination filter
            async->mDestUri = originalRequest.header(h_To).uri().getAOR(false /* addPort? */);
            if (mDestFilterRegex)
            {
                int ret = regexec(mDestFilterRegex, async->mDestUri.c_str(), 0, 0, 0/*eflags*/);
                if (ret == 0)
                {
                    // match 
                    DebugLog(<< " MESSAGE not silo'd due to destination filter: " << async->mDestUri);
                    return Processor::Continue;
                }
            }

            // TODO (future) - check a max messages per user setting

            NameAddr from(originalRequest.header(h_From));
            from.remove(p_tag); // remove from tag
            async->mSourceUri = Data::from(from);
            time(&async->mOriginalSendTime);  // Get now timestamp

            // Dispatch async request to worker thread pool
            mAsyncDispatcher->post(async_ptr);

            SipMessage response;
            InfoLog(<< "Message was Silo'd responding with a " << mSuccessStatusCode);
            Helper::makeResponse(response, context.getOriginalRequest(), mSuccessStatusCode);
            context.sendResponse(response);
            return SkipThisChain;
        }
    }

    // In all cases we continue - this is just a passive monkey that stores MESSAGES for later replay to recipient
    return Processor::Continue;
}

bool
MyRegistrarHandler::asyncProcess(AsyncProcessorMessage* msg)
{
    // Running inside a worker thread here
    AsyncAddToSiloMessage* addToSilo = dynamic_cast<AsyncAddToSiloMessage*>(msg);
    if (addToSilo)
    {
        // Check if database cleanup period has passed, and if so run a cleanup pass through the database to remove
        // silo'd messages that have been stored beyond the MessageSiloExpirationTime.  If mExpirationTime is configured
        // as 0, then records never expire, so no need to peform the cleanup.
        // Note: addToSilo->mOriginalSendTime is always now - so no need to requery current time
        // Run cleanup before adding new records to save iterating through 1 extra item
        if (mExpirationTime > 0 && (addToSilo->mOriginalSendTime - mLastSiloCleanupTime) > SILO_CLEANUP_PERIOD)
        {
            mLastSiloCleanupTime = addToSilo->mOriginalSendTime;  // reset stored silo cleanup time

            mSiloStore.cleanupExpiredSiloRecords(addToSilo->mOriginalSendTime, mExpirationTime);
        }

        // TODO - look for addMessage failures and queue up to be attempted to be written later (ie. when db is back and live)
        mSiloStore.addMessage(addToSilo->mDestUri, addToSilo->mSourceUri, addToSilo->mOriginalSendTime, addToSilo->getTransactionId(), addToSilo->mMimeType, addToSilo->mMessageBody);
        return false;
    }

    AsyncDrainSiloMessage* drainSilo = dynamic_cast<AsyncDrainSiloMessage*>(msg);
    if (drainSilo)
    {
        AbstractDb::SiloRecordList recordList;
        if (mSiloStore.getSiloRecords(drainSilo->mAor, recordList))
        {
            time_t now = time(0);

            // Note:  Tesing with BerkeleyDb and MySQL reveals that these databases return the records in insert order
            //        so there is no need to sort the records here.

            AbstractDb::SiloRecordList::iterator siloIt = recordList.begin();
            for (; siloIt != recordList.end(); siloIt++)
            {
                DebugLog(<< "DrainSilo:  Dest=" << siloIt->mDestUri << ", Source=" << siloIt->mSourceUri << ", Datetime=" << Data::from(DateCategory(siloIt->mOriginalSentTime)) << ", MimeType=" << siloIt->mMimeType << ", Body=" << siloIt->mMessageBody);

                // Only send if not too old
                if ((unsigned long)(now - siloIt->mOriginalSentTime) <= mExpirationTime)
                {
                    ContactList::iterator contactIt = drainSilo->mRequestContacts.begin();
                    for (; contactIt != drainSilo->mRequestContacts.end(); contactIt++)
                    {
                        // send messages to each contact from register message - honour path
                        ContactInstanceRecord& rec = *contactIt;

                        // Removed contacts can be in the list, but they will be expired, don't send to them
                        if (rec.mRegExpires > (uint64_t)now)
                        {
                            std::unique_ptr<SipMessage> msg(new SipMessage);
                            RequestLine rLine(MESSAGE);
                            rLine.uri() = rec.mContact.uri();
                            msg->header(h_RequestLine) = rLine;
                            msg->header(h_To) = NameAddr(siloIt->mDestUri);
                            msg->header(h_MaxForwards).value() = 20;
                            msg->header(h_CSeq).method() = MESSAGE;
                            msg->header(h_CSeq).sequence() = 1;
                            msg->header(h_From) = NameAddr(siloIt->mSourceUri);
                            msg->header(h_From).param(p_tag) = Helper::computeTag(Helper::tagSize);
                            msg->header(h_CallId).value() = Helper::computeCallId();
                            Via via;
                            msg->header(h_Vias).push_back(via);

                            // add routes from registration path
                            if (!rec.mSipPath.empty())
                            {
                                msg->header(h_Routes).append(rec.mSipPath);
                            }

                            // Add Date Header if enabled
                            if (mAddDateHeader)
                            {
                                msg->header(h_Date) = DateCategory(siloIt->mOriginalSentTime);
                            }

                            if (rec.mUseFlowRouting &&
                                rec.mReceivedFrom.mFlowKey)
                            {
                                // .bwc. We only override the destination if we are sending to an
                                // outbound contact. If this is not an outbound contact, but the
                                // endpoint has given us a Contact with the correct ip-address and 
                                // port, we might be able to find the connection they formed when they
                                // registered earlier, but that will happen down in TransportSelector.
                                msg->setDestination(rec.mReceivedFrom);
                            }

                            // Helper::processStrictRoute(*msg.get());  // Path headers must have ;lr so this isn't required

                            // Add mime body
                            HeaderFieldValue hfv(siloIt->mMessageBody.data(), siloIt->mMessageBody.size());
                            Mime type;
                            ParseBuffer pb(siloIt->mMimeType);
                            type.parse(pb);
                            PlainContents contents(hfv, type);
                            msg->setContents(&contents);  // need to clone since body data isn't owned by message yet

                            mAsyncDispatcher->mStack->send(std::move(msg));
                        }
                    }
                }

                // Delete record from database
                // Note:  A potential feature enhancement would be to monitor the MESSAGE reponses and only remove
                //        from the database when a 200 reponses is seen.  Care must be taken to avoid
                //        looping and handle scenarios when a user never uses a device capable of IM.
                mSiloStore.deleteSiloRecord(siloIt->mOriginalSentTime, siloIt->mTid);
            }
        }
        return false;
    }

    return false; // Nothing to queue to stack
}
bool MyRegistrarHandler::onRefresh(resip::ServerRegistrationHandle sr, const resip::SipMessage& reg)
{
    resip::Uri mAor = GetSrcUri(reg);
    resip::Tuple fromtu = reg.getSource();
    //resip::Tuple localtu = CTools::GetInLocalIP(fromtu);
    //Data localip = resip::Tuple::inet_ntop(localtu);
    Data username;
    if (reg.exists(h_Authorizations))
    {
        auto authList = reg.header(h_Authorizations);
        for (auto& iter : authList)
        {
            if (iter.exists(p_username))
            {
                username = iter.param(p_username);
            }
        }
    }
    if (reg.exists(h_ProxyAuthorizations))
    {
        auto authList = reg.header(h_ProxyAuthorizations);
        for (auto& iter : authList)
        {
            if (iter.exists(p_username))
            {
                username = iter.param(p_username);
            }
        }
    }
    if (!username.empty())
    {
        IDeviceMngrSvr& devmng = GetIDeviceMngr();
        SqliteDb* pdb = dynamic_cast<SqliteDb*>(mAbstractDb);
        if (pdb)
        {
            std::shared_ptr<Device> pDev = devmng.queryDevice(username.c_str());
            if (pDev)
            {
                std::shared_ptr<SipServerDeviceInfo> pGbDev = std::static_pointer_cast<SipServerDeviceInfo>(pDev);
                if (pGbDev)
                {
                    Data ip = resip::Tuple::inet_ntop(reg.getSource());
                    pGbDev->setIp(ip.c_str());
                    int port = reg.getSource().getPort();
                    pGbDev->setPort(port);
                    std::string hostAddr = std::str_format("%s:%d", ip.c_str(), port);
                    pGbDev->setHostAddress(hostAddr);
                    CDateTime nowtm;
                    pGbDev->setUpdateTime(nowtm.tmFormat());

                    if (reg.exists(h_Expires))
                        pGbDev->setExpires(reg.header(h_Expires).value());
                    devmng.updateDevice(pGbDev.get());
                }
            }
        }
    }
    return true;
    //SipMessage Reg200ok;
    //bool AuthorResult = true;
    //try
    //{
    //    if (reg.exists(h_Authorizations))
    //    {
    //        for (Auths::const_iterator i = reg.header(h_Authorizations).begin(); i != reg.header(h_Authorizations).end(); ++i)
    //        {
    //            if (i->exists(p_sign1))
    //            {
    //                //char downdevpblcbuf[256] = { 0 };
    //                auto dev = GetDevInfoPtrByGBID(mAor.user().c_str());
    //                //snprintf(downdevpblcbuf, 256, "/data/app/%s", dev->strPubKey.c_str());
    //                FILE* cryptkeyfp = fopen(dev->strPubKey.c_str(), "r+b");
    //                unsigned int   filesize = 0;
    //                if (cryptkeyfp)
    //                {
    //                    fseek(cryptkeyfp, 0, SEEK_END);
    //                    filesize = ftell(cryptkeyfp);
    //                }
    //                else
    //                {
    //                    AuthorResult = false;
    //                    break;
    //                }
    //                char* pPbKeyBuf = new char[filesize + 1];
    //                memset(pPbKeyBuf, 0, filesize + 1);
    //                fseek(cryptkeyfp, 0, SEEK_SET);
    //                fread(pPbKeyBuf, 1, filesize, cryptkeyfp);
    //                pPbKeyBuf[filesize] = 0;
    //                fclose(cryptkeyfp);

    //                Data R1("1234567812345678");// = Random::getRandom(16);
    //                //Data R1 = Random::getRandom(16);
    //                std::string vkek(R1.c_str(), (size_t)R1.size());
    //                dev->NewVkek = vkek;
    //                Auth auth;
    //                std::string strEncData;
    //                CEncryptLib::Sm2Encrypt(pPbKeyBuf, vkek, strEncData);//用设备公钥对vkek加密得到cryptkey
    //                auth.param(p_cryptkey) = Data(strEncData.c_str(), strEncData.size()).base64encode();
    //                LogOut("GB", L_DEBUG, "******vkek:%s enc 64:%s", vkek.c_str(), auth.param(p_cryptkey).c_str());
    //                if (isEqualNoCase(i->scheme(), AuthScheme))
    //                {
    //                    auth.scheme() = AuthScheme;
    //                    Helper::makeResponse(Reg200ok, reg, 200);
    //                }
    //                else if (isEqualNoCase(i->scheme(), AuthScheme_b))
    //                {
    //                    auth.scheme() = AuthScheme_b;
    //                    AuthorResult = false;
    //                    if (i->exists(p_random1) && i->exists(p_random2) && i->exists(p_deviceid))
    //                    {
    //                        auth.param(p_random1) = i->param(p_random1);
    //                        auth.param(p_random2) = i->param(p_random2);
    //                        auth.param(p_deviceid) = i->param(p_deviceid);

    //                        Data sign2pb = auth.param(p_random1) + auth.param(p_random2) + auth.param(p_deviceid) + auth.param(p_cryptkey);

    //                        char* pSvrPriBuf = NULL;
    //                        FILE* fstream = fopen("/data/app/para/device_private_key.pem", "r+b");
    //                        unsigned int   filesize = 0;
    //                        if (fstream)
    //                        {
    //                            fseek(fstream, 0, SEEK_END);
    //                            filesize = ftell(fstream);
    //                        }
    //                        else
    //                        {
    //                            AuthorResult = false;
    //                            break;
    //                        }
    //                        pSvrPriBuf = new char[filesize + 1];//32 K
    //                        memset(pSvrPriBuf, 0, filesize + 1);
    //                        fseek(fstream, 0, SEEK_SET);
    //                        fread(pSvrPriBuf, 1, filesize, fstream);
    //                        pSvrPriBuf[filesize] = 0;
    //                        fclose(fstream);
    //                        std::string strsign2;
    //                        CEncryptLib::Sm2Sign(pSvrPriBuf, sign2pb.c_str(), strsign2);//用管理平台私钥对random1+random2+deviceid+cryptkey签名得到sign2

    //                        Data base64sign2(strsign2.c_str(), strsign2.size());
    //                        auth.param(p_sign2) = base64sign2.base64encode();

    //                        AuthorResult = true;
    //                    }
    //                    if (AuthorResult)
    //                    {
    //                        Helper::makeResponse(Reg200ok, reg, 200);
    //                    }
    //                    else
    //                    {
    //                        Helper::makeResponse(Reg200ok, reg, 403);
    //                    }
    //                }
    //                auth.param(p_algorithm) = "A:SM2;H:SM3;S:SM4/OFB/PKCS5;SI:SM3-SM2";// "MD5";

    //                delete[]pPbKeyBuf; pPbKeyBuf = NULL;

    //                Data SecurityInfo;
    //                {
    //                    oDataStream dataStream(SecurityInfo);
    //                    auth.encodeParsed(dataStream);
    //                }
    //                StringCategories& exHeaders = Reg200ok.header(h_exHdsecurity);
    //                exHeaders.push_back(StringCategory(SecurityInfo));

    //                datatime28181 data281(time(0));
    //                HeaderFieldValueList  hfv;
    //                Data CurDate(data281.c_str());
    //                hfv.push_back(CurDate.c_str(), CurDate.size(), false);
    //                Reg200ok.setRawHeader(&hfv, Headers::Date);
    //                //resip::SipDateCategory DateTime(Data("yyyy-MM-ddTHH:mm:ss.SSS"));
    //                //Data strDate;
    //                {
    //                    //oDataStream dateStream(strDate);
    //                    //DateTime.encodeParsed(dateStream);
    //                }
    //                //hfv.push_back(strDate.c_str(), strDate.size(), false);
    //                //Reg200ok.setRawHeader(&hfv, Headers::Date);
    //                sr->accept(Reg200ok);
    //                std::cout << "\n" << Reg200ok << std::endl;
    //                return false;
    //            }
    //        }
    //    }
    //    if (AuthorResult)
    //    {
    //        Helper::makeResponse(Reg200ok, reg, 200);
    //    }
    //    else
    //    {
    //        Helper::makeResponse(Reg200ok, reg, 403);
    //    }
    //    HeaderFieldValueList  hfv; //= msg.getRawHeader(headerType);
    //    //resip::SipDateCategory DateTime(Data("yyyy-MM-ddTHH:mm:ss.SSS"));
    //    Data strDate;
    //    {
    //        oDataStream dateStream(strDate);
    //        //DateTime.encodeParsed(dateStream);
    //    }
    //    //hfv.push_back(strDate.c_str(), strDate.size(), false);
    //    //Reg200ok.setRawHeader(&hfv, Headers::Date);
    //    sr->accept(Reg200ok);


    //    CSIPGJMrg::CallDevOnlineEvent(mAor.user().c_str(), mAor.host().c_str(), mAor.port(), _GB_ADD, true, (void*)localip.c_str());
    //    return false;
    //}
    //catch (const std::exception&)
    //{
    //    std::cout << "reg 200 ok error" << std::endl;
    //}
    //return false;
}

bool MyRegistrarHandler::onRemove(resip::ServerRegistrationHandle sr, const resip::SipMessage& reg)
{
    resip::Uri mAor = GetSrcUri(reg);
    //CSIPGJMrg::CallDevOnlineEvent(mAor.user().c_str(), mAor.host().c_str(), mAor.port(), _GB_REMOVE, true);
    return true;
}
bool MyRegistrarHandler::onRemoveAll(resip::ServerRegistrationHandle sr, const resip::SipMessage& reg)
{
    resip::Uri mAor = GetSrcUri(reg);
    //CSIPGJMrg::CallDevOnlineEvent(mAor.user().c_str(), mAor.host().c_str(), mAor.port(), _GB_REMOVEALL, true);
    return true;
}
bool MyRegistrarHandler::onAdd(resip::ServerRegistrationHandle sr, const resip::SipMessage& reg)
{
    std::shared_ptr<ContactList> oContactList = sr->getOriginalContacts();
    const ContactList& reqContactList = sr->getRequestContacts();
    resip::Uri mAor = GetSrcUri(reg);
    resip::Tuple fromtu = reg.getSource();
    resip::Tuple dstTu = ((resip::SipMessage)reg).getDestination();
    resip::Tuple receivedTu = reg.getReceivedTransportTuple();
    Tuple publicAddress = Helper::getClientPublicAddress(reg);
    //resip::Tuple localtu = CTools::GetInLocalIP(fromtu);
    //Data localip = resip::Tuple::inet_ntop(localtu);
    IDeviceMngrSvr &devmng = GetIDeviceMngr();
    SqliteDb* pdb = dynamic_cast<SqliteDb*>(mAbstractDb);
    if (pdb)
    {
        Data deviceid;
        if (reg.exists(h_Authorizations))
        {
            auto authList = reg.header(h_Authorizations);
            for (auto &iter : authList)
            {
                if (iter.exists(p_username))
                {
                    deviceid = iter.param(p_username);
                }
            }
        }
        if (reg.exists(h_ProxyAuthorizations))
        {
            auto authList = reg.header(h_ProxyAuthorizations);
            for (auto& iter : authList)
            {
                if (iter.exists(p_username))
                {
                    deviceid = iter.param(p_username);
                }
            }
        }
        std::shared_ptr<Device> pDev = devmng.queryDevice(deviceid.c_str());
        if (pDev)
        {
            std::shared_ptr<SipServerDeviceInfo> pGbDev = std::static_pointer_cast<SipServerDeviceInfo>(pDev);
            if (pGbDev)
            {
                Data ip = resip::Tuple::inet_ntop(reg.getSource());
                pGbDev->setIp(ip.c_str());
                int port = reg.getSource().getPort();
                pGbDev->setPort(port);
                std::string hostAddr = std::str_format("%s:%d", ip.c_str(), port);
                pGbDev->setHostAddress(hostAddr);
                CDateTime nowtm;
                pGbDev->setUpdateTime(nowtm.tmFormat());

                if (reg.exists(h_Expires))
                    pGbDev->setExpires(reg.header(h_Expires).value());
                devmng.updateDevice(pGbDev.get());
            }
        }
        else
        {
            SipServerDeviceInfo devinfo;
            devinfo.setDeviceId(deviceid.c_str());
            Data ip = resip::Tuple::inet_ntop(reg.getSource());
            devinfo.setIp(ip.c_str());
            int port = reg.getSource().getPort();
            devinfo.setPort(port);
            std::string hostAddr = std::str_format("%s:%d", ip.c_str(), port);
            devinfo.setHostAddress(hostAddr);
            if (reg.exists(h_Expires))
                devinfo.setExpires(reg.header(h_Expires).value());
            if (reg.exists(h_Vias))
            {
                devinfo.setTransport(reg.header(h_Vias).begin()->transport().c_str());
            }
            devinfo.setOnline(1);
            devinfo.setDevAccessProtocal(Device::DEV_ACCESS_GB28181);
            devmng.online(&devinfo);
        }
    }

    AsyncDrainSiloMessage* async = new AsyncDrainSiloMessage(*this, Data::Empty, 0);  // tid and tu not needed since no response expected
    async->mAor = reg.header(h_To).uri().getAOR(false /* addPort? */);
    async->mRequestContacts = sr->getRequestContacts();
    std::unique_ptr<ApplicationMessage> async_ptr(async);
    mAsyncDispatcher->post(async_ptr);

    SipMessage success;
    Helper::makeResponse(success, reg, 200);
    if (Helper::isClientBehindNAT(reg))
    {
        Tuple ClientPublicAddress = Helper::getClientPublicAddress(reg);
        if (success.exists(h_Contacts))
        {
            success.header(h_Contacts).front().uri().host() = resip::Tuple::inet_ntop(ClientPublicAddress);
            success.header(h_Contacts).front().uri().port() = ClientPublicAddress.getPort();
        }
        else
        {
            success.header(h_Contacts).push_back(reg.header(h_Contacts).front());
            success.header(h_Contacts).front().uri().host() = resip::Tuple::inet_ntop(ClientPublicAddress);
            success.header(h_Contacts).front().uri().port() = ClientPublicAddress.getPort();
        }
    }
    DateCategory now(resip::TmType::GB28181Date);
    success.header(h_Date) = now;
    sr->accept(success);
    return false;
    return true;
    /*SipMessage success;
    Helper::makeResponse(success, reg, 200);

    DateCategory now(resip::TmType::GB28181Date);
    success.header(h_Date) = now;
    sr->accept(success);*/
    
    /*Data id;
    if (success.exists(h_Contacts))
    {
        id = success.header(h_Contacts).begin()->uri()
    }
    for (auto& it : success.header(h_Contacts))
    {
         id = it.uri().user();
    }
    for (auto& iter : success.header(h_Vias))
    {

    }*/
    //SipMessage Reg200ok;
    //bool AuthorResult = true;
    //try
    //{
    //    if (reg.exists(h_Authorizations))
    //    {
    //        for (Auths::const_iterator i = reg.header(h_Authorizations).begin(); i != reg.header(h_Authorizations).end(); ++i)
    //        {
    //            if (i->exists(p_sign1))
    //            {
    //                auto dev = GetDevInfoPtrByGBID(mAor.user().c_str());
    //                FILE* cryptkeyfp = fopen(dev->strPubKey.c_str(), "r+b");
    //                unsigned int   filesize = 0;
    //                if (cryptkeyfp)
    //                {
    //                    fseek(cryptkeyfp, 0, SEEK_END);
    //                    filesize = ftell(cryptkeyfp);
    //                }
    //                else
    //                {
    //                    AuthorResult = false;
    //                    break;
    //                }
    //                char* pPbKeyBuf = new char[filesize + 1];
    //                memset(pPbKeyBuf, 0, filesize + 1);
    //                fseek(cryptkeyfp, 0, SEEK_SET);
    //                fread(pPbKeyBuf, 1, filesize, cryptkeyfp);
    //                pPbKeyBuf[filesize] = 0;
    //                fclose(cryptkeyfp);

    //                Data R1("1234567812345678");// = Random::getRandom(16);
    //                //Data R1 = Random::getRandom(16);
    //                std::string vkek(R1.c_str(), (size_t)R1.size());
    //                dev->NewVkek = vkek;
    //                Auth auth;
    //                std::string strEncData;
    //                CEncryptLib::Sm2Encrypt(pPbKeyBuf, vkek, strEncData);//用设备公钥对vkek加密得到cryptkey
    //                delete[]pPbKeyBuf; pPbKeyBuf = NULL;
    //                auth.param(p_cryptkey) = Data(strEncData.c_str(), strEncData.size()).base64encode();
    //                LogOut("GB", L_DEBUG, "******vkek:%s enc 64:%s", vkek.c_str(), auth.param(p_cryptkey).c_str());
    //                if (isEqualNoCase(i->scheme(), AuthScheme))
    //                {
    //                    auth.scheme() = AuthScheme;
    //                    Helper::makeResponse(Reg200ok, reg, 200);
    //                }
    //                else if (isEqualNoCase(i->scheme(), AuthScheme_b))
    //                {
    //                    auth.scheme() = AuthScheme_b;
    //                    AuthorResult = false;
    //                    if (i->exists(p_random1) && i->exists(p_random2) && i->exists(p_deviceid))
    //                    {
    //                        auth.param(p_random1) = i->param(p_random1);
    //                        auth.param(p_random2) = i->param(p_random2);
    //                        auth.param(p_deviceid) = i->param(p_deviceid);

    //                        Data sign2pb = auth.param(p_random1) + auth.param(p_random2) + auth.param(p_deviceid) + auth.param(p_cryptkey);

    //                        char* pSvrPriBuf = NULL;
    //                        FILE* fstream = fopen("/data/app/para/device_private_key.pem", "r+b");
    //                        unsigned int   filesize = 0;
    //                        if (fstream)
    //                        {
    //                            fseek(fstream, 0, SEEK_END);
    //                            filesize = ftell(fstream);
    //                        }
    //                        else
    //                        {
    //                            AuthorResult = false;
    //                            break;
    //                        }
    //                        pSvrPriBuf = new char[filesize + 1];//32 K
    //                        memset(pSvrPriBuf, 0, filesize + 1);
    //                        fseek(fstream, 0, SEEK_SET);
    //                        fread(pSvrPriBuf, 1, filesize, fstream);
    //                        pSvrPriBuf[filesize] = 0;
    //                        fclose(fstream);
    //                        std::string strsign2;
    //                        CEncryptLib::Sm2Sign(pSvrPriBuf, sign2pb.c_str(), strsign2);//用管理平台私钥对random1+random2+deviceid+cryptkey签名得到sign2
    //                        delete[]pSvrPriBuf; pSvrPriBuf = NULL;
    //                        Data base64sign2(strsign2.c_str(), strsign2.size());
    //                        auth.param(p_sign2) = base64sign2.base64encode();

    //                        AuthorResult = true;
    //                    }
    //                    if (AuthorResult)
    //                    {
    //                        Helper::makeResponse(Reg200ok, reg, 200);
    //                    }
    //                    else
    //                    {
    //                        Helper::makeResponse(Reg200ok, reg, 403);
    //                    }
    //                }
    //                auth.param(p_algorithm) = "A:SM2;H:SM3;S:SM4/OFB/PKCS5;SI:SM3-SM2";// "MD5";

    //                Data SecurityInfo;
    //                {
    //                    oDataStream dataStream(SecurityInfo);
    //                    auth.encodeParsed(dataStream);
    //                }
    //                StringCategories& exHeaders = Reg200ok.header(h_exHdsecurity);
    //                exHeaders.push_back(StringCategory(SecurityInfo));


    //                datatime28181 data281(time(0));
    //                HeaderFieldValueList  hfv;
    //                Data CurDate(data281.c_str());
    //                hfv.push_back(CurDate.c_str(), CurDate.size(), false);
    //                Reg200ok.setRawHeader(&hfv, Headers::Date);
    //                //resip::SipDateCategory DateTime(Data("yyyy-MM-ddTHH:mm:ss.SSS"));
    //                Data strDate;
    //                {
    //                    oDataStream dateStream(strDate);
    //                    //DateTime.encodeParsed(dateStream);
    //                }
    //                //hfv.push_back(strDate.c_str(), strDate.size(), false);
    //                //Reg200ok.setRawHeader(&hfv, Headers::Date);
    //                sr->accept(Reg200ok);
    //                CSIPGJMrg::CallDevOnlineEvent(mAor.user().c_str(), mAor.host().c_str(), mAor.port(), _GB_ADD, true, (void*)localip.c_str());
    //                std::cout << "\n" << Reg200ok << std::endl;
    //                return false;
    //            }
    //        }
    //    }
    //    if (AuthorResult)
    //    {
    //        Helper::makeResponse(Reg200ok, reg, 200);
    //    }
    //    else
    //    {
    //        Helper::makeResponse(Reg200ok, reg, 403);
    //    }
    //    datatime28181 data281(time(0));
    //    HeaderFieldValueList  hfv;
    //    Data CurDate(data281.c_str());
    //    hfv.push_back(CurDate.c_str(), CurDate.size(), false);
    //    Reg200ok.setRawHeader(&hfv, Headers::Date);
    //    //resip::SipDateCategory DateTime(Data("yyyy-MM-ddTHH:mm:ss.SSS"));
    //    Data strDate;
    //    {
    //        oDataStream dateStream(strDate);
    //        //DateTime.encodeParsed(dateStream);
    //    }
    //    //hfv.push_back(strDate.c_str(), strDate.size(), false);
    //    //Reg200ok.setRawHeader(&hfv, Headers::Date);
    //    sr->accept(Reg200ok);


    //    return false;
    //}
    //catch (const std::exception&)
    //{
    //    std::cout << "reg 200 ok error" << std::endl;
    //}
    //return false;
}

resip::Uri MyRegistrarHandler::GetSrcUri(const resip::SipMessage& reg)
{
    resip::Uri Aor;
    resip::Tuple tu = reg.getSource();
    Aor.user() = reg.header(h_From).uri().user();
    Aor.host() = resip::Tuple::inet_ntop(tu);
    Aor.port() = tu.getPort();
    return Aor;
}
