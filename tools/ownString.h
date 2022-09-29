#ifndef OWN_STRING_H_
#define OWN_STRING_H_
#include <iostream>
#include <vector>

namespace std
{
	//×Ö·û´®²ÎÊý±ØÐëÊÇc×Ö·û´®
	std::string str_format(const std::string format, ...);

	std::string to_string(const std::wstring& input);
	std::wstring to_wstring(const std::string& input);

	std::vector<std::string> vStringSplit(const  std::string& s, const std::string& delim = ",");
	std::vector<std::string> SipSubjectSplit(const  std::string& subject);

#ifdef _WIN32
	std::string Utf8ToGbk(const std::string& in, const std::string& localename = ".936");
	std::string GbkToUtf8(const std::string& in, const std::string& localename = ".936");
#else
	std::string Utf8ToGbk(const std::string& in, const std::string& localename = "zh_CN.GB18030");
	std::string GbkToUtf8(const std::string& in, const std::string& localename = "zh_CN.GB18030");
#endif
}
#endif