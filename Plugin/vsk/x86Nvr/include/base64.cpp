/*
 * base64.c -- base-64 conversion routines.
 *
 * For license terms, see the file COPYING in this directory.
 *
 * This base 64 encoding is defined in RFC2045 section 6.8,
 * "Base64 Content-Transfer-Encoding", but lines must not be broken in the
 * scheme used here.
 */

/*
 * This code borrowed from fetchmail sources by
 * Eric S. Raymond <esr@snark.thyrsus.com>.
 */
#include "base64.h"
#include <memory.h>


static const char base64digits[] =
   "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

#define BAD	(char)-1
static const char base64val[] = {
    BAD,BAD,BAD,BAD, BAD,BAD,BAD,BAD, BAD,BAD,BAD,BAD, BAD,BAD,BAD,BAD,
    BAD,BAD,BAD,BAD, BAD,BAD,BAD,BAD, BAD,BAD,BAD,BAD, BAD,BAD,BAD,BAD,
    BAD,BAD,BAD,BAD, BAD,BAD,BAD,BAD, BAD,BAD,BAD, 62, BAD,BAD,BAD, 63,
     52, 53, 54, 55,  56, 57, 58, 59,  60, 61,BAD,BAD, BAD,BAD,BAD,BAD,
    BAD,  0,  1,  2,   3,  4,  5,  6,   7,  8,  9, 10,  11, 12, 13, 14,
     15, 16, 17, 18,  19, 20, 21, 22,  23, 24, 25,BAD, BAD,BAD,BAD,BAD,
    BAD, 26, 27, 28,  29, 30, 31, 32,  33, 34, 35, 36,  37, 38, 39, 40,
     41, 42, 43, 44,  45, 46, 47, 48,  49, 50, 51,BAD, BAD,BAD,BAD,BAD
};
#define DECODE64(c)  (isascii(c) ? base64val[c] : BAD)

void to64frombits(unsigned char *out, const unsigned char *in, int inlen)
/* raw bytes in quasi-big-endian order to base 64 string (NUL-terminated) */
{
    for (; inlen >= 3; inlen -= 3)
    {
	*out++ = base64digits[in[0] >> 2];
	*out++ = base64digits[((in[0] << 4) & 0x30) | (in[1] >> 4)];
	*out++ = base64digits[((in[1] << 2) & 0x3c) | (in[2] >> 6)];
	*out++ = base64digits[in[2] & 0x3f];
	in += 3;
    }
    if (inlen > 0)
    {
	unsigned char fragment;
    
	*out++ = base64digits[in[0] >> 2];
	fragment = (in[0] << 4) & 0x30;
	if (inlen > 1)
	    fragment |= in[1] >> 4;
	*out++ = base64digits[fragment];
	*out++ = (inlen < 2) ? '=' : base64digits[(in[1] << 2) & 0x3c];
	*out++ = '=';
    }
    *out = '\0';
}

int from64tobits(char *out, const char *in)
/* base 64 to raw bytes in quasi-big-endian order, returning count of bytes */
{
	int len = 0;
	register unsigned char digit4;
    if (in[0] == '+' && in[1] == ' ')
	in += 2;
    if (*in == '\r')
		return(0);

	do {
		register unsigned char digit1, digit2, digit3;
		digit1 = in[0];
		if (DECODE64(digit1) == BAD)
			return(-1);
		digit2 = in[1];
		if (DECODE64(digit2) == BAD)
			return(-1);
		digit3 = in[2];
		if (digit3 != '=' && DECODE64(digit3) == BAD)
			return(-1); 
		digit4 = in[3];
		if (digit4 != '=' && DECODE64(digit4) == BAD)
			return(-1);
		in += 4;
		*out++ = (DECODE64(digit1) << 2) | (DECODE64(digit2) >> 4);
		++len;
		if (digit3 != '=')
		{
			*out++ = ((DECODE64(digit2) << 4) & 0xf0) | (DECODE64(digit3) >> 2);
			++len;
			if (digit4 != '=')
			{
			*out++ = ((DECODE64(digit3) << 6) & 0xc0) | DECODE64(digit4);
			++len;
			}
		}
    } while 
	(*in && *in != '\r' && digit4 != '=');

    return (len);
}

/* base64.c ends here */



std::string BinToBase64(const std::string& strBinText)
{
	const static int nStackSize = 512;
	unsigned char chBuf[nStackSize] = {0};
	int nInSize = strBinText.size();
	if (nInSize > (nStackSize >> 1))  // nInSize > (nStackSize / 2)
	{
		unsigned char *pOutBuf = new unsigned char[nInSize + nInSize];
		if (pOutBuf != NULL)
		{
			memset(pOutBuf, 0, nInSize + nInSize);
			to64frombits(pOutBuf, (unsigned char *) strBinText.c_str(), nInSize);
			std::string strRet((char *)pOutBuf);
			delete[] pOutBuf;
			pOutBuf = NULL;
			return strRet;
		}
	}
	else
	{
		to64frombits(chBuf, (unsigned char *) strBinText.c_str(), nInSize);
		return std::string((char *) chBuf);
	}

	return "";
}

std::string Base64ToBin(const std::string& strBase64Text)
{
	const static int nStackSize = 512;
	char chBuf[nStackSize] = {0};
	int nInSize = strBase64Text.size();
	if (nInSize > nStackSize)
	{
		char *pOutBuf = new char[nInSize];
		if (pOutBuf != NULL)
		{
			memset(pOutBuf, 0, nInSize);
			int nLen = from64tobits(pOutBuf, (char *) strBase64Text.c_str());
			if (nLen == 0)
			{
				delete[] pOutBuf;
				pOutBuf = NULL;
				return std::string();
			}
			std::string strRet((char *)pOutBuf, nLen);
			delete[] pOutBuf;
			pOutBuf = NULL;
			return strRet;
		}
	}
	else
	{
		int nLen = from64tobits(chBuf, (char *) strBase64Text.c_str());
		if (nLen > 0)
		{
			return std::string((char *)chBuf, nLen);
		}
	}

	return std::string();
}
