#include "CodeConversion.h"

CodeConversion::CodeConversion(){}
CodeConversion::~CodeConversion(){}
int CodeConversion::charset_convert1(char *inbuf,size_t inlen,char *outbuf,size_t *  poutlen,char * from_charset,char * to_charset)
{
	char **pin = &inbuf;
	char **pout = &outbuf;
	int outlen = * poutlen;
	GMUTEX lock(m_CodeMutex);
	cd = iconv_open(to_charset,from_charset);
	if (cd == (iconv_t)-1) {
		perror("open error");
		return -1;
	}
	memset(outbuf,0,outlen);
	if (iconv(cd,pin,&inlen,pout,poutlen) == -1) {
		perror("Iconv error");
	}
	iconv_close(cd);
	return 0;
}
int CodeConversion::charset_convert2(std::string insrc,std::string &outdst,char * from_charset,char * to_charset)
{
	size_t inlen = insrc.size();
	char *inbuf = (char*)insrc.c_str();
	
	size_t outlen = inlen * 4;
	char *outbuf = (char*)malloc(outlen);
	if(outbuf == NULL)
	{
		return -2;
	}
	memset(outbuf,0,outlen);
	
	char *pout = outbuf;
	try
	{
		GMUTEX lock(m_CodeMutex);
		cd = iconv_open(to_charset,from_charset);
		if (cd == (iconv_t)-1) {
			perror("open error");
			free((void*)outbuf);
			return -1;
		}
		if (iconv(cd,&inbuf,&inlen,&pout,&outlen) == -1) {
			perror("Iconv error");
		}
		iconv_close(cd);
		outdst = outbuf;
		free((void*)outbuf);
	}
	catch(...)
	{
	}
	return 0;
}
int preNUm(unsigned char byte) {
    unsigned char mask = 0x80;
    int num = 0;
    for (int i = 0; i < 8; i++) {
        if ((byte & mask) == mask) {
            mask = mask >> 1;
            num++;
        } else {
            break;
        }
    }
	return num;
}
bool is_str_utf8(const char* data, int len) {
    int num = 0;
    int i = 0;
    while (i < len) {
        if ((data[i] & 0x80) == 0x00) {
            // 0XXX_XXXX
            i++;
			continue;
        }
        else if ((num = preNUm(data[i])) > 2) {
        // 110X_XXXX 10XX_XXXX
        // 1110_XXXX 10XX_XXXX 10XX_XXXX
        // 1111_0XXX 10XX_XXXX 10XX_XXXX 10XX_XXXX
        // 1111_10XX 10XX_XXXX 10XX_XXXX 10XX_XXXX 10XX_XXXX
        // 1111_110X 10XX_XXXX 10XX_XXXX 10XX_XXXX 10XX_XXXX 10XX_XXXX
        // preNUm() 返回首个字节8个bits中首 0bit前面1bit的个数，该数量也是该字符所使用的字节数        
        i++;
        for(int j = 0; j < num - 1; j++) {
            //判断后面num - 1 个字节是不是都是10开
            if ((data[i] & 0xc0) != 0x80) {
					return false;
				}
				i++;
        }
    } else {
        //其他情况说明不是utf-8
        return false;
    }
    } 
    return true;
}
std::string remove_space(std::string str)
{
	if(!str.empty())
	{
	    int s = str.find_first_not_of(" ");
	    int e = str.find_last_not_of(" ");
		if(s >= 0 && e >= s)
	    	return str.substr(s,e-s+1);
	}
	return "";
}

CodeConversion g_CodeConversion;

int charset_convert(char *inbuf,size_t inlen,char *outbuf,size_t *  poutlen,char * from_charset,char * to_charset)
{
	return g_CodeConversion.charset_convert1(inbuf, inlen, outbuf, poutlen, from_charset, to_charset);
}
int charset_convert(std::string insrc,std::string &outdst,char * from_charset,char * to_charset)
{
	return g_CodeConversion.charset_convert2(insrc, outdst, from_charset, to_charset);
}
std::string Utf8ToGbk(std::string strutf8)
{
	std::string strdst;
	if(charset_convert(strutf8, strdst, "UTF−8", "GB18030") == 0)
	{
		return strdst;
	}
	return strutf8;
}
std::string GbkToUtf8(std::string strgbk)
{
	std::string strdst;
	if(charset_convert(strgbk, strdst, "GB18030", "UTF−8") == 0)
	{
		return strdst;
	}
	return strgbk;
}
std::vector<std::string> vStringSplit(const  std::string& s, const std::string& delim)
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
static const unsigned char e0f4[] = {
	0xa0 | ((2 - 1) << 2) | 1, /* e0 */
	0x80 | ((4 - 1) << 2) | 1, /* e1 */
	0x80 | ((4 - 1) << 2) | 1, /* e2 */
	0x80 | ((4 - 1) << 2) | 1, /* e3 */
	0x80 | ((4 - 1) << 2) | 1, /* e4 */
	0x80 | ((4 - 1) << 2) | 1, /* e5 */
	0x80 | ((4 - 1) << 2) | 1, /* e6 */
	0x80 | ((4 - 1) << 2) | 1, /* e7 */
	0x80 | ((4 - 1) << 2) | 1, /* e8 */
	0x80 | ((4 - 1) << 2) | 1, /* e9 */
	0x80 | ((4 - 1) << 2) | 1, /* ea */
	0x80 | ((4 - 1) << 2) | 1, /* eb */
	0x80 | ((4 - 1) << 2) | 1, /* ec */
	0x80 | ((2 - 1) << 2) | 1, /* ed */
	0x80 | ((4 - 1) << 2) | 1, /* ee */
	0x80 | ((4 - 1) << 2) | 1, /* ef */
	0x90 | ((3 - 1) << 2) | 2, /* f0 */
	0x80 | ((4 - 1) << 2) | 2, /* f1 */
	0x80 | ((4 - 1) << 2) | 2, /* f2 */
	0x80 | ((4 - 1) << 2) | 2, /* f3 */
	0x80 | ((1 - 1) << 2) | 2, /* f4 */

	0,			   /* s0 */
	0x80 | ((4 - 1) << 2) | 0, /* s2 */
	0x80 | ((4 - 1) << 2) | 1, /* s3 */
};

int check_byte_utf8(unsigned char state, unsigned char c)
{
	unsigned char s = state;

	if (!s) {
		if (c >= 0x80) {
			if (c < 0xc2 || c > 0xf4)
				return -1;
			if (c < 0xe0)
				return 0x80 | ((4 - 1) << 2);
			else
				return e0f4[c - 0xe0];
		}

		return s;
	}
	if (c < (s & 0xf0) || c >= (s & 0xf0) + 0x10 + ((s << 2) & 0x30))
		return -1;

	return e0f4[21 + (s & 3)];
}

int check_utf8(unsigned char *state, unsigned char *buf, size_t len)
{
	unsigned char s = *state;

	while (len--) {
		unsigned char c = *buf++;

		if (!s) {
			if (c >= 0x80) {
				if (c < 0xc2 || c > 0xf4)
					return 1;
				if (c < 0xe0)
					s = 0x80 | ((4 - 1) << 2);
				else
					s = e0f4[c - 0xe0];
			}
		} else {
			if (c < (s & 0xf0) ||
			    c >= (s & 0xf0) + 0x10 + ((s << 2) & 0x30))
				return 1;
			s = e0f4[21 + (s & 3)];
		}
	}

	*state = s;

	return 0;
}


