
#include <iostream>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <utility>

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include "rutil/Logger.hxx"

#include "repro/AclStore.hxx"
#include "RegistServerAuthenticatorFactory.hxx"
#include "repro/ReproRADIUSServerAuthManager.hxx"
#include "RegistrarServerAuthManager.hxx"
#include "repro/ReproTlsPeerAuthManager.hxx"
#include "repro/UserAuthGrabber.hxx"
#include "resip/stack/Worker.hxx"
#include "repro/monkeys/CertificateAuthenticator.hxx"
#include "repro/monkeys/DigestAuthenticator.hxx"
#include "repro/monkeys/RADIUSAuthenticator.hxx"

using namespace std;
using namespace resip;
using namespace repro;

#define RESIPROCATE_SUBSYSTEM resip::Subsystem::REPRO

RegistAuthenticatorFactory::RegistAuthenticatorFactory(ProxyConfig& proxyConfig, SipStack& sipStack, DialogUsageManager* dum)
    : mProxyConfig(proxyConfig),
    mSipStack(sipStack),
    mDum(dum),
    mEnableCertAuth(mProxyConfig.getConfigBool("EnableCertificateAuthenticator", false)),
    mEnableDigestAuth(!mProxyConfig.getConfigBool("DisableAuth", false)),
    mEnableRADIUS(mProxyConfig.getConfigBool("EnableRADIUS", false)),
    mRADIUSConfiguration(mProxyConfig.getConfigData("RADIUSConfiguration", "")),
    mStaticRealm(mProxyConfig.getConfigData("StaticRealm", "")),
    mDigestChallengeThirdParties(!mEnableCertAuth),
    mCertificateAuthManager((DumFeature*)0),
    mServerAuthManager((ServerAuthManager*)0)
{
}

RegistAuthenticatorFactory::~RegistAuthenticatorFactory()
{
}

void
RegistAuthenticatorFactory::init()
{
    if (!mAuthRequestDispatcher.get())
    {
        int numAuthGrabberWorkerThreads = mProxyConfig.getConfigInt("NumAuthGrabberWorkerThreads", 2);
        if (numAuthGrabberWorkerThreads < 1)
        {
            numAuthGrabberWorkerThreads = 1; // must have at least one thread
        }
        std::unique_ptr<Worker> grabber(new UserAuthGrabber(*mProxyConfig.getDataStore()));
        mAuthRequestDispatcher.reset(new Dispatcher(std::move(grabber), &mSipStack, numAuthGrabberWorkerThreads));
    }

    // TODO: should be implemented using AbstractDb
    loadCommonNameMappings();
}

void
RegistAuthenticatorFactory::loadCommonNameMappings()
{
    // Already loaded?
    if (!mCommonNameMappings.empty())
        return;

    Data mappingsFileName = mProxyConfig.getConfigData("CommonNameMappings", "");
    if (mappingsFileName.empty())
        return;

    InfoLog(<< "trying to load common name mappings from file: " << mappingsFileName);

    ifstream mappingsFile(mappingsFileName.c_str());
    if (!mappingsFile)
    {
        ErrLog(<< "failed to open mappings file: " << mappingsFileName << ", aborting");
        throw std::runtime_error("Error opening/reading mappings file");
    }

    string sline;
    while (getline(mappingsFile, sline))
    {
        Data line(sline);
        Data cn;
        PermittedFromAddresses permitted;
        ParseBuffer pb(line);

        pb.skipWhitespace();
        const char* anchor = pb.position();
        if (pb.eof() || *anchor == '#') continue;  // if line is a comment or blank then skip it

        // Look for end of name
        pb.skipToOneOf("\t");
        pb.data(cn, anchor);
        if (mCommonNameMappings.find(cn) != mCommonNameMappings.end())
        {
            ErrLog(<< "CN '" << cn << "' repeated in mappings file");
            throw std::runtime_error("CN repeated in mappings file");
        }
        pb.skipChar('\t');

        while (!pb.eof())
        {
            pb.skipWhitespace();
            if (pb.eof())
                continue;

            Data value;
            anchor = pb.position();
            pb.skipToOneOf(",\r\n ");
            pb.data(value, anchor);
            if (!value.empty())
            {
                StackLog(<< "Loading CN '" << cn << "', found mapping '" << value << "'");
                permitted.insert(value);
            }
            if (!pb.eof())
                pb.skipChar();
        }

        DebugLog(<< "Loaded mapping for CN '" << cn << "', " << permitted.size() << " mapping(s)");
        mCommonNameMappings[cn] = permitted;
    }
}

std::shared_ptr<DumFeature>
RegistAuthenticatorFactory::getCertificateAuthManager()
{
    init();
    if (!mCertificateAuthManager.get())
    {
        Store* db = mProxyConfig.getDataStore();
        resip_assert(db);
        AclStore& aclStore = db->mAclStore;
        mCertificateAuthManager.reset(new ReproTlsPeerAuthManager(*mDum, mDum->dumIncomingTarget(), getDispatcher(), aclStore, true, mCommonNameMappings));
    }
    return mCertificateAuthManager;
}

std::unique_ptr<Processor>
RegistAuthenticatorFactory::getCertificateAuthenticator()
{
    init();
    Store* db = mProxyConfig.getDataStore();
    resip_assert(db);
    AclStore& aclStore = db->mAclStore;
    return std::unique_ptr<Processor>(new CertificateAuthenticator(mProxyConfig, getDispatcher(), &mSipStack, aclStore, true, mCommonNameMappings));
}

std::shared_ptr<ServerAuthManager>
RegistAuthenticatorFactory::getServerAuthManager()
{
    init();
    if (!mServerAuthManager.get())
    {
        if (mEnableRADIUS)
        {
#ifdef USE_RADIUS_CLIENT
            mServerAuthManager.reset(new ReproRADIUSServerAuthManager(*mDum,
                mProxyConfig.getDataStore()->mAclStore,
                !mProxyConfig.getConfigBool("DisableAuthInt", false) /*useAuthInt*/,
                mProxyConfig.getConfigBool("RejectBadNonces", false),
                mRADIUSConfiguration,
                mDigestChallengeThirdParties,
                mStaticRealm));
#else
            ErrLog(<< "can't create ReproRADIUSServerAuthManager, not compiled with RADIUS support");
#endif
        }
        else
        {
            mServerAuthManager.reset(new RegistServerAuthManager(*mDum,
                getDispatcher(),
                mProxyConfig.getDataStore()->mAclStore,
                !mProxyConfig.getConfigBool("DisableAuthInt", false) /*useAuthInt*/,
                mProxyConfig.getConfigBool("RejectBadNonces", false),
                mDigestChallengeThirdParties,
                mStaticRealm));
        }
    }
    return mServerAuthManager;
}

std::unique_ptr<Processor>
RegistAuthenticatorFactory::getDigestAuthenticator()
{
    init();
    if (mEnableRADIUS)
    {
#ifdef USE_RADIUS_CLIENT
        return std::unique_ptr<Processor>(new RADIUSAuthenticator(mProxyConfig, mRADIUSConfiguration, mStaticRealm));
#else
        ErrLog(<< "can't create RADIUSAuthenticator, not compiled with RADIUS support");
        return nullptr;
#endif
    }
    else
    {
        return std::unique_ptr<Processor>(new DigestAuthenticator(mProxyConfig, getDispatcher(), mStaticRealm));
    }
}

Dispatcher*
RegistAuthenticatorFactory::getDispatcher()
{
    init();
    return mAuthRequestDispatcher.get();
}