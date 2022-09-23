#ifndef OWN_STRING_H_
#define OWN_STRING_H_
#include <iostream>
#include <vector>

namespace ownString
{
	std::string str_format(const std::string format, ...);
	std::wstring wstr_format(const std::wstring format, ...);

	std::vector<std::string> vStringSplit(const  std::string& s, const std::string& delim = ",");
	std::vector<std::string> SipSubjectSplit(const  std::string& subject);
}
#endif