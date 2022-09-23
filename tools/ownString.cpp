#include "ownString.h"
#include <stdarg.h>
#include <memory>

std::string ownString::str_format(const std::string format, ...)
{
    va_list args;
    va_start(args, format);
	auto size_buf = std::snprintf(nullptr, 0, format.c_str(), args) + 1;
    va_end(args);
	std::unique_ptr<char[]> buf(new(std::nothrow) char[size_buf]);
	if (!buf)
		return std::string("");
    va_start(args, format);
	std::snprintf(buf.get(), size_buf, format.c_str(), args);
    va_end(args);
	return std::string(buf.get(), buf.get() + size_buf - 1);
}
std::wstring ownString::wstr_format(const std::wstring format, ...)
{
   /* va_list args;
    va_start(args, format);
	auto size_buf = std::snprintf(nullptr, 0, format.c_str(), args) + 1;
    va_end(args);

	std::unique_ptr<char[]> buf(new(std::nothrow) char[size_buf]);
	if (!buf)
		return std::wstring();
    va_start(args, format);
	std::snprintf(buf.get(), size_buf, format.c_str(), args);
    va_end(args);
	return std::wstring(buf.get(), buf.get() + size_buf - 1);*/
    return std::wstring();
}
std::vector<std::string> ownString::vStringSplit(const  std::string& s, const std::string& delim)
{
    std::vector<std::string> elems;
    size_t pos = 0;
    size_t len = s.length();
    size_t delim_len = delim.length();
    if (delim_len == 0) return elems;
    while (pos < len)
    {
        int find_pos = s.find(delim, pos);
        if (find_pos < 0)
        {
            elems.push_back(s.substr(pos, len - pos));
            break;
        }
        elems.push_back(s.substr(pos, find_pos - pos));
        pos = find_pos + delim_len;
    }
    return elems;
}
std::vector<std::string> ownString::SipSubjectSplit(const  std::string& s)
{
    std::vector<std::string> elems;
    std::vector<std::string> subjectArray = ownString::vStringSplit(s, ",");
    if (subjectArray.size() >= 2)
    {
        elems = ownString::vStringSplit(subjectArray[0], ":");
        std::vector<std::string> elems1 = ownString::vStringSplit(subjectArray[1], ":");
        elems.insert(elems.end(), elems1.begin(), elems1.end());
    }
    return elems;
}