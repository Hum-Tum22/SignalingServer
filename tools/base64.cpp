#include "base64.h"

static unsigned char base64_map[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

int base64_encode(uint8_t dst[], const uint8_t* src, int srcLen)
{
    unsigned char* str = (unsigned char*)src;
    long len;
    long str_len;
    long flag;
    unsigned char* res;
    str_len = srcLen;
    switch(str_len % 3)
    {
    case 0:flag = 0; len = str_len / 3 * 4; break;
    case 1:flag = 1; len = (str_len / 3 + 1) * 4; break;
    case 2:flag = 2; len = (str_len / 3 + 1) * 4; break;
    }
    if(sizeof(dst) > len + 1)
    {
        res = dst;
        for(int i = 0, j = 0; j < str_len - flag; j += 3, i += 4)
        {
            res[i] = base64_map[str[j] >> 2];
            res[i + 1] = base64_map[(str[j] & 0x3) << 4 | str[j + 1] >> 4];
            res[i + 2] = base64_map[(str[j + 1] & 0xf) << 2 | (str[j + 2] >> 6)];
            res[i + 3] = base64_map[str[j + 2] & 0x3f];
        }
        switch(flag)
        {
        case 0:break;
        case 1:res[len - 4] = base64_map[str[str_len - 1] >> 2];
            res[len - 3] = base64_map[(str[str_len - 1] & 0x3) << 4];
            res[len - 2] = res[len - 1] = '='; break;
        case 2:
            res[len - 4] = base64_map[str[str_len - 2] >> 2];
            res[len - 3] = base64_map[(str[str_len - 2] & 0x3) << 4 | str[str_len - 1] >> 4];
            res[len - 2] = base64_map[(str[str_len - 1] & 0xf) << 2];
            res[len - 1] = '=';
            break;
        }
    }
    else
    {
        return len + 1;
    }
    res[len] = '\0';
    return 0;
}
unsigned char findPos(const unsigned char* base64_map, unsigned char c)
{
    for(uint32_t i = 0; i < strlen((const char*)base64_map); i++)
    {
        if(base64_map[i] == c)
            return i;
    }
    return 0;
}
unsigned char* base64_decode(const char* code0)
{
    unsigned char* code = (unsigned char*)code0;
    long len, str_len, flag = 0;
    unsigned char* res;
    len = strlen((const char*)code);
    if(code[len - 1] == '=')
    {
        if(code[len - 2] == '=')
        {
            flag = 1;
            str_len = len / 4 * 3 - 2;
        }

        else
        {
            flag = 2;
            str_len = len / 4 * 3 - 1;
        }

    }
    else
        str_len = len / 4 * 3;
    unsigned char a[4];
    res = (unsigned char*)malloc(sizeof(unsigned char) * str_len + 1);
    for(int i = 0, j = 0; j < str_len - flag; j += 3, i += 4)
    {
        a[0] = findPos(base64_map, code[i]);
        a[1] = findPos(base64_map, code[i + 1]);
        a[2] = findPos(base64_map, code[i + 2]);
        a[3] = findPos(base64_map, code[i + 3]);
        res[j] = a[0] << 2 | a[1] >> 4;
        res[j + 1] = a[1] << 4 | a[2] >> 2;
        res[j + 2] = a[2] << 6 | a[3];
    }

    switch(flag)
    {
    case 0:break;
    case 1:
    {
        a[0] = findPos(base64_map, code[len - 4]);
        a[1] = findPos(base64_map, code[len - 3]);
        res[str_len - 1] = a[0] << 2 | a[1] >> 4;
        break;
    }
    case 2: {
        a[0] = findPos(base64_map, code[len - 4]);
        a[1] = findPos(base64_map, code[len - 3]);
        a[2] = findPos(base64_map, code[len - 2]);
        res[str_len - 2] = a[0] << 2 | a[1] >> 4;
        res[str_len - 1] = a[1] << 4 | a[2] >> 2;
        break;
    }
    }
    res[str_len] = '\0';
    return res;
}