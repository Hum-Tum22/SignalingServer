#ifndef SIP_EXTENSION_INFO_H_
#define SIP_EXTENSION_INFO_H_
#include "resip/stack/ExtensionParameter.hxx"
#include "resip/stack/ExtensionHeader.hxx"

using namespace resip;

static ExtensionParameter p_keyversion("keyversion");

static ExtensionParameter p_random1("random1");
static ExtensionParameter p_random2("random2");
static ExtensionParameter p_serverid("serverid");
static ExtensionParameter p_sign1("sign1");
static ExtensionParameter p_sign2("sign2");
static ExtensionParameter p_cryptkey("cryptkey");
static ExtensionParameter p_deviceid("deviceid");


static ExtensionHeader h_exHdsecurity("SecurityInfo");

static ExtensionHeader h_exHdnote("Note");



//ONE_WAY_AUTHENTICATION 单向认证
//TWO_WAY_AUTHENTICATION 双向认证
#define ONE_WAY_AUTHENTICATION

//#ifdef ONE_WAY_AUTHENTICATION
static Data AuthScheme_b("Bidirection");
//#elif TWO_WAY_AUTHENTICATION
static Data AuthScheme("Unidirection");
//#endif
#endif