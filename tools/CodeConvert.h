
#include <iostream>

//linux�£�
//zh_CN.GBK
//zh_CN.GB2312
//zh_CN.GB18030
//
//windows�£�
//��׼��ʽ��locale��
//Chinese_China.936
//zh-CN
//.936
namespace ownCodeCvt
{
#ifdef _WIN32
	std::string Utf8ToGbk(const std::string& in, const std::string& localename = ".936");
	std::string GbkToUtf8(const std::string& in, const std::string& localename = ".936");
#else
	std::string Utf8ToGbk(const std::string& in, const std::string& localename = "zh_CN.GB18030");
	std::string GbkToUtf8(const std::string& in, const std::string& localename = "zh_CN.GB18030");
#endif
}

