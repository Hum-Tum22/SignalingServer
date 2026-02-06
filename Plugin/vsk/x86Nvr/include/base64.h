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

#pragma once

#include <ctype.h>
#include <string>

void to64frombits(unsigned char *out, const unsigned char *in, int inlen);
int from64tobits(char *out, const char *in);


std::string BinToBase64(const std::string& strBinText);
std::string Base64ToBin(const std::string& strBase64Text);