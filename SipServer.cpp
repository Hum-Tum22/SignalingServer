#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <iostream>
#include <fstream>
#include <stdexcept>
#ifndef WIN32
#include <syslog.h>
#endif

#ifdef REPRO_DSO_PLUGINS

// in an autotools build, this is defined using pkglibdir
#ifndef REPRO_DSO_PLUGIN_DIR_DEFAULT
#define REPRO_DSO_PLUGIN_DIR_DEFAULT ""
#endif

// This is the UNIX way of doing DSO, an alternative implementation
// for Windows needs to include the relevant Windows headers here
// and implement the loader code further below
#include <dlfcn.h>

#endif

#include "rutil/ResipAssert.h"
#include "rutil/Log.hxx"
#include "rutil/Logger.hxx"
#include "rutil/DnsUtil.hxx"
#include "rutil/dns/DnsStub.hxx"
#include "rutil/GeneralCongestionManager.hxx"
#include "rutil/TransportType.hxx"
#include "rutil/hep/HepAgent.hxx"
#include "rutil/Subsystem.hxx"
#include "rutil/Data.hxx"

#include "resip/stack/SipStack.hxx"
#include "resip/stack/Compression.hxx"
#include "resip/stack/EventStackThread.hxx"
#include "resip/stack/ExtendedDomainMatcher.hxx"
#include "resip/stack/HEPSipMessageLoggingHandler.hxx"
#include "resip/stack/InteropHelper.hxx"
#include "resip/stack/ConnectionManager.hxx"
#include "resip/stack/TransactionState.hxx"
#include "resip/stack/WsCookieContextFactory.hxx"

#include "resip/dum/InMemorySyncRegDb.hxx"
#include "resip/dum/InMemorySyncPubDb.hxx"
#include "resip/dum/MasterProfile.hxx"
#include "resip/dum/DialogUsageManager.hxx"
#include "resip/dum/DumThread.hxx"
#include "resip/dum/TlsPeerAuthManager.hxx"
#include "resip/dum/WsCookieAuthManager.hxx"

//#include "repro/AsyncProcessorWorker.hxx"
//#include "repro/ReproRunner.hxx"
//#include "repro/Proxy.hxx"
//#include "repro/ProxyConfig.hxx"

//#include "resip/stack/Dispatcher.hxx"
//#include "repro/UserAuthGrabber.hxx"
//#include "repro/ProcessorChain.hxx"
//#include "repro/ReproVersion.hxx"
//#include "repro/WebAdmin.hxx"
//#include "repro/WebAdminThread.hxx"
//#include "repro/Registrar.hxx"
//#include "repro/ReproAuthenticatorFactory.hxx"
//#include "repro/ReproServerAuthManager.hxx"
//#include "repro/RegSyncClient.hxx"
//#include "repro/RegSyncServer.hxx"
//#include "repro/RegSyncServerThread.hxx"
//#include "repro/CommandServer.hxx"
//#include "repro/CommandServerThread.hxx"
//#include "repro/BasicWsConnectionValidator.hxx"
//#include "repro/monkeys/CookieAuthenticator.hxx"
//#include "repro/monkeys/IsTrustedNode.hxx"
//#include "repro/monkeys/AmIResponsible.hxx"
//#include "repro/monkeys/DigestAuthenticator.hxx"
//#include "repro/monkeys/LocationServer.hxx"
//#include "repro/monkeys/RecursiveRedirect.hxx"
//#include "repro/monkeys/SimpleStaticRoute.hxx"
//#include "repro/monkeys/StaticRoute.hxx"
//#include "repro/monkeys/StrictRouteFixup.hxx"
//#include "repro/monkeys/OutboundTargetHandler.hxx"
//#include "repro/monkeys/QValueTargetHandler.hxx"
//#include "repro/monkeys/SimpleTargetHandler.hxx"
//#include "repro/monkeys/GeoProximityTargetSorter.hxx"
//#include "repro/monkeys/RequestFilter.hxx"
//#include "repro/monkeys/MessageSilo.hxx"
//#include "repro/monkeys/CertificateAuthenticator.hxx"
//#include "repro/stateAgents/PresenceServer.hxx"

#include "RegistrarServer/RegistServerAuthenticatorFactory.hxx"
#include "RegistrarServer/RegistrarServerAuthManager.hxx"
#include "RegistrarServer/BasicWsConnectionValidator.hxx"
#include "RegistrarServer/RegistServer.h"
#include "SipServer.h"
#include "UserAgent/UserAgentMgr.h"
#include "SipServerConfig.h"
#include "http.h"
#include "deviceMng/deviceMng.h"
#include "deviceMng/JsonDevice.h"

#include "myjsondef.h"
#include "writer.h"
#include "stringbuffer.h"
#include "document.h"

#if defined(USE_SSL)
#include "repro/stateAgents/CertServer.hxx"
#include "resip/stack/ssl/Security.hxx"
#define DEFAULT_TLS_METHOD SecurityTypes::SSLv23
#endif


#include "SqliteDb.h"
#if defined(USE_MYSQL)
#include "MySqlDb.hxx"
#endif

#if defined(USE_POSTGRESQL)
#include "repro/PostgreSqlDb.hxx"
#endif

#include "rutil/WinLeakCheck.hxx"

#define RESIPROCATE_SUBSYSTEM resip::Subsystem::TEST

using namespace resip;
using namespace regist;
using namespace sipserver;
using namespace std;


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ReproLogger : public ExternalLogger
{
public:
    virtual ~ReproLogger() {}
    /** return true to also do default logging, false to supress default logging. */
    virtual bool operator()(Log::Level level,
        const Subsystem& subsystem,
        const Data& appName,
        const char* file,
        int line,
        const Data& message,
        const Data& messageWithHeaders,
        const Data& instanceName)
    {
        // Log any errors to the screen 
        if (level <= Log::Err)
        {
            resipCout << messageWithHeaders << endl;
        }
        return true;
    }
};
ReproLogger g_ReproLogger;

class ReproSipMessageLoggingHandler : public Transport::SipMessageLoggingHandler
{
public:
    virtual ~ReproSipMessageLoggingHandler() {}
    virtual void outboundMessage(const Tuple& source, const Tuple& destination, const SipMessage& msg)
    {
        InfoLog(<< "\r\n*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*\r\n"
            << "OUTBOUND: Src=" << source << ", Dst=" << destination << "\r\n\r\n"
            << msg
            << "*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*");
        cout << "\r\n*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*\r\n"
            << "OUTBOUND: Src=" << source << ", Dst=" << destination << "\r\n\r\n"
            << msg
            << "*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*" << endl;
    }
    virtual void outboundRetransmit(const Tuple& source, const Tuple& destination, const SendData& data)
    {
        InfoLog(<< "\r\n*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*\r\n"
            << "OUTBOUND(retransmit): Src=" << source << ", Dst=" << destination << "\r\n\r\n"
            << data.data
            << "*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*");
        cout << "\r\n*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*\r\n"
            << "OUTBOUND(retransmit): Src=" << source << ", Dst=" << destination << "\r\n\r\n"
            << data.data
            << "*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*" << endl;
    }
    virtual void inboundMessage(const Tuple& source, const Tuple& destination, const SipMessage& msg)
    {
        InfoLog(<< "\r\n*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*\r\n"
            << "INBOUND: Src=" << source << ", Dst=" << destination << "\r\n\r\n"
            << msg
            << "*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*");
        cout << "\r\n*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*v*\r\n"
            << "INBOUND: Src=" << source << ", Dst=" << destination << "\r\n\r\n"
            << msg
            << "*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*^*" << endl;
    }
};


SipServer::SipServer()
    : mRunning(false)
    , mRestarting(false)
    , mThreadedStack(false)
    , mUseV4(true)
    , mUseV6(false)
    , mRegSyncPort(0)
    , mProxyConfig(0)
    , mFdPollGrp(0)
    , mAsyncProcessHandler(0)
    , mSipStack(0)
    , mStackThread(0)
    , mAbstractDb(0)
    , mRuntimeAbstractDb(0)
    , mRegistrationPersistenceManager(0)
    , mPublicationPersistenceManager(0)
    , mAsyncProcessorDispatcher(0)
    , mMonkeys(0)
    , mLemurs(0)
    , mBaboons(0)
    , mProxy(0)
    , mWebAdminThread(0)
    , mRegistrar(0)
    , mPresenceServer(0)
    , mDum(0)
    , mDumThread(0)
    , mCertServer(0)
    , mRegSyncClient(0)
    , mRegSyncServerV4(0)
    , mRegSyncServerV6(0)
    , mRegSyncServerAMQP(0)
    , mRegSyncServerThread(0)
    //, mCommandServerThread(0)
    , mCongestionManager(0)
    , mUserAgent(0)
    , mRegistSv(0)
    , zlmHttpPort(8080)
    , gbHttpPort(8090)
{
}

SipServer::~SipServer()
{
    if (mRunning) shutdown();
}

SipServer* SipServer::g_server = NULL;
SipServer* SipServer::Instance()
{
    if (!g_server)
    {
        g_server = new SipServer();
    }
    return g_server;
}
bool
SipServer::run(int argc, char** argv)
{
    if (mRunning) return false;

    installSignalHandler();

    if (!mRestarting)
    {
        // Store original arc and argv - so we can reuse them on restart request
        mArgc = argc;
        mArgv = argv;
    }
    // Parse command line and configuration file
    resip_assert(!mProxyConfig);
    Data defaultConfigFilename("repro.config");
    try
    {
        //char* p = NULL;
        mProxyConfig = &GetSipServerConfig();// new MyServerConfig();
        //mProxyConfig->parseConfigFile(defaultConfigFilename);
        //mProxyConfig->parseConfig(mArgc, mArgv, defaultConfigFilename);
        //mProxyConfig->parseConfig(1, &p, defaultConfigFilename);
    }
    catch (BaseException& ex)
    {
        std::cerr << "Error parsing configuration: " << ex << std::endl;
#ifndef WIN32
        syslog(LOG_DAEMON | LOG_CRIT, "%s", ex.getMessage().c_str());
#endif
        return false;
    }
    //其它初始化
    zlmHttpPort = mProxyConfig->getConfigInt("zlmhttpport", 8080);
    gbHttpPort = mProxyConfig->getConfigInt("HttpPort", 8090);
    zlmHost = mProxyConfig->getConfigData("zlmhost", "127.0.0.1", true).c_str();
    // Non-Windows server process stuff
    if (!mRestarting)
    {
        setPidFile(mProxyConfig->getConfigData("PidFile", Data::Empty, true));

        if (isAlreadyRunning())
        {
            std::cerr << "Already running, will not start two instances.  Please stop existing process and/or delete PID file.";
#ifndef WIN32
            syslog(LOG_DAEMON | LOG_CRIT, "Already running, will not start two instances.  Please stop existing process and/or delete PID file.");
#endif
            return false;
        }

        if (mProxyConfig->getConfigBool("Daemonize", false))
        {
            daemonize();
        }
    }

    // Initialize resip logger
    Data loggingType = mProxyConfig->getConfigData("LoggingType", "cout", true);
    /*Log::initialize(
        *mProxyConfig,
        mArgv[0],
        isEqualNoCase(loggingType, "file") ? &g_ReproLogger : 0);*/
    Log::initialize(Log::Cout, Log::Stack, "gb28181");

    //InfoLog(<< "Starting repro version " << VersionUtils::instance().releaseVersion() << "...");

    // Create SipStack and associated objects
    if (!createSipStack())
    {
        return false;
    }

    // Load the plugins after creating the stack, as they may need it
    if (!loadPlugins())
    {
        return false;
    }

    // Drop privileges (can do this now that sockets are bound)
    Data runAsUser = mProxyConfig->getConfigData("RunAsUser", Data::Empty, true);
    Data runAsGroup = mProxyConfig->getConfigData("RunAsGroup", Data::Empty, true);
    if (!runAsUser.empty())
    {
        InfoLog(<< "Trying to drop privileges, configured uid = " << runAsUser << " gid = " << runAsGroup);
        dropPrivileges(runAsUser, runAsGroup);
    }

    // Create datastore
    /*if (!createDatastore())
    {
        return false;
    }*/

    //// Create authentication mechanism
    //createAuthenticatorFactory();

    //// Create DialogUsageManager that handles ServerRegistration,
    //// and potentially certificate subscription server
    //createDialogUsageManager();
    if (!createRegistServer())
    {
        return false;
    }
    if (!mUserAgent)
    {
        int iRtpPortMin = mProxyConfig->getConfigInt("RtpPortRangeMin", 17000);
        int iRtpPortMax = mProxyConfig->getConfigInt("RtpPortRangeMax", 18000);
        mUserAgent = new UaMgr(*mSipStack, iRtpPortMin, iRtpPortMax);
    }

    // Create the Proxy and associate objects
    if (!createProxy())
    {
        return false;
    }

    // Create HTTP WebAdmin and Thread
    if (!createWebAdmin())
    {
        return false;
    }

    // Create reg sync components if required
    createRegSync();

    // Create command server if required
    if (!mRestarting)
    {
        createCommandServer();
    }

    // Make it all go - startup all threads
    mThreadedStack = mProxyConfig->getConfigBool("ThreadedStack", true);
    if (mThreadedStack)
    {
        // If configured, then start the sub-threads within the stack
        mSipStack->run();
    }
    mStackThread->run();
    if (mRegistSv)
        mRegistSv->Setup();
    if (mDumThread)
    {
        mDumThread->run();
    }
    mUserAgent->run();
    /*mProxy->run();
    if (mWebAdminThread)
    {
        mWebAdminThread->run();
    }
    if (!mRestarting && mCommandServerThread)
    {
        mCommandServerThread->run();
    }
    if (mRegSyncServerThread)
    {
        mRegSyncServerThread->run();
    }
    if (mRegSyncClient)
    {
        mRegSyncClient->run();
    }
    if (mRegSyncServerAMQP && mRegSyncServerAMQP->getThread().get())
    {
        mRegSyncServerAMQP->getThread()->run();
    }*/

    getQDCCTVNodeInfo();

    Uri target;// ("sip:34020000002000000002@192.168.2.140:5060");
    Data upId("34020000002000000002");
    target.user() = mProxyConfig->getConfigData("UPID", upId);
    Data uphost("192.168.1.223");
    target.host() = mProxyConfig->getConfigData("UPHOST", uphost);
    target.port() = mProxyConfig->getConfigInt("UPPORT", 8080);
    //Uri target("sip:34021000002000000001@192.168.1.138:5060");
    
    Uri fromUri("sip:34020000002000000001@192.168.1.230:8099");
    Data passwd("12345");
    passwd = mProxyConfig->getConfigData("UPPASSWORD", passwd);
    mUserAgent->DoRegist(target, fromUri, passwd);
    mRunning = true;

    return true;
}

void
SipServer::shutdown()
{
    if (!mRunning) return;

    // Tell all threads to shutdown
    if (mWebAdminThread)
    {
        //mWebAdminThread->shutdown();
    }
    if (mDumThread)
    {
        mDumThread->shutdown();
    }
    //mProxy->shutdown();
    mStackThread->shutdown();
    //if (!mRestarting && mCommandServerThread)  // leave command server running if we are restarting
    //{
    //    mCommandServerThread->shutdown();
    //}
    //if (mRegSyncServerThread)
    //{
    //    mRegSyncServerThread->shutdown();
    //}
    //if (mRegSyncClient)
    //{
    //    mRegSyncClient->shutdown();
    //}
    //if (mRegSyncServerAMQP && mRegSyncServerAMQP->getThread().get())
    //{
    //    mRegSyncServerAMQP->getThread()->shutdown();
    //}

    // Wait for all threads to shutdown, and destroy objects
    //mProxy->join();
    if (mThreadedStack)
    {
        mSipStack->shutdownAndJoinThreads();
    }
    mStackThread->join();
    /*if (mWebAdminThread)
    {
        mWebAdminThread->join();
    }*/
    if (mDumThread)
    {
        mDumThread->join();
    }
   
    if (mAsyncProcessorDispatcher)
    {
        // Both proxy and dum threads are down at this point, we can 
        // destroy the async processor dispatcher and associated threads now
        delete mAsyncProcessorDispatcher;
        mAsyncProcessorDispatcher = 0;
    }
    //if (!mRestarting && mCommandServerThread)  // we leave command server running during restart
    //{
    //    mCommandServerThread->join();
    //}
    //if (mRegSyncServerThread)
    //{
    //    mRegSyncServerThread->join();
    //}
    //if (mRegSyncClient)
    //{
    //    mRegSyncClient->join();
    //}
    //if (mRegSyncServerAMQP && mRegSyncServerAMQP->getThread().get())
    //{
    //    mRegSyncServerAMQP->getThread()->join();
    //}

    mSipStack->setCongestionManager(0);

    cleanupObjects();
    mRunning = false;
}

void
SipServer::restart()
{
    if (!mRunning) return;
    mRestarting = true;
    shutdown();
    run(0, 0);
    mRestarting = false;
}

void
SipServer::onReload()
{
    mSipStack->onReload();
    // Let the plugins know
    /*std::vector<Plugin*>::iterator it;
    for (it = mPlugins.begin(); it != mPlugins.end(); it++)
    {
        (*it)->onReload();
    }*/
}

void
SipServer::cleanupObjects()
{
    if (!mRestarting)
    {
        // We leave command server running during restart
        /*delete mCommandServerThread; mCommandServerThread = 0;
        for (std::list<CommandServer*>::iterator it = mCommandServerList.begin(); it != mCommandServerList.end(); it++)
        {
            delete (*it);
        }
        mCommandServerList.clear();*/
    }
    /*delete mRegSyncServerThread; mRegSyncServerThread = 0;
    delete mRegSyncServerAMQP; mRegSyncServerAMQP = 0;
    delete mRegSyncServerV6; mRegSyncServerV6 = 0;
    delete mRegSyncServerV4; mRegSyncServerV4 = 0;
    delete mRegSyncClient; mRegSyncClient = 0;*/
#if defined(USE_SSL)
    delete mCertServer; mCertServer = 0;
#endif
    delete mDumThread; mDumThread = 0;
    delete mDum; mDum = 0;
    delete mRegistrar; mRegistrar = 0;
    //delete mPresenceServer; mPresenceServer = 0;
    //delete mWebAdminThread; mWebAdminThread = 0;
    /*for (std::list<WebAdmin*>::iterator it = mWebAdminList.begin(); it != mWebAdminList.end(); it++)
    {
        delete (*it);
    }*/
    mWebAdminList.clear();
    //delete mProxy; mProxy = 0;
    //delete mBaboons; mBaboons = 0;
    //delete mLemurs; mLemurs = 0;
    //delete mMonkeys; mMonkeys = 0;
    delete mAsyncProcessorDispatcher; mAsyncProcessorDispatcher = 0;
    if (!mRestarting)
    {
        // If we are restarting then leave the In Memory Registration and Publication database intact
        delete mRegistrationPersistenceManager; mRegistrationPersistenceManager = 0;
        delete mPublicationPersistenceManager; mPublicationPersistenceManager = 0;
    }
    delete mAbstractDb; mAbstractDb = 0;
    delete mRuntimeAbstractDb; mRuntimeAbstractDb = 0;
    delete mStackThread; mStackThread = 0;
    delete mSipStack; mSipStack = 0;
    delete mCongestionManager; mCongestionManager = 0;
    delete mAsyncProcessHandler; mAsyncProcessHandler = 0;
    delete mFdPollGrp; mFdPollGrp = 0;
    delete mProxyConfig; mProxyConfig = 0;
}

bool
SipServer::loadPlugins()
{
    std::vector<Data> pluginNames;
    mProxyConfig->getConfigValue("LoadPlugins", pluginNames);

#ifdef REPRO_DSO_PLUGINS
    if (pluginNames.empty())
    {
        DebugLog(<< "LoadPlugins not specified, not attempting to load any plugins");
        return true;
    }

    const Data& pluginDirectory = mProxyConfig->getConfigData("PluginDirectory", REPRO_DSO_PLUGIN_DIR_DEFAULT, true);
    if (pluginDirectory.empty())
    {
        ErrLog(<< "LoadPlugins specified but PluginDirectory not specified, can't load plugins");
        return false;
    }
    for (std::vector<Data>::iterator it = pluginNames.begin(); it != pluginNames.end(); it++)
    {
        void* dlib;
        // FIXME:
        // - not all platforms use the .so extension
        // - detect and use correct directory separator charactor
        // - do we need to support relative paths here?
        // - should we use the filename prefix 'lib', 'mod' or something else?
        Data name = pluginDirectory + '/' + "lib" + *it + ".so";
        dlib = dlopen(name.c_str(), RTLD_NOW | RTLD_GLOBAL);
        if (!dlib)
        {
            ErrLog(<< "Failed to load plugin " << *it << " (" << name << "): " << dlerror());
            return false;
        }
        ReproPluginDescriptor* desc = (ReproPluginDescriptor*)dlsym(dlib, "reproPluginDesc");
        if (!desc)
        {
            ErrLog(<< "Failed to find reproPluginDesc in plugin " << *it << " (" << name << "): " << dlerror());
            return false;
        }
        if (!(desc->mPluginApiVersion == REPRO_DSO_PLUGIN_API_VERSION))
        {
            ErrLog(<< "Failed to load plugin " << *it << " (" << name << "): found version " << desc->mPluginApiVersion << ", expecting version " << REPRO_DSO_PLUGIN_API_VERSION);
        }
        DebugLog(<< "Trying to instantiate plugin " << *it);
        // Instantiate the plugin object and add it to our runtime environment
        Plugin* plugin = desc->creationFunc();
        if (!plugin)
        {
            ErrLog(<< "Failed to instantiate plugin " << *it << " (" << name << ")");
            return false;
        }
        if (!plugin->init(*mSipStack, mProxyConfig))
        {
            ErrLog(<< "Failed to initialize plugin " << *it << " (" << name << ")");
            return false;
        }
        mPlugins.push_back(plugin);
    }
    return true;
#else
    if (!pluginNames.empty())
    {
        ErrLog(<< "LoadPlugins specified but repro not compiled with plugin DSO support");
        return false;
    }
    DebugLog(<< "Not compiled with plugin DSO support");
    return true;
#endif
}

void
SipServer::setOpenSSLCTXOptionsFromConfig(const Data& configVar, long& opts)
{
#ifdef USE_SSL
    std::set<Data> values;
    if (mProxyConfig->getConfigValue(configVar, values))
    {
        opts = 0;
        for (std::set<Data>::iterator it = values.begin();
            it != values.end(); it++)
        {
            opts |= Security::parseOpenSSLCTXOption(*it);
        }
    }
#endif
}

bool
SipServer::createSipStack()
{
    // Override T1 timer if configured to do so
    unsigned long overrideT1 = mProxyConfig->getConfigUnsignedLong("TimerT1", 0);
    if (overrideT1)
    {
        WarningLog(<< "Overriding T1! (new value is " << overrideT1 << ")");
        resip::Timer::resetT1(overrideT1);
    }

    // Set TCP Connect timeout 
    resip::Timer::TcpConnectTimeout = mProxyConfig->getConfigUnsignedLong("TCPConnectTimeout", 10000);  // Default to 10 seconds

    // Set DNS Greylist Duration
    resip::TransactionState::DnsGreylistDurationMs = mProxyConfig->getConfigUnsignedLong("DNSGreylistDuration", 1800000);  // Default to 30mins

    unsigned long messageSizeLimit = mProxyConfig->getConfigUnsignedLong("StreamMessageSizeLimit", 0);
    if (messageSizeLimit > 0)
    {
        DebugLog(<< "Using maximum message size " << messageSizeLimit << " on stream-based transports");
        ConnectionBase::setMessageSizeMax(messageSizeLimit);
    }

    // Create Security (TLS / Certificates) and Compression (SigComp) objects if
    // pre-precessor defines are enabled
    Security* security = 0;
    Compression* compression = 0;
#ifdef USE_SSL
    setOpenSSLCTXOptionsFromConfig(
        "OpenSSLCTXSetOptions", BaseSecurity::OpenSSLCTXSetOptions);
    setOpenSSLCTXOptionsFromConfig(
        "OpenSSLCTXClearOptions", BaseSecurity::OpenSSLCTXClearOptions);
    Security::CipherList cipherList = Security::StrongestSuite;
    Data ciphers = mProxyConfig->getConfigData("OpenSSLCipherList", Data::Empty);
    if (!ciphers.empty())
    {
        cipherList = ciphers;
    }
    Data certPath = mProxyConfig->getConfigData("CertificatePath", Data::Empty);
    Data dHParamsFilename = mProxyConfig->getConfigData("TlsDHParamsFilename", Data::Empty);
    if (certPath.empty())
    {
        security = new Security(cipherList, mProxyConfig->getConfigData("TLSPrivateKeyPassPhrase", Data::Empty), dHParamsFilename);
    }
    else
    {
        security = new Security(certPath, cipherList, mProxyConfig->getConfigData("TLSPrivateKeyPassPhrase", Data::Empty), dHParamsFilename);
    }
    std::vector<Data> caDirNames;
    mProxyConfig->getConfigValue("CADirectory", caDirNames);
    for (std::vector<Data>::const_iterator caDir = caDirNames.begin();
        caDir != caDirNames.end(); caDir++)
    {
        security->addCADirectory(*caDir);
    }
    std::vector<Data> caFileNames;
    mProxyConfig->getConfigValue("CAFile", caFileNames);
    for (std::vector<Data>::const_iterator caFile = caFileNames.begin();
        caFile != caFileNames.end(); caFile++)
    {
        security->addCAFile(*caFile);
    }
    BaseSecurity::setAllowWildcardCertificates(mProxyConfig->getConfigBool("AllowWildcardCertificates", false));
#endif

#ifdef USE_SIGCOMP
    compression = new Compression(Compression::DEFLATE);
#endif

    // Create EventThreadInterruptor used to wake up the stack for 
    // for reasons other than an Fd signalling
    resip_assert(!mFdPollGrp);
    mFdPollGrp = FdPollGrp::create();
    resip_assert(!mAsyncProcessHandler);
    mAsyncProcessHandler = new EventThreadInterruptor(*mFdPollGrp);

    // Set Flags that will enable/disable IPv4 and/or IPv6, based on 
    // configuration and pre-processor flags
    mUseV4 = !mProxyConfig->getConfigBool("DisableIPv4", false);
#ifdef USE_IPV6
    mUseV6 = mProxyConfig->getConfigBool("EnableIPv6", true);
#else
    bool useV6 = false;
#endif
    if (mUseV4) InfoLog(<< "V4 enabled");
    if (mUseV6) InfoLog(<< "V6 enabled");

    // Build DNS Server list from config
    DnsStub::NameserverList dnsServers;
    std::vector<resip::Data> dnsServersConfig;
    mProxyConfig->getConfigValue("DNSServers", dnsServersConfig);
    for (std::vector<resip::Data>::iterator it = dnsServersConfig.begin(); it != dnsServersConfig.end(); it++)
    {
        if ((mUseV4 && DnsUtil::isIpV4Address(*it)) || (mUseV6 && DnsUtil::isIpV6Address(*it)))
        {
            InfoLog(<< "Using DNS Server from config: " << *it);
            dnsServers.push_back(Tuple(*it, 0, UNKNOWN_TRANSPORT).toGenericIPAddress());
        }
    }

    // Create the SipStack Object
    resip_assert(!mSipStack);
    mSipStack = new SipStack(security,
        dnsServers,
        mAsyncProcessHandler,
        /*stateless*/false,
        /*socketFunc*/0,
        compression,
        mFdPollGrp);

    // Set any enum suffixes from configuration
    std::vector<Data> enumSuffixes;
    mProxyConfig->getConfigValue("EnumSuffixes", enumSuffixes);
    if (enumSuffixes.size() > 0)
    {
        mSipStack->setEnumSuffixes(enumSuffixes);
    }

    // Set any enum domains from configuration
    std::map<Data, Data> enumDomains;
    std::vector<Data> _enumDomains;
    mProxyConfig->getConfigValue("EnumDomains", _enumDomains);
    if (enumSuffixes.size() > 0)
    {
        for (std::vector<Data>::iterator it = _enumDomains.begin(); it != _enumDomains.end(); it++)
        {
            enumDomains[*it] = *it;
        }
        mSipStack->setEnumDomains(enumDomains);
    }

    // Add External Stats handler
    mSipStack->setExternalStatsHandler(this);

    // Set Transport SipMessage Logging Handler - if enabled
    Data captureHost;
    mProxyConfig->getConfigValue("CaptureHost", captureHost);
    if (!captureHost.empty())
    {
        int capturePort = mProxyConfig->getConfigInt("CapturePort", 9060);
        int captureAgentID = mProxyConfig->getConfigInt("CaptureAgentID", 2001);
        auto agent = std::make_shared<HepAgent>(captureHost, capturePort, captureAgentID);
        mSipStack->setTransportSipMessageLoggingHandler(std::make_shared<HEPSipMessageLoggingHandler>(agent));
    }
    else if (mProxyConfig->getConfigBool("EnableSipMessageLogging", false))
    {
        mSipStack->setTransportSipMessageLoggingHandler(std::make_shared<ReproSipMessageLoggingHandler>());
    }

    // Add stack transports
    bool allTransportsSpecifyRecordRoute = false;
    if (!addTransports(allTransportsSpecifyRecordRoute))
    {
        cleanupObjects();
        return false;
    }

    // Enable and configure RFC5626 Outbound support
    InteropHelper::setOutboundVersion(mProxyConfig->getConfigInt("OutboundVersion", 5626));
    InteropHelper::setOutboundSupported(mProxyConfig->getConfigBool("DisableOutbound", false) ? false : true);
    InteropHelper::setRRTokenHackEnabled(mProxyConfig->getConfigBool("EnableFlowTokens", false));
    InteropHelper::setAllowInboundFlowTokensForNonDirectClients(mProxyConfig->getConfigBool("AllowInboundFlowTokensForNonDirectClients", false));
    InteropHelper::setAssumeFirstHopSupportsOutboundEnabled(mProxyConfig->getConfigBool("AssumeFirstHopSupportsOutbound", false));
    InteropHelper::setAssumeFirstHopSupportsFlowTokensEnabled(mProxyConfig->getConfigBool("AssumeFirstHopSupportsFlowTokens", false));
    Data clientNATDetectionMode = mProxyConfig->getConfigData("ClientNatDetectionMode", "DISABLED");
    if (isEqualNoCase(clientNATDetectionMode, "ENABLED"))
    {
        InteropHelper::setClientNATDetectionMode(InteropHelper::ClientNATDetectionEnabled);
    }
    else if (isEqualNoCase(clientNATDetectionMode, "PRIVATE_TO_PUBLIC"))
    {
        InteropHelper::setClientNATDetectionMode(InteropHelper::ClientNATDetectionPrivateToPublicOnly);
    }
    ConnectionManager::MinimumGcHeadroom = mProxyConfig->getConfigUnsignedLong("TCPMinimumGCHeadroom", 0);
    unsigned long tcpConnectionGCAge = mProxyConfig->getConfigUnsignedLong("TCPConnectionGCAge", 0);
    if (tcpConnectionGCAge > 0)
    {
        ConnectionManager::MinimumGcAge = tcpConnectionGCAge * 1000;
        ConnectionManager::EnableAgressiveGc = true;
    }
    unsigned long outboundFlowTimer = mProxyConfig->getConfigUnsignedLong("FlowTimer", 0);
    if (outboundFlowTimer > 0)
    {
        InteropHelper::setFlowTimerSeconds(outboundFlowTimer);
        if (tcpConnectionGCAge == 0)
        {
            // This should be set too when using outboundFlowTimer
            ConnectionManager::MinimumGcAge = 7200000;
        }
        ConnectionManager::EnableAgressiveGc = true;
    }

    // Decide whether or not to add rport to the Via header
    InteropHelper::setRportEnabled(mProxyConfig->getConfigBool("AddViaRport", true));

    // Check Path and RecordRoute settings, print warning if features are enabled that
    // require record-routing and record-route uri(s) is not configured
    bool forceRecordRoute = mProxyConfig->getConfigBool("ForceRecordRouting", false);
    Uri recordRouteUri;
    mProxyConfig->getConfigValue("RecordRouteUri", recordRouteUri);
    if ((InteropHelper::getOutboundSupported()
        || InteropHelper::getRRTokenHackEnabled()
        || InteropHelper::getClientNATDetectionMode() != InteropHelper::ClientNATDetectionDisabled
        || forceRecordRoute
        )
        && !(allTransportsSpecifyRecordRoute || !recordRouteUri.host().empty()))
    {
        CritLog(<< "In order for outbound support, the Record-Route flow-token"
            " hack, or force-record-route to work, you MUST specify a Record-Route URI. Launching "
            "without...");
        InteropHelper::setOutboundSupported(false);
        InteropHelper::setRRTokenHackEnabled(false);
        InteropHelper::setClientNATDetectionMode(InteropHelper::ClientNATDetectionDisabled);
        forceRecordRoute = false;
    }

    // Configure misc. stack settings
    mSipStack->setFixBadDialogIdentifiers(false);
    mSipStack->setFixBadCSeqNumbers(false);
    int statsLogInterval = mProxyConfig->getConfigInt("StatisticsLogInterval", 60);
    if (statsLogInterval > 0)
    {
        mSipStack->setStatisticsInterval(statsLogInterval);
        mSipStack->statisticsManagerEnabled() = true;
    }
    else
    {
        mSipStack->statisticsManagerEnabled() = false;
    }

    // Create Congestion Manager, if required
    resip_assert(!mCongestionManager);
    if (mProxyConfig->getConfigBool("CongestionManagement", true))
    {
        Data metricData = mProxyConfig->getConfigData("CongestionManagementMetric", "WAIT_TIME", true);
        GeneralCongestionManager::MetricType metric = GeneralCongestionManager::WAIT_TIME;
        if (isEqualNoCase(metricData, "TIME_DEPTH"))
        {
            metric = GeneralCongestionManager::TIME_DEPTH;
        }
        else if (isEqualNoCase(metricData, "SIZE"))
        {
            metric = GeneralCongestionManager::SIZE;
        }
        else if (!isEqualNoCase(metricData, "WAIT_TIME"))
        {
            WarningLog(<< "CongestionManagementMetric specified as an unknown value (" << metricData << "), defaulting to WAIT_TIME.");
        }
        mCongestionManager = new GeneralCongestionManager(
            metric,
            mProxyConfig->getConfigUnsignedLong("CongestionManagementTolerance", 200));
        mSipStack->setCongestionManager(mCongestionManager);
    }

    // Create base thread to run stack in (note:  stack may use other sub-threads, depending on configuration)
    resip_assert(!mStackThread);
    mStackThread = new EventStackThread(*mSipStack,
        *dynamic_cast<EventThreadInterruptor*>(mAsyncProcessHandler),
        *mFdPollGrp);
    return true;
}

bool
SipServer::createDatastore()
{
    // Create Database access objects
    resip_assert(!mAbstractDb);
    resip_assert(!mRuntimeAbstractDb);
    int defaultDatabaseIndex = mProxyConfig->getConfigInt("DefaultDatabase", -1);
    if (defaultDatabaseIndex >= 0)
    {
        mAbstractDb = mProxyConfig->getDatabase(defaultDatabaseIndex);
        if (!mAbstractDb)
        {
            CritLog(<< "Failed to get configuration database");
            cleanupObjects();
            return false;
        }
    }
    else     // Try legacy configuration parameter names
    {
#ifdef USE_MYSQL
        Data mySQLServer;
        mProxyConfig->getConfigValue("MySQLServer", mySQLServer);
        if (!mySQLServer.empty())
        {
            WarningLog(<< "Using deprecated parameter MySQLServer, please update to indexed Database definitions.");
            mAbstractDb = new MySqlDb(*mProxyConfig, mySQLServer,
                mProxyConfig->getConfigData("MySQLUser", Data::Empty),
                mProxyConfig->getConfigData("MySQLPassword", Data::Empty),
                mProxyConfig->getConfigData("MySQLDatabaseName", Data::Empty),
                mProxyConfig->getConfigUnsignedLong("MySQLPort", 0),
                mProxyConfig->getConfigData("MySQLCustomUserAuthQuery", Data::Empty));
        }
#endif
        if (!mAbstractDb)
        {
            //mAbstractDb = new BerkeleyDb(mProxyConfig->getConfigData("DatabasePath", "./", true));
            mAbstractDb = mProxyConfig->getDatabase(defaultDatabaseIndex);
        }
    }
    int runtimeDatabaseIndex = mProxyConfig->getConfigInt("RuntimeDatabase", -1);
    if (runtimeDatabaseIndex >= 0)
    {
        mRuntimeAbstractDb = mProxyConfig->getDatabase(runtimeDatabaseIndex);
        if (!mRuntimeAbstractDb || !mRuntimeAbstractDb->isSane())
        {
            CritLog(<< "Failed to get runtime database");
            cleanupObjects();
            return false;
        }
    }
#ifdef USE_MYSQL
    else     // Try legacy configuration parameter names
    {
        Data runtimeMySQLServer;
        mProxyConfig->getConfigValue("RuntimeMySQLServer", runtimeMySQLServer);
        if (!runtimeMySQLServer.empty())
        {
            WarningLog(<< "Using deprecated parameter RuntimeMySQLServer, please update to indexed Database definitions.");
            mRuntimeAbstractDb = new MySqlDb(*mProxyConfig, runtimeMySQLServer,
                mProxyConfig->getConfigData("RuntimeMySQLUser", Data::Empty),
                mProxyConfig->getConfigData("RuntimeMySQLPassword", Data::Empty),
                mProxyConfig->getConfigData("RuntimeMySQLDatabaseName", Data::Empty),
                mProxyConfig->getConfigUnsignedLong("RuntimeMySQLPort", 0),
                mProxyConfig->getConfigData("MySQLCustomUserAuthQuery", Data::Empty));
        }
    }
#endif
    resip_assert(mAbstractDb);
    if (!mAbstractDb->isSane())
    {
        CritLog(<< "Failed to open configuration database");
        cleanupObjects();
        return false;
    }
    if (mRuntimeAbstractDb && !mRuntimeAbstractDb->isSane())
    {
        CritLog(<< "Failed to open runtime configuration database");
        cleanupObjects();
        return false;
    }
    mProxyConfig->createDataStore(mAbstractDb, mRuntimeAbstractDb);

    // Create ImMemory Registration Database
    mRegSyncPort = mProxyConfig->getConfigInt("RegSyncPort", 0);
    // We only need removed records to linger if we have reg sync enabled
    if (!mRestarting)  // If we are restarting then we left the InMemorySyncRegDb and InMemorySyncPubDb intact at restart - don't recreate
    {
        resip_assert(!mRegistrationPersistenceManager);
        mRegistrationPersistenceManager = new InMemorySyncRegDb(mRegSyncPort ? 86400 /* 24 hours */ : 0 /* removeLingerSecs */);  // !slg! could make linger time a setting
        resip_assert(!mPublicationPersistenceManager);
        mPublicationPersistenceManager = new InMemorySyncPubDb((mRegSyncPort && mProxyConfig->getConfigBool("EnablePublicationReplication", false)) ? true : false);
    }
    resip_assert(mRegistrationPersistenceManager);
    resip_assert(mPublicationPersistenceManager);

    // Copy contacts from the StaticRegStore to the RegistrationPersistanceManager
    populateRegistrations();

    return true;
}

bool SipServer::createRegistServer()
{
    if (!mRegistSv)
    {
        mRegistSv = new CRegistServer(mProxyConfig, *mSipStack);
    }
    return mRegistSv->InitRegistServer();
}
bool SipServer::createRegistServerEx()
{
    //resip::InteropHelper::setRRTokenHackEnabled(args.mEnableFlowTokenHack);
    //resip::InteropHelper::setOutboundSupported(true);
    //resip::InteropHelper::setOutboundVersion(5626); // RFC 5626

    ////mProxy.addDomain("localhost");

    //// !bwc! TODO Once we have something we _do_ support, put that here.
    ////mProxy.addSupportedOption("p-fakeoption");
    //mSipStack->addAlias("localhost", 5060);
    //mSipStack->addAlias("localhost", 5061);

    //std::list<resip::Data> domains;
    //domains.push_back("127.0.0.1");
    //domains.push_back("localhost");


    //std::vector<Data> enumSuffixes;
    //enumSuffixes.push_back(args.mEnumSuffix);
    //mSipStack->setEnumSuffixes(enumSuffixes);


    //mProfile->clearSupportedMethods();
    //mProfile->addSupportedMethod(resip::REGISTER);
    //mProfile->addSupportedScheme(Symbols::Sips);

    //mDum->setMasterProfile(mProfile);
    //mDum->setServerRegistrationHandler(&mRegistrar);
    //mDum->setRegistrationPersistenceManager(&mRegData);
    //mDum->addDomain(host);

    //// Install rules so that the registrar only gets REGISTERs
    //resip::MessageFilterRule::MethodList methodList;
    //methodList.push_back(resip::REGISTER);

    //resip::MessageFilterRuleList ruleList;
    //ruleList.push_back(MessageFilterRule(resip::MessageFilterRule::SchemeList(),
    //    resip::MessageFilterRule::Any,
    //    methodList));
    //mDum->setMessageFilterRuleList(ruleList);

    //auto authMgr = std::make_shared<ReproServerAuthManager>(*mDum,
    //    mAuthRequestDispatcher,
    //    mConfig.getDataStore()->mAclStore,
    //    true,
    //    false,
    //    true);
    //mDum->setServerAuthManager(authMgr);

    //mStack->registerTransactionUser(mProxy);

    //if (args.mUseCongestionManager)
    //{
    //    mCongestionManager = std::unique_ptr<GeneralCongestionManager>(new GeneralCongestionManager(
    //        GeneralCongestionManager::WAIT_TIME,
    //        200));
    //    mStack->setCongestionManager(mCongestionManager.get());
    //}

    //if (args.mThreadedStack)
    //{
    //    mStack->run();
    //}

    //mStackThread->run();
    //mProxy.run();
    //mDumThread->run();
    return false;
}
bool
SipServer::createProxy()
{
    // Create AsyncProcessorDispatcher thread pool that is shared by the processsors for
    // any asynchronous tasks (ie: RequestFilter and MessageSilo processors)
//    int numAsyncProcessorWorkerThreads = mProxyConfig->getConfigInt("NumAsyncProcessorWorkerThreads", 2);
//    if (numAsyncProcessorWorkerThreads > 0)
//    {
//        resip_assert(!mAsyncProcessorDispatcher);
//        mAsyncProcessorDispatcher = new Dispatcher(std::unique_ptr<Worker>(new AsyncProcessorWorker),
//            mSipStack,
//            numAsyncProcessorWorkerThreads);
//    }
//
//    std::vector<Plugin*>::iterator it;
//
//    // Create proxy processor chains
//    /* Explanation:  "Monkeys" are processors which operate on incoming requests
//                     "Lemurs"  are processors which operate on incoming responses
//                     "Baboons" are processors which operate on a request for each target
//                               as the request is about to be forwarded to that target */
//                               // Make Monkeys
//    resip_assert(!mMonkeys);
//    mMonkeys = new ProcessorChain(Processor::REQUEST_CHAIN);
//    makeRequestProcessorChain(*mMonkeys);
//    InfoLog(<< *mMonkeys);
//    for (it = mPlugins.begin(); it != mPlugins.end(); it++)
//    {
//        (*it)->onRequestProcessorChainPopulated(*mMonkeys);
//    }
//
//    // Make Lemurs
//    resip_assert(!mLemurs);
//    mLemurs = new ProcessorChain(Processor::RESPONSE_CHAIN);
//    makeResponseProcessorChain(*mLemurs);
//    InfoLog(<< *mLemurs);
//    for (it = mPlugins.begin(); it != mPlugins.end(); it++)
//    {
//        (*it)->onResponseProcessorChainPopulated(*mLemurs);
//    }
//
//    // Make Baboons
//    resip_assert(!mBaboons);
//    mBaboons = new ProcessorChain(Processor::TARGET_CHAIN);
//    makeTargetProcessorChain(*mBaboons);
//    InfoLog(<< *mBaboons);
//    for (it = mPlugins.begin(); it != mPlugins.end(); it++)
//    {
//        (*it)->onTargetProcessorChainPopulated(*mBaboons);
//    }
//
//    // Create main Proxy class
//    resip_assert(!mProxy);
//    mProxy = new Proxy(*mSipStack,
//        *mProxyConfig,
//        *mMonkeys,
//        *mLemurs,
//        *mBaboons);
//    addDomains(*mProxy);
//    mHttpRealm = mProxyConfig->getConfigData("HttpAdminRealm", mDefaultRealm);
//
//    // Set Server Text
//#ifdef PACKAGE_VERSION
//    Data serverText(mProxyConfig->getConfigData("ServerText", "repro " PACKAGE_VERSION));
//#else
//    Data serverText(mProxyConfig->getConfigData("ServerText", Data::Empty));
//#endif
//    if (!serverText.empty())
//    {
//        mProxy->setServerText(serverText);
//    }
//
//    // Register the Proxy class a stack transaction user
//    // Note:  This is done after creating the DialogUsageManager so that it acts 
//    // like a catchall and will handle all requests the DUM does not
//    mSipStack->registerTransactionUser(*mProxy);
//
//    // Map the Registrar to the Proxy
//    if (mRegistrar)
//    {
//        mRegistrar->setProxy(mProxy);
//    }
//
//    // Add the transport specific RecordRoutes that were stored in addTransports to the Proxy
//    for (TransportRecordRouteMap::iterator it = mStartupTransportRecordRoutes.begin();
//        it != mStartupTransportRecordRoutes.end(); it++)
//    {
//        mProxy->addTransportRecordRoute(it->first, it->second);
//    }

    return true;
}

void
SipServer::populateRegistrations()
{
    resip_assert(mRegistrationPersistenceManager);
    resip_assert(mProxyConfig);
    resip_assert(mProxyConfig->getDataStore());

    // Copy contacts from the StaticRegStore to the RegistrationPersistanceManager
    StaticRegStore::StaticRegRecordMap& staticRegList = mProxyConfig->getDataStore()->mStaticRegStore.getStaticRegList();
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

bool
SipServer::createWebAdmin()
{
    resip_assert(mWebAdminList.empty());
    resip_assert(!mWebAdminThread);
    return true;
    //std::vector<resip::Data> httpServerBindAddresses;
    //mProxyConfig->getConfigValue("HttpBindAddress", httpServerBindAddresses);
    //int httpPort = mProxyConfig->getConfigInt("HttpPort", 5080);

    //if (httpPort)
    //{
    //    if (httpServerBindAddresses.empty())
    //    {
    //        if (mUseV4)
    //        {
    //            httpServerBindAddresses.push_back("0.0.0.0");
    //        }
    //        if (mUseV6)
    //        {
    //            httpServerBindAddresses.push_back("::");
    //        }
    //    }

    //    for (std::vector<resip::Data>::iterator it = httpServerBindAddresses.begin(); it != httpServerBindAddresses.end(); it++)
    //    {
    //        if (mUseV4 && DnsUtil::isIpV4Address(*it))
    //        {
    //            WebAdmin* webAdminV4 = 0;

    //            try
    //            {
    //                webAdminV4 = new WebAdmin(*mProxy,
    //                    *mRegistrationPersistenceManager,
    //                    *mPublicationPersistenceManager,
    //                    mHttpRealm,
    //                    httpPort,
    //                    V4,
    //                    *it);
    //            }
    //            catch (WebAdmin::ConfigException& ex)
    //            {
    //                ErrLog(<< "Exception when starting WebAdmin: " << ex.getMessage());
    //                webAdminV4 = 0;
    //            }

    //            if (!webAdminV4 || !webAdminV4->isSane())
    //            {
    //                CritLog(<< "Failed to start WebAdminV4");
    //                delete webAdminV4;
    //                cleanupObjects();
    //                return false;
    //            }

    //            mWebAdminList.push_back(webAdminV4);
    //        }

    //        if (mUseV6 && DnsUtil::isIpV6Address(*it))
    //        {
    //            WebAdmin* webAdminV6 = 0;

    //            try
    //            {
    //                webAdminV6 = new WebAdmin(*mProxy,
    //                    *mRegistrationPersistenceManager,
    //                    *mPublicationPersistenceManager,
    //                    mHttpRealm,
    //                    httpPort,
    //                    V6,
    //                    *it);
    //            }
    //            catch (WebAdmin::ConfigException& ex)
    //            {
    //                ErrLog(<< "Exception when starting WebAdmin: " << ex.getMessage());
    //                webAdminV6 = 0;
    //            }

    //            if (!webAdminV6 || !webAdminV6->isSane())
    //            {
    //                CritLog(<< "Failed to start WebAdminV6");
    //                delete webAdminV6;
    //                cleanupObjects();
    //                return false;
    //            }

    //            mWebAdminList.push_back(webAdminV6);
    //        }
    //    }

    //    // This shouldn't happen because it would return false before
    //    // it reached this point
    //    if (!mWebAdminList.empty())
    //    {
    //        mWebAdminThread = new WebAdminThread(mWebAdminList);
    //        return true;
    //    }
    //}

    CritLog(<< "Failed to start any WebAdmin");
    return false;
}

void
SipServer::createRegSync()
{
    /*resip_assert(!mRegSyncClient);
    resip_assert(!mRegSyncServerV4);
    resip_assert(!mRegSyncServerV6);
    resip_assert(!mRegSyncServerAMQP);
    resip_assert(!mRegSyncServerThread);
    bool enablePublicationReplication = mProxyConfig->getConfigBool("EnablePublicationReplication", false);
    if (mRegSyncPort != 0)
    {
        std::list<RegSyncServer*> regSyncServerList;
        if (mUseV4)
        {
            mRegSyncServerV4 = new RegSyncServer(dynamic_cast<InMemorySyncRegDb*>(mRegistrationPersistenceManager),
                mRegSyncPort, V4,
                enablePublicationReplication ? dynamic_cast<InMemorySyncPubDb*>(mPublicationPersistenceManager) : 0);
            regSyncServerList.push_back(mRegSyncServerV4);
        }
        if (mUseV6)
        {
            mRegSyncServerV6 = new RegSyncServer(dynamic_cast<InMemorySyncRegDb*>(mRegistrationPersistenceManager),
                mRegSyncPort, V6,
                enablePublicationReplication ? dynamic_cast<InMemorySyncPubDb*>(mPublicationPersistenceManager) : 0);
            regSyncServerList.push_back(mRegSyncServerV6);
        }
        if (!regSyncServerList.empty())
        {
            mRegSyncServerThread = new RegSyncServerThread(regSyncServerList);
        }
        Data regSyncPeerAddress(mProxyConfig->getConfigData("RegSyncPeer", ""));
        if (!regSyncPeerAddress.empty())
        {
            int remoteRegSyncPort = mProxyConfig->getConfigInt("RemoteRegSyncPort", 0);
            if (remoteRegSyncPort == 0)
            {
                remoteRegSyncPort = mRegSyncPort;
            }
            mRegSyncClient = new RegSyncClient(dynamic_cast<InMemorySyncRegDb*>(mRegistrationPersistenceManager),
                regSyncPeerAddress, remoteRegSyncPort,
                enablePublicationReplication ? dynamic_cast<InMemorySyncPubDb*>(mPublicationPersistenceManager) : 0);
        }
    }
    Data regSyncBrokerTopic = mProxyConfig->getConfigData("RegSyncBrokerTopic", Data::Empty);
    if (!regSyncBrokerTopic.empty())
    {
        mRegSyncServerAMQP = new RegSyncServer(dynamic_cast<InMemorySyncRegDb*>(mRegistrationPersistenceManager),
            regSyncBrokerTopic,
            enablePublicationReplication ? dynamic_cast<InMemorySyncPubDb*>(mPublicationPersistenceManager) : 0);
    }*/
}

void
SipServer::createCommandServer()
{
    /*resip_assert(mCommandServerList.empty());
    resip_assert(!mCommandServerThread);

    std::vector<resip::Data> commandServerBindAddresses;
    mProxyConfig->getConfigValue("CommandBindAddress", commandServerBindAddresses);
    int commandPort = mProxyConfig->getConfigInt("CommandPort", 5081);

    if (commandPort != 0)
    {
        if (commandServerBindAddresses.empty())
        {
            if (mUseV4)
            {
                commandServerBindAddresses.push_back("0.0.0.0");
            }
            if (mUseV6)
            {
                commandServerBindAddresses.push_back("::");
            }
        }

        for (std::vector<resip::Data>::iterator it = commandServerBindAddresses.begin(); it != commandServerBindAddresses.end(); it++)
        {
            if (mUseV4 && DnsUtil::isIpV4Address(*it))
            {
                CommandServer* pCommandServerV4 = new CommandServer(*this, *it, commandPort, V4);

                if (pCommandServerV4->isSane())
                {
                    mCommandServerList.push_back(pCommandServerV4);
                }
                else
                {
                    CritLog(<< "Failed to start CommandServerV4");
                    delete pCommandServerV4;
                }
            }

            if (mUseV6 && DnsUtil::isIpV6Address(*it))
            {
                CommandServer* pCommandServerV6 = new CommandServer(*this, *it, commandPort, V6);

                if (pCommandServerV6->isSane())
                {
                    mCommandServerList.push_back(pCommandServerV6);
                }
                else
                {
                    CritLog(<< "Failed to start CommandServerV6");
                    delete pCommandServerV6;
                }
            }
        }

        if (!mCommandServerList.empty())
        {
            mCommandServerThread = new CommandServerThread(mCommandServerList);
        }
    }*/
}

void
SipServer::initDomainMatcher()
{
    resip_assert(mProxyConfig);

    auto matcher = std::make_shared<ExtendedDomainMatcher>();
    mDomainMatcher = matcher;

    std::vector<Data> configDomains;
    if (mProxyConfig->getConfigValue("Domains", configDomains))
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
    if (mProxyConfig->getConfigValue("DomainSuffixes", configDomainSuffixes))
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

    const ConfigStore::ConfigData& dList = mProxyConfig->getDataStore()->mConfigStore.getConfigs();
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
       add any of the items below to the Domains config option in repro.config

    Data localhostname(DnsUtil::getLocalHostName());
    InfoLog (<< "Adding local hostname domain " << localhostname );
    matcher->addDomain(localhostname);
    if ( mDefaultRealm.empty() )
    {
       mDefaultRealm = localhostname;
    }

    InfoLog (<< "Adding localhost domain.");
    matcher->addDomain("localhost");
    if ( mDefaultRealm.empty() )
    {
       mDefaultRealm = "localhost";
    }

    list<pair<Data,Data> > ips = DnsUtil::getInterfaces();
    for ( list<pair<Data,Data> >::const_iterator i=ips.begin(); i!=ips.end(); i++)
    {
       InfoLog( << "Adding domain for IP " << i->second << " from interface " << i->first  );
       matcher->addDomain(i->second);
    }

    InfoLog (<< "Adding 127.0.0.1 domain.");
    matcher->addDomain("127.0.0.1"); */

    if (mDefaultRealm.empty())
    {
        mDefaultRealm = "Unconfigured";
    }
}

void
SipServer::addDomains(TransactionUser& tu)
{
    if (!mDomainMatcher)
    {
        initDomainMatcher();
    }
    tu.setDomainMatcher(mDomainMatcher);
}

bool
SipServer::addTransports(bool& allTransportsSpecifyRecordRoute)
{
    resip_assert(mProxyConfig);
    resip_assert(mSipStack);

    allTransportsSpecifyRecordRoute = false;
    mStartupTransportRecordRoutes.clear();

    bool useEmailAsSIP = mProxyConfig->getConfigBool("TLSUseEmailAsSIP", false);
    Data wsCookieAuthSharedSecret = mProxyConfig->getConfigData("WSCookieAuthSharedSecret", Data::Empty);
    std::shared_ptr<BasicWsConnectionValidator> basicWsConnectionValidator;
    std::shared_ptr<WsCookieContextFactory> wsCookieContextFactory;
    if (!wsCookieAuthSharedSecret.empty())
    {
        basicWsConnectionValidator = std::make_shared<BasicWsConnectionValidator>(wsCookieAuthSharedSecret);
        Data infoCookieName = mProxyConfig->getConfigData("WSCookieNameInfo", Data::Empty);
        Data extraCookieName = mProxyConfig->getConfigData("WSCookieNameExtra", Data::Empty);
        Data macCookieName = mProxyConfig->getConfigData("WSCookieNameMac", Data::Empty);

        wsCookieContextFactory = std::make_shared<BasicWsCookieContextFactory>(infoCookieName, extraCookieName, macCookieName);
    }

    try
    {
        // Check if advanced transport settings are provided
        ConfigParse::NestedConfigMap m = mProxyConfig->getConfigNested("Transport");
        DebugLog(<< "Found " << m.size() << " interface(s) defined in the advanced format");
        if (!m.empty())
        {
            // Sample config file format for advanced transport settings
            // Transport1Interface = 192.168.1.106:5061
            // Transport1Type = TLS
            // Transport1TlsDomain = sipdomain.com
            // Transport1TlsCertificate = /etc/ssl/crt/sipdomain.com.pem
            // Transport1TlsPrivateKey = /etc/ssl/private/sipdomain.com.pem
            // Transport1TlsPrivateKeyPassPhrase = <pwd>
            // Transport1TlsClientVerification = None
            // Transport1RecordRouteUri = sip:sipdomain.com;transport=TLS
            // Transport1RcvBufLen = 2000

            allTransportsSpecifyRecordRoute = true;

            const char* anchor;
            for (ConfigParse::NestedConfigMap::iterator it = m.begin();
                it != m.end();
                it++)
            {
                int idx = it->first;
                SipConfigParse tc(it->second);
                Data transportPrefix = "Transport" + Data(idx);
                DebugLog(<< "checking values for transport: " << idx);
                Data interfaceSettings = tc.getConfigData("Interface", Data::Empty, true);

                // Parse out interface settings
                ParseBuffer pb(interfaceSettings);
                anchor = pb.position();
                pb.skipToEnd();
                pb.skipBackToChar(':');  // For IPv6 the last : should be the port
                pb.skipBackChar();
                if (!pb.eof())
                {
                    Data ipAddr;
                    Data portData;
                    pb.data(ipAddr, anchor);
                    pb.skipChar();
                    anchor = pb.position();
                    pb.skipToEnd();
                    pb.data(portData, anchor);
                    if (!DnsUtil::isIpAddress(ipAddr))
                    {
                        CritLog(<< "Malformed IP-address found in " << transportPrefix << "Interface setting: " << ipAddr);
                    }
                    int port = portData.convertInt();
                    if (port == 0)
                    {
                        CritLog(<< "Invalid port found in " << transportPrefix << " setting: " << port);
                    }
                    TransportType tt = Tuple::toTransport(tc.getConfigData("Type", "UDP"));
                    if (tt == UNKNOWN_TRANSPORT)
                    {
                        CritLog(<< "Unknown transport type found in " << transportPrefix << "Type setting: " << tc.getConfigData("Type", "UDP"));
                    }
                    Data tlsDomain = tc.getConfigData("TlsDomain", Data::Empty);
                    Data tlsCertificate = tc.getConfigData("TlsCertificate", Data::Empty);
                    Data tlsPrivateKey = tc.getConfigData("TlsPrivateKey", Data::Empty);
                    Data tlsPrivateKeyPassPhrase = tc.getConfigData("TlsPrivateKeyPassPhrase", Data::Empty);
                    SecurityTypes::TlsClientVerificationMode cvm = tc.getConfigClientVerificationMode("TlsClientVerification", SecurityTypes::None);
                    SecurityTypes::SSLType sslType = SecurityTypes::NoSSL;
#ifdef USE_SSL
                    sslType = tc.getConfigSSLType("TlsConnectionMethod", DEFAULT_TLS_METHOD);
#endif

#ifdef USE_SSL
                    // Make sure certificate material available before trying to instantiate Transport
                    if (isSecure(tt))
                    {
                        Security* security = mSipStack->getSecurity();
                        resip_assert(security != 0);
                        // FIXME: see comments about CertificatePath
                        if (!tlsCertificate.empty())
                        {
                            security->addDomainCertPEM(tlsDomain, Data::fromFile(tlsCertificate));
                        }
                        if (!tlsPrivateKey.empty())
                        {
                            security->addDomainPrivateKeyPEM(tlsDomain, Data::fromFile(tlsPrivateKey), tlsPrivateKeyPassPhrase);
                        }
                    }
#endif

                    Transport* t = mSipStack->addTransport(tt,
                        port,
                        DnsUtil::isIpV6Address(ipAddr) ? V6 : V4,
                        StunEnabled,
                        ipAddr,       // interface to bind to
                        tlsDomain,
                        tlsPrivateKeyPassPhrase,  // private key passphrase
                        sslType, // sslType
                        0,            // transport flags
                        tlsCertificate, tlsPrivateKey,
                        cvm,          // tls client verification mode
                        useEmailAsSIP,
                        basicWsConnectionValidator, wsCookieContextFactory);

                    if (t)
                    {
                        int rcvBufLen = tc.getConfigInt("RcvBufLen", 0);
                        if (rcvBufLen > 0)
                        {
#if defined(RESIP_SIPSTACK_HAVE_FDPOLL)
                            // this new method is part of the epoll changeset,
                            // which isn't commited yet.
                            t->setRcvBufLen(rcvBufLen);
#else
                            resip_assert(0);
#endif
                        }

                        Data recordRouteUri = tc.getConfigData("RecordRouteUri", Data::Empty);
                        if (!recordRouteUri.empty())
                        {
                            try
                            {
                                if (isEqualNoCase(recordRouteUri, "auto")) // auto generated record route uri
                                {
                                    NameAddr rr;
                                    if (isSecure(tt))
                                    {
                                        rr.uri().host() = tlsDomain;
                                    }
                                    else
                                    {
                                        rr.uri().host() = ipAddr;
                                    }
                                    rr.uri().port() = port;
                                    rr.uri().param(resip::p_transport) = resip::Tuple::toDataLower(tt);
                                    mStartupTransportRecordRoutes[t->getKey()] = rr;  // Store to be added to Proxy after it is created
                                    InfoLog(<< "Transport specific record-route enabled (generated): " << rr);
                                }
                                else
                                {
                                    NameAddr rr(recordRouteUri);
                                    mStartupTransportRecordRoutes[t->getKey()] = rr;  // Store to be added to Proxy after it is created
                                    InfoLog(<< "Transport specific record-route enabled: " << rr);
                                }
                            }
                            catch (BaseException& e)
                            {
                                ErrLog(<< "Invalid uri provided in " << transportPrefix << "RecordRouteUri setting (ignoring): " << e);
                                allTransportsSpecifyRecordRoute = false;
                            }
                        }
                        else
                        {
                            allTransportsSpecifyRecordRoute = false;
                        }
                    }
                }
                else
                {
                    CritLog(<< "Port not specified in " << transportPrefix << " setting: expected format is <IPAddress>:<Port>");
                    return false;
                }
            }
        }
        else
        {
            Data ipAddress = mProxyConfig->getConfigData("IPAddress", Data::Empty, true);
            bool isV4Address = DnsUtil::isIpV4Address(ipAddress);
            bool isV6Address = DnsUtil::isIpV6Address(ipAddress);
            if (!isV4Address && !isV6Address)
            {
                if (!ipAddress.empty())
                {
                    ErrLog(<< "Malformed IP-address found in IPAddress setting, ignoring (binding to all interfaces): " << ipAddress);
                }
                ipAddress = Data::Empty;
                isV4Address = true;
                isV6Address = true;
            }
            int udpPort = mProxyConfig->getConfigInt("UDPPort", 5060);
            int tcpPort = mProxyConfig->getConfigInt("TCPPort", 5060);
            int tlsPort = mProxyConfig->getConfigInt("TLSPort", 5061);
            int wsPort = mProxyConfig->getConfigInt("WSPort", 80);
            int wssPort = mProxyConfig->getConfigInt("WSSPort", 443);
            int dtlsPort = mProxyConfig->getConfigInt("DTLSPort", 0);
            Data tlsDomain = mProxyConfig->getConfigData("TLSDomainName", Data::Empty);
            Data tlsCertificate = mProxyConfig->getConfigData("TLSCertificate", Data::Empty);
            Data tlsPrivateKey = mProxyConfig->getConfigData("TLSPrivateKey", Data::Empty);
            Data tlsPrivateKeyPassPhrase = mProxyConfig->getConfigData("TlsPrivateKeyPassPhrase", Data::Empty);
            SecurityTypes::TlsClientVerificationMode cvm = mProxyConfig->getConfigClientVerificationMode("TLSClientVerification", SecurityTypes::None);
            SecurityTypes::SSLType sslType = SecurityTypes::NoSSL;
#ifdef USE_SSL
            sslType = mProxyConfig->getConfigSSLType("TLSConnectionMethod", DEFAULT_TLS_METHOD);
#endif

#ifdef USE_SSL
            // Make sure certificate material available before trying to instantiate Transport
            if (tlsPort || wssPort || dtlsPort)
            {
                Security* security = mSipStack->getSecurity();
                resip_assert(security != 0);
                // FIXME: should check that EITHER CertificatePath was set or both of these
                // are supplied
                // In any case, it will still give a helpful error when it fails to
                // create the transport
                if (!tlsCertificate.empty())
                {
                    security->addDomainCertPEM(tlsDomain, Data::fromFile(tlsCertificate));
                }
                if (!tlsPrivateKey.empty())
                {
                    security->addDomainPrivateKeyPEM(tlsDomain, Data::fromFile(tlsPrivateKey));
                }
            }
#endif

            if (udpPort)
            {
                if (mUseV4 && isV4Address) mSipStack->addTransport(UDP, udpPort, V4, StunEnabled, ipAddress);
                if (mUseV6 && isV6Address) mSipStack->addTransport(UDP, udpPort, V6, StunEnabled, ipAddress);
            }
            if (tcpPort)
            {
                if (mUseV4 && isV4Address) mSipStack->addTransport(TCP, tcpPort, V4, StunEnabled, ipAddress);
                if (mUseV6 && isV6Address) mSipStack->addTransport(TCP, tcpPort, V6, StunEnabled, ipAddress);
            }
            if (tlsPort)
            {
                if (mUseV4 && isV4Address) mSipStack->addTransport(TLS, tlsPort, V4, StunEnabled, ipAddress, tlsDomain, tlsPrivateKeyPassPhrase, sslType, 0, tlsCertificate, tlsPrivateKey, cvm, useEmailAsSIP);
                if (mUseV6 && isV6Address) mSipStack->addTransport(TLS, tlsPort, V6, StunEnabled, ipAddress, tlsDomain, tlsPrivateKeyPassPhrase, sslType, 0, tlsCertificate, tlsPrivateKey, cvm, useEmailAsSIP);
            }
            if (wsPort)
            {
                if (mUseV4 && isV4Address) mSipStack->addTransport(WS, wsPort, V4, StunEnabled, ipAddress, Data::Empty, Data::Empty, SecurityTypes::NoSSL, 0, Data::Empty, Data::Empty, SecurityTypes::None, false, basicWsConnectionValidator, wsCookieContextFactory);
                if (mUseV6 && isV6Address) mSipStack->addTransport(WS, wsPort, V6, StunEnabled, ipAddress, Data::Empty, Data::Empty, SecurityTypes::NoSSL, 0, Data::Empty, Data::Empty, SecurityTypes::None, false, basicWsConnectionValidator, wsCookieContextFactory);
            }
            if (wssPort)
            {
                if (mUseV4 && isV4Address) mSipStack->addTransport(WSS, wssPort, V4, StunEnabled, ipAddress, tlsDomain, tlsPrivateKeyPassPhrase, sslType, 0, tlsCertificate, tlsPrivateKey, cvm, useEmailAsSIP, basicWsConnectionValidator, wsCookieContextFactory);
                if (mUseV6 && isV6Address) mSipStack->addTransport(WSS, wssPort, V6, StunEnabled, ipAddress, tlsDomain, tlsPrivateKeyPassPhrase, sslType, 0, tlsCertificate, tlsPrivateKey, cvm, useEmailAsSIP, basicWsConnectionValidator, wsCookieContextFactory);
            }
            if (dtlsPort)
            {
                if (mUseV4 && isV4Address) mSipStack->addTransport(DTLS, dtlsPort, V4, StunEnabled, ipAddress, tlsDomain, tlsPrivateKeyPassPhrase, sslType, 0, tlsCertificate, tlsPrivateKey);
                if (mUseV6 && isV6Address) mSipStack->addTransport(DTLS, dtlsPort, V6, StunEnabled, ipAddress, tlsDomain, tlsPrivateKeyPassPhrase, sslType, 0, tlsCertificate, tlsPrivateKey);
            }
        }
    }
    catch (BaseException& e)
    {
        std::cerr << "Likely a port is already in use" << endl;
        InfoLog(<< "Caught: " << e);
        return false;
    }
    return true;
}

//void
//SipServer::addProcessor(repro::ProcessorChain& chain, std::unique_ptr<Processor> processor)
//{
//    chain.addProcessor(std::move(processor));
//}
//
//void  // Monkeys
//SipServer::makeRequestProcessorChain(ProcessorChain& chain)
//{
//    resip_assert(mProxyConfig);
//    resip_assert(mRegistrationPersistenceManager);
//
//    // Add strict route fixup monkey
//    addProcessor(chain, std::unique_ptr<Processor>(new StrictRouteFixup));
//
//    // Add is trusted node monkey
//    addProcessor(chain, std::unique_ptr<Processor>(new IsTrustedNode(*mProxyConfig)));
//
//    // Add Certificate Authenticator - if required
//    resip_assert(mAuthFactory);
//    if (mAuthFactory->certificateAuthEnabled())
//    {
//        // TODO: perhaps this should be initialised from the trusted node
//        // monkey?  Or should the list of trusted TLS peers be independent
//        // from the trusted node list?
//        // Should we used the same trustedPeers object that was
//        // passed to TlsPeerAuthManager perhaps?
//        addProcessor(chain, mAuthFactory->getCertificateAuthenticator());
//    }
//
//    Data wsCookieAuthSharedSecret = mProxyConfig->getConfigData("WSCookieAuthSharedSecret", Data::Empty);
//    Data wsCookieExtraHeaderName = mProxyConfig->getConfigData("WSCookieExtraHeaderName", "X-WS-Session-Extra");
//    if (!mAuthFactory->digestAuthEnabled() && !wsCookieAuthSharedSecret.empty())
//    {
//        addProcessor(chain, std::unique_ptr<Processor>(new CookieAuthenticator(wsCookieAuthSharedSecret, wsCookieExtraHeaderName, mSipStack)));
//    }
//
//    // Add digest authenticator monkey - if required
//    if (mAuthFactory->digestAuthEnabled())
//    {
//        addProcessor(chain, mAuthFactory->getDigestAuthenticator());
//    }
//
//    // Add am I responsible monkey
//    addProcessor(chain, std::unique_ptr<Processor>(new AmIResponsible(mProxyConfig->getConfigBool("AlwaysAllowRelaying", false))));
//
//    // Add RequestFilter monkey
//    if (!mProxyConfig->getConfigBool("DisableRequestFilterProcessor", false))
//    {
//        if (mAsyncProcessorDispatcher)
//        {
//            addProcessor(chain, std::unique_ptr<Processor>(new RequestFilter(*mProxyConfig, mAsyncProcessorDispatcher)));
//        }
//        else
//        {
//            WarningLog(<< "Could not start RequestFilter Processor due to no worker thread pool (NumAsyncProcessorWorkerThreads=0)");
//        }
//    }
//
//    // [TODO] support for GRUU is on roadmap.  When it is added the GruuMonkey will go here
//
//    // [TODO] support for Manipulating Tel URIs is on the roadmap.
//    //        When added, the telUriMonkey will go here 
//
//    std::vector<Data> routeSet;
//    mProxyConfig->getConfigValue("Routes", routeSet);
//    if (routeSet.empty())
//    {
//        // add static route monkey
//        addProcessor(chain, std::unique_ptr<Processor>(new StaticRoute(*mProxyConfig)));
//    }
//    else
//    {
//        // add simple static route monkey
//        addProcessor(chain, std::unique_ptr<Processor>(new SimpleStaticRoute(*mProxyConfig)));
//    }
//
//    // Add location server monkey
//    addProcessor(chain, std::unique_ptr<Processor>(new LocationServer(*mProxyConfig, *mRegistrationPersistenceManager, mAuthFactory->getDispatcher())));
//
//    // Add message silo monkey
//    if (mProxyConfig->getConfigBool("MessageSiloEnabled", false))
//    {
//        if (mAsyncProcessorDispatcher && mRegistrar)
//        {
//            MessageSilo* silo = new MessageSilo(*mProxyConfig, mAsyncProcessorDispatcher);
//            mRegistrar->addRegistrarHandler(silo);
//            addProcessor(chain, std::unique_ptr<Processor>(silo));
//        }
//        else
//        {
//            WarningLog(<< "Could not start MessageSilo Processor due to no worker thread pool (NumAsyncProcessorWorkerThreads=0) or Registrar");
//        }
//    }
//}
//
//void  // Lemurs
//SipServer::makeResponseProcessorChain(ProcessorChain& chain)
//{
//    resip_assert(mProxyConfig);
//    resip_assert(mRegistrationPersistenceManager);
//
//    // Add outbound target handler lemur
//    addProcessor(chain, std::unique_ptr<Processor>(new OutboundTargetHandler(*mRegistrationPersistenceManager)));
//
//    if (mProxyConfig->getConfigBool("RecursiveRedirect", false))
//    {
//        // Add recursive redirect lemur
//        addProcessor(chain, std::unique_ptr<Processor>(new RecursiveRedirect));
//    }
//}
//
//void  // Baboons
//SipServer::makeTargetProcessorChain(ProcessorChain& chain)
//{
//    resip_assert(mProxyConfig);
//
//#ifndef RESIP_FIXED_POINT
//    if (mProxyConfig->getConfigBool("GeoProximityTargetSorting", false))
//    {
//        addProcessor(chain, std::unique_ptr<Processor>(new GeoProximityTargetSorter(*mProxyConfig)));
//    }
//#endif
//
//    if (mProxyConfig->getConfigBool("QValue", true))
//    {
//        // Add q value target handler baboon
//        addProcessor(chain, std::unique_ptr<Processor>(new QValueTargetHandler(*mProxyConfig)));
//    }
//
//    // Add simple target handler baboon
//    addProcessor(chain, std::unique_ptr<Processor>(new SimpleTargetHandler));
//}
//
bool SipServer::operator()(resip::StatisticsMessage& statsMessage)
{
    // Dispatch to each command server
    /*for (std::list<CommandServer*>::iterator it = mCommandServerList.begin(); it != mCommandServerList.end(); it++)
    {
        (*it)->handleStatisticsMessage(statsMessage);
    }*/
    return true;
}
SipServer* GetServer()
{
    return SipServer::Instance();
}

int SipServer::getQDCCTVNodeInfo()
{
    Data httpUrl("http:");

    Data httphost("192.168.1.223");
    httphost = mProxyConfig->getConfigData("CCTVHOST", httphost);
    int port = mProxyConfig->getConfigInt("CCTVPORT", 8080);
    httpUrl += httphost;
    httpUrl += Data(":");
    httpUrl += Data(port);
    httpUrl += Data("/device/gbInfo");

    std::string dirstr = GetRequest(httpUrl.c_str());

    rapidjson::Document document;
    document.Parse((char*)dirstr.c_str());
    if (!document.HasParseError())
    {
        if (document.HasMember("data") && document["data"].IsObject())
        {
            rapidjson::Value& body = document["data"];
            int count = json_check_int32(body, "Count");
            if (body.HasMember("Data") && body["Data"].IsArray())
            {
                rapidjson::Value& msbody = body["Data"];
                for (size_t i = 0; i < msbody.Size(); i++)
                {
                    VirtualOrganization voTop;
                    voTop.Name = json_check_string(msbody[i], "Title");
                    voTop.DeviceID = json_check_string(msbody[i], "GBId");
                    DeviceMng::Instance().addVirtualOrganization(voTop);

                    int port = json_check_int32(msbody[i], "ManagePort");
                    //port = 7000;
                    std::string user = "admin";// json_check_string(msbody[i], "ManageUser");
                    std::string pass = "12345";// json_check_string(msbody[i], "ManagePass");
                    std::string nvrIp = json_check_string(msbody[i], "ManageIp");
                    std::string nvrId = json_check_string(msbody[i], "nvrDid");
                    int nvrStatus = json_check_int32(msbody[i], "status");
                    auto dev = std::make_shared<JsonNvrDevic>(nvrId.c_str(), nvrIp.c_str(), port, user.c_str(), pass.c_str());
                    dev->setStatus(nvrStatus ? 0 : 1);
                    DeviceMng::Instance().addDevice(dev);
                    if (msbody[i].HasMember("Upward") && msbody[i]["Upward"].IsObject())
                    {
                        rapidjson::Value& upbody = msbody[i]["Upward"];
                        VirtualOrganization subVo;
                        subVo.Name = json_check_string(upbody, "Title");
                        subVo.DeviceID = json_check_string(upbody, "GBId");
                        subVo.ParentID = voTop.DeviceID;
                        DeviceMng::Instance().addVirtualOrganization(subVo);
                        
                        if (upbody.HasMember("Data") && upbody["Data"].IsArray())
                        {
                            rapidjson::Value& ipcbody = upbody["Data"];
                            for (size_t j = 0; j < ipcbody.Size(); j++)
                            {
                                std::string name = json_check_string(ipcbody[j], "ipc");
                                std::string childId = json_check_string(ipcbody[j], "GBId");
                                int ipcStatus = json_check_int32(ipcbody[j], "status");//1异常，0正常
                                ipcStatus = ipcStatus ? 0 : 1;
                                auto childDev = new JsonChildDevic(childId.c_str());
                                childDev->setParentDev(dev);
                                childDev->setName(name);
                                childDev->setStatus(ipcStatus);
                                childDev->setParentId(subVo.DeviceID);
                                DeviceMng::Instance().addChildDevice(childDev);
                            }
                        }
                    }
                    if (msbody[i].HasMember("Downward") && msbody[i]["Downward"].IsObject())
                    {
                        rapidjson::Value& downbody = msbody[i]["Downward"];
                        VirtualOrganization subVo;
                        subVo.Name = json_check_string(downbody, "Title");
                        subVo.DeviceID = json_check_string(downbody, "GBId");
                        subVo.ParentID = voTop.DeviceID;
                        DeviceMng::Instance().addVirtualOrganization(subVo);
                        if (downbody.HasMember("Data") && downbody["Data"].IsArray())
                        {
                            rapidjson::Value& ipcbody = downbody["Data"];
                            for (size_t j = 0; j < ipcbody.Size(); j++)
                            {
                                std::string childName = json_check_string(ipcbody[j], "ipc");
                                std::string childId = json_check_string(ipcbody[j], "GBId");
                                int ipcStatus = json_check_int32(ipcbody[j], "status");
                                ipcStatus = ipcStatus ? 0 : 1;
                                auto childDev = new JsonChildDevic(childId.c_str());
                                childDev->setParentDev(dev);
                                childDev->setName(childName);
                                childDev->setStatus(ipcStatus);
                                childDev->setParentId(subVo.DeviceID);
                                DeviceMng::Instance().addChildDevice(childDev);
                            }
                        }
                    }
                    if (msbody[i].HasMember("KKIpc") && msbody[i]["KKIpc"].IsObject())
                    {
                        rapidjson::Value& kkbody = msbody[i]["KKIpc"];
                        VirtualOrganization subVo;
                        subVo.Name = json_check_string(kkbody, "Title");
                        subVo.DeviceID = json_check_string(kkbody, "GBId");
                        subVo.ParentID = voTop.DeviceID;
                        DeviceMng::Instance().addVirtualOrganization(subVo);
                        if (kkbody.HasMember("Data") && kkbody["Data"].IsArray())
                        {
                            rapidjson::Value& ipcbody = kkbody["Data"];
                            for (size_t j = 0; j < ipcbody.Size(); j++)
                            {
                                std::string childName = json_check_string(ipcbody[j], "ipc");
                                std::string childId = json_check_string(ipcbody[j], "GBId");
                                int ipcStatus = json_check_int32(ipcbody[j], "status");
                                ipcStatus = ipcStatus ? 0 : 1;
                                auto childDev = new JsonChildDevic(childId.c_str());
                                childDev->setParentDev(dev);
                                childDev->setName(childName);
                                childDev->setStatus(ipcStatus);
                                childDev->setParentId(subVo.DeviceID);
                                DeviceMng::Instance().addChildDevice(childDev);
                            }
                        }
                    }

                }
            }
            return count;
        }
    }
    return 0;
}
