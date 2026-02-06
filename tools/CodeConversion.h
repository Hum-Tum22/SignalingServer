#ifndef CODE_CONVERSION_H
#define CODE_CONVERSION_H
#include <iconv.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <string>
#include <mutex>

#include <iostream>
#include <string>
#include <vector>

/*	
Full Unicode
UTF-8
UCS-2, UCS-2BE, UCS-2LE
UCS-4, UCS-4BE, UCS-4LE
UTF-16, UTF-16BE, UTF-16LE
UTF-32, UTF-32BE, UTF-32LE
UTF-7
C99, JAVA
*/
/*
Chinese
EUC-CN, HZ, GBK, CP936, GB18030, EUC-TW, BIG5, CP950, BIG5-HKSCS, BIG5-HKSCS:2001, BIG5-HKSCS:1999, ISO-2022-CN, ISO-2022-CN-EXT
*/
// GB2312 标准共收录 6763 个汉字
// GBK 共收入 21886 个汉字和图形符号
// GB18030 与 GB 2312-1980 和 GBK 兼容，共收录汉字70244个
class CodeConversion
{
public:
	CodeConversion();
	~CodeConversion();
	int charset_convert1(char *inbuf,size_t inlen,char *outbuf,size_t *  poutlen,char * from_charset,char * to_charset);
	int charset_convert2(std::string insrc,std::string &outdst,char * from_charset,char * to_charset);
private:
	using GMUTEX = std::lock_guard<std::mutex>;
	iconv_t cd;
	std::mutex m_CodeMutex;
};
int charset_convert(char *inbuf,size_t inlen,char *outbuf,size_t *  poutlen,char * from_charset,char * to_charset);
int charset_convert(std::string insrc,std::string &outdst,char * from_charset,char * to_charset);
bool is_str_utf8(const char* data, int len);
std::string Utf8ToGbk(std::string strutf8);//utf8=>GB18030
std::string GbkToUtf8(std::string strgbk);//GB18030=>utf8
//去除前后空格
std::string remove_space(std::string str);
std::vector<std::string> vStringSplit(const  std::string& s, const std::string& delim);
#endif