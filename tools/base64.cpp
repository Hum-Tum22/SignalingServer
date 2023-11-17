#include "base64.h"

static unsigned char base64_map[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";//数组形式,方便修改

int base64_encode(uint8_t dst[], const uint8_t* src, int srcLen)
{
	unsigned char* str = (unsigned char*)src;	//转为unsigned char无符号,移位操作时可以防止错误
	long len;				//base64处理后的字符串长度
	long str_len;			//源字符串长度
	long flag;				//用于标识模3后的余数
	unsigned char* res;		//返回的字符串
	str_len = srcLen;
	switch (str_len % 3)	//判断模3的余数
	{
	case 0:flag = 0; len = str_len / 3 * 4; break;
	case 1:flag = 1; len = (str_len / 3 + 1) * 4; break;
	case 2:flag = 2; len = (str_len / 3 + 1) * 4; break;
	}
	if (sizeof(dst) > len + 1)
	{
		res = dst;
		for (int i = 0, j = 0; j < str_len - flag; j += 3, i += 4)//先处理整除部分
		{
			//注意&运算和位移运算的优先级,是先位移后与或非
			res[i] = base64_map[str[j] >> 2];
			res[i + 1] = base64_map[(str[j] & 0x3) << 4 | str[j + 1] >> 4];
			res[i + 2] = base64_map[(str[j + 1] & 0xf) << 2 | (str[j + 2] >> 6)];
			res[i + 3] = base64_map[str[j + 2] & 0x3f];
		}
		//不满足被三整除时,要矫正
		switch (flag)
		{
		case 0:break;	//满足时直接退出
		case 1:res[len - 4] = base64_map[str[str_len - 1] >> 2];	//只剩一个字符时,右移两位得到高六位
			res[len - 3] = base64_map[(str[str_len - 1] & 0x3) << 4];//获得低二位再右移四位,自动补0
			res[len - 2] = res[len - 1] = '='; break;				//最后两个补=
		case 2:
			res[len - 4] = base64_map[str[str_len - 2] >> 2];				//剩两个字符时,右移两位得高六位
			res[len - 3] = base64_map[(str[str_len - 2] & 0x3) << 4 | str[str_len - 1] >> 4];	//第一个字符低二位和第二个字符高四位
			res[len - 2] = base64_map[(str[str_len - 1] & 0xf) << 2];	//第二个字符低四位,左移两位自动补0
			res[len - 1] = '=';											//最后一个补=
			break;
		}
	}
	else
	{
		return len + 1;
	}
	res[len] = '\0';	//补上字符串结束标识
	return 0;
}
unsigned char findPos(const unsigned char* base64_map, unsigned char c)//查找下标所在位置
{
	for (uint32_t i = 0; i < strlen((const char*)base64_map); i++)
	{
		if (base64_map[i] == c)
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
	if (code[len - 1] == '=')
	{
		if (code[len - 2] == '=')
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
	for (int i = 0, j = 0; j < str_len - flag; j += 3, i += 4)
	{
		a[0] = findPos(base64_map, code[i]);		//code[]每一个字符对应base64表中的位置,用位置值反推原始数据值
		a[1] = findPos(base64_map, code[i + 1]);
		a[2] = findPos(base64_map, code[i + 2]);
		a[3] = findPos(base64_map, code[i + 3]);
		res[j] = a[0] << 2 | a[1] >> 4;		//取出第一个字符对应base64表的十进制数的前6位与第二个字符对应base64表的十进制数的后2位进行组合  
		res[j + 1] = a[1] << 4 | a[2] >> 2;	//取出第二个字符对应base64表的十进制数的后4位与第三个字符对应bas464表的十进制数的后4位进行组合  
		res[j + 2] = a[2] << 6 | a[3];	   //取出第三个字符对应base64表的十进制数的后2位与第4个字符进行组合 
	}

	switch (flag)
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

//int main()
//{
//	//测试数据 hello
//	//aGVsbG8=
//	//aGVsbG82
//	//aGVsbG==
//	char str[100];
//	int flag;
//	printf("请选择功能:\n");
//	printf("1.base64加密\n");
//	printf("2.base64解密\n");
//	scanf("%d", &flag);
//	printf("请输入字符串:\n");
//	scanf("%s", str);
//	if (flag == 1)
//		printf("加密后的字符串是:%s", base64_encode(str));
//	else
//		printf("解密后的字符串是:%s", base64_decode(str));
//	return 0;
//}