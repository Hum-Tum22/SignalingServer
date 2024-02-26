#if !defined(RESIP_REPROAUTHENTICATORFACTORY_HXX)
#define RESIP_REPROAUTHENTICATORFACTORY_HXX 

#include <memory>
#include <set>

#include "rutil/Data.hxx"
#include "resip/stack/SipStack.hxx"
#include "resip/stack/Dispatcher.hxx"
#include "resip/dum/DialogUsageManager.hxx"
#include "resip/dum/TlsPeerAuthManager.hxx"
#include "repro/AuthenticatorFactory.hxx"
#include "repro/ProxyConfig.hxx"

namespace repro
{

	class RegistAuthenticatorFactory : public AuthenticatorFactory
	{
	public:
		RegistAuthenticatorFactory(ProxyConfig& proxyConfig, resip::SipStack& sipStack, resip::DialogUsageManager* dum);
		virtual ~RegistAuthenticatorFactory();

		virtual void setDum(resip::DialogUsageManager* dum) { mDum = dum; };

		virtual bool certificateAuthEnabled() { return mEnableCertAuth; };

		virtual std::shared_ptr<resip::DumFeature> getCertificateAuthManager();
		virtual std::unique_ptr<Processor> getCertificateAuthenticator();

		virtual bool digestAuthEnabled() { return mEnableDigestAuth; };

		virtual std::shared_ptr<resip::ServerAuthManager> getServerAuthManager();
		virtual std::unique_ptr<Processor> getDigestAuthenticator();

		virtual resip::Dispatcher* getDispatcher();

	private:
		void init();
		void loadCommonNameMappings();

		ProxyConfig& mProxyConfig;
		resip::SipStack& mSipStack;
		resip::DialogUsageManager* mDum;

		bool mEnableCertAuth;
		bool mEnableDigestAuth;
		bool mEnableRADIUS;

		resip::Data mRADIUSConfiguration;

		resip::Data mStaticRealm;

		// Maintains existing behavior for non-TLS cert auth users
		bool mDigestChallengeThirdParties;

		resip::CommonNameMappings mCommonNameMappings;

		std::unique_ptr<resip::Dispatcher> mAuthRequestDispatcher;

		std::shared_ptr<resip::DumFeature> mCertificateAuthManager;
		std::shared_ptr<resip::ServerAuthManager> mServerAuthManager;
	};

}

#endif