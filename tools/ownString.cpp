#include "ownString.h"
#include <stdarg.h>
#include <memory>
#include <cstdio>
#include <codecvt>
#include <locale>
#include <string>

//std::string std::str_format(const std::string format, ...)
//{
//    va_list args;
//    va_start(args, format);
//	auto size_buf = std::vsnprintf(nullptr, 0, format.c_str(), args) + 1;
//    va_end(args);
//	std::unique_ptr<char[]> buf(new(std::nothrow) char[size_buf]);
//	if (!buf)
//		return std::string("");
//    va_start(args, format);
//    size_buf = std::vsnprintf(buf.get(), size_buf, format.c_str(), args);
//    va_end(args);
//	return std::string(buf.get(), buf.get() + size_buf);
//}
//std::string std::to_string(const std::wstring& input)
//{
//    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
//    return converter.to_bytes(input);
//}
//std::wstring std::to_wstring(const std::string& input)
//{
//    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
//    return converter.from_bytes(input);
//}
//
//std::vector<std::string> std::vStringSplit(const  std::string& s, const std::string& delim)
//{
//    std::vector<std::string> elems;
//    size_t pos = 0;
//    size_t len = s.length();
//    size_t delim_len = delim.length();
//    if (delim_len == 0) return elems;
//    while (pos < len)
//    {
//        int find_pos = s.find(delim, pos);
//        if (find_pos < 0)
//        {
//            elems.push_back(s.substr(pos, len - pos));
//            break;
//        }
//        elems.push_back(s.substr(pos, find_pos - pos));
//        pos = find_pos + delim_len;
//    }
//    return elems;
//}
//std::vector<std::string> std::SipSubjectSplit(const  std::string& s)
//{
//    std::vector<std::string> elems;
//    std::vector<std::string> subjectArray = std::vStringSplit(s, ",");
//    if (subjectArray.size() >= 2)
//    {
//        elems = std::vStringSplit(subjectArray[0], ":");
//        std::vector<std::string> elems1 = std::vStringSplit(subjectArray[1], ":");
//        elems.insert(elems.end(), elems1.begin(), elems1.end());
//    }
//    return elems;
//}
//std::string std::Utf8ToGbk(const std::string& in, char const* localename)
//{
//	//std::locale(localename);
//	//std::wstring_convert<std::codecvt_utf8<wchar_t>> cvt_utf8; // UTF-8 <-> Unicode转换器 
//	//std::wstring_convert<std::codecvt<wchar_t, char, std::mbstate_t>> cvt_ansi{ new std::codecvt_byname<wchar_t, char, std::mbstate_t>(localename.c_str()) }; // GBK <-> Unicode转换器 
//	//std::wstring ws = cvt_utf8.from_bytes(in); // UTF-8转换为Unicode 
//	//return cvt_ansi.to_bytes(ws);
//
//	std::wstring_convert<std::codecvt_utf8<wchar_t>> cutf8;
//	std::wstring wTemp = cutf8.from_bytes(in);
//	std::locale loc(localename);
//	const wchar_t* pwszNext = nullptr;
//	char* pszNext = nullptr;
//	mbstate_t state = {};
//	std::vector<char> buff(wTemp.size() * 2);
//	int res = std::use_facet<std::codecvt<wchar_t, char, mbstate_t> >
//		(loc).out(state, wTemp.data(), wTemp.data() + wTemp.size(), pwszNext, buff.data(), buff.data() + buff.size(), pszNext);
//	if (std::codecvt_base::ok == res)
//	{
//		return std::string(buff.data(), pszNext);
//	}
//	return "";
//}
//
//std::string std::GbkToUtf8(const std::string& in, char const* localename)
//{
//	//std::wstring_convert<std::codecvt_utf8<wchar_t>> cvt_utf8; // UTF-8 <-> Unicode转换器 
//	//std::wstring_convert<std::codecvt_byname<wchar_t, char, std::mbstate_t>> cvt_ansi{ new std::codecvt_byname<wchar_t, char, std::mbstate_t>(localename.c_str()) }; // GBK <-> Unicode转换器 
//	//std::wstring ws = cvt_ansi.from_bytes(in);
//	//return cvt_utf8.to_bytes(ws);
//
//	std::vector<wchar_t> buff(in.size());
//	std::locale loc(localename);
//	wchar_t* pwszNext = nullptr;
//	const char* pszNext = nullptr;
//	mbstate_t state = {};
//	int res = std::use_facet<std::codecvt<wchar_t, char, mbstate_t> >
//		(loc).in(state, in.data(), in.data() + in.size(), pszNext, buff.data(), buff.data() + buff.size(), pwszNext);
//
//	if (std::codecvt_base::ok == res)
//	{
//		std::wstring_convert<std::codecvt_utf8<wchar_t>> cutf8;
//		return cutf8.to_bytes(std::wstring(buff.data(), pwszNext));
//	}
//	return "";
//}