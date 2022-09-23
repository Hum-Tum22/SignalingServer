#include "CodeConvert.h"

#include <codecvt>
#include <locale>
#include <vector>


std::string ownCodeCvt::Utf8ToGbk(const std::string& in, const std::string& localename)
{
	//std::locale(localename);
	//std::wstring_convert<std::codecvt_utf8<wchar_t>> cvt_utf8; // UTF-8 <-> Unicode转换器 
	//std::wstring_convert<std::codecvt<wchar_t, char, std::mbstate_t>> cvt_ansi{ new std::codecvt_byname<wchar_t, char, std::mbstate_t>(localename.c_str()) }; // GBK <-> Unicode转换器 
	//std::wstring ws = cvt_utf8.from_bytes(in); // UTF-8转换为Unicode 
	//return cvt_ansi.to_bytes(ws);

	std::wstring_convert<std::codecvt_utf8<wchar_t>> cutf8;
	std::wstring wTemp = cutf8.from_bytes(in);
	std::locale loc(localename);
	const wchar_t* pwszNext = nullptr;
	char* pszNext = nullptr;
	mbstate_t state = {};
	std::vector<char> buff(wTemp.size() * 2);
	int res = std::use_facet<std::codecvt<wchar_t, char, mbstate_t> >
		(loc).out(state, wTemp.data(), wTemp.data() + wTemp.size(), pwszNext, buff.data(), buff.data() + buff.size(), pszNext);
	if (std::codecvt_base::ok == res)
	{
		return std::string(buff.data(), pszNext);
	}
	return "";
}

std::string ownCodeCvt::GbkToUtf8(const std::string& in, const std::string& localename)
{
	//std::wstring_convert<std::codecvt_utf8<wchar_t>> cvt_utf8; // UTF-8 <-> Unicode转换器 
	//std::wstring_convert<std::codecvt_byname<wchar_t, char, std::mbstate_t>> cvt_ansi{ new std::codecvt_byname<wchar_t, char, std::mbstate_t>(localename.c_str()) }; // GBK <-> Unicode转换器 
	//std::wstring ws = cvt_ansi.from_bytes(in);
	//return cvt_utf8.to_bytes(ws);

	std::vector<wchar_t> buff(in.size());
	std::locale loc(localename);
	wchar_t* pwszNext = nullptr;
	const char* pszNext = nullptr;
	mbstate_t state = {};
	int res = std::use_facet<std::codecvt<wchar_t, char, mbstate_t> >
		(loc).in(state, in.data(), in.data() + in.size(), pszNext, buff.data(), buff.data() + buff.size(), pwszNext);

	if (std::codecvt_base::ok == res)
	{
		std::wstring_convert<std::codecvt_utf8<wchar_t>> cutf8;
		return cutf8.to_bytes(std::wstring(buff.data(), pwszNext));
	}
	return "";
}