#include "base64.h"

static unsigned char base64_map[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";//������ʽ,�����޸�

int base64_encode(uint8_t dst[], const uint8_t* src, int srcLen)
{
	unsigned char* str = (unsigned char*)src;	//תΪunsigned char�޷���,��λ����ʱ���Է�ֹ����
	long len;				//base64�������ַ�������
	long str_len;			//Դ�ַ�������
	long flag;				//���ڱ�ʶģ3�������
	unsigned char* res;		//���ص��ַ���
	str_len = srcLen;
	switch (str_len % 3)	//�ж�ģ3������
	{
	case 0:flag = 0; len = str_len / 3 * 4; break;
	case 1:flag = 1; len = (str_len / 3 + 1) * 4; break;
	case 2:flag = 2; len = (str_len / 3 + 1) * 4; break;
	}
	if (sizeof(dst) > len + 1)
	{
		res = dst;
		for (int i = 0, j = 0; j < str_len - flag; j += 3, i += 4)//�ȴ�����������
		{
			//ע��&�����λ����������ȼ�,����λ�ƺ�����
			res[i] = base64_map[str[j] >> 2];
			res[i + 1] = base64_map[(str[j] & 0x3) << 4 | str[j + 1] >> 4];
			res[i + 2] = base64_map[(str[j + 1] & 0xf) << 2 | (str[j + 2] >> 6)];
			res[i + 3] = base64_map[str[j + 2] & 0x3f];
		}
		//�����㱻������ʱ,Ҫ����
		switch (flag)
		{
		case 0:break;	//����ʱֱ���˳�
		case 1:res[len - 4] = base64_map[str[str_len - 1] >> 2];	//ֻʣһ���ַ�ʱ,������λ�õ�����λ
			res[len - 3] = base64_map[(str[str_len - 1] & 0x3) << 4];//��õͶ�λ��������λ,�Զ���0
			res[len - 2] = res[len - 1] = '='; break;				//���������=
		case 2:
			res[len - 4] = base64_map[str[str_len - 2] >> 2];				//ʣ�����ַ�ʱ,������λ�ø���λ
			res[len - 3] = base64_map[(str[str_len - 2] & 0x3) << 4 | str[str_len - 1] >> 4];	//��һ���ַ��Ͷ�λ�͵ڶ����ַ�����λ
			res[len - 2] = base64_map[(str[str_len - 1] & 0xf) << 2];	//�ڶ����ַ�����λ,������λ�Զ���0
			res[len - 1] = '=';											//���һ����=
			break;
		}
	}
	else
	{
		return len + 1;
	}
	res[len] = '\0';	//�����ַ���������ʶ
	return 0;
}
unsigned char findPos(const unsigned char* base64_map, unsigned char c)//�����±�����λ��
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
		a[0] = findPos(base64_map, code[i]);		//code[]ÿһ���ַ���Ӧbase64���е�λ��,��λ��ֵ����ԭʼ����ֵ
		a[1] = findPos(base64_map, code[i + 1]);
		a[2] = findPos(base64_map, code[i + 2]);
		a[3] = findPos(base64_map, code[i + 3]);
		res[j] = a[0] << 2 | a[1] >> 4;		//ȡ����һ���ַ���Ӧbase64���ʮ��������ǰ6λ��ڶ����ַ���Ӧbase64���ʮ�������ĺ�2λ�������  
		res[j + 1] = a[1] << 4 | a[2] >> 2;	//ȡ���ڶ����ַ���Ӧbase64���ʮ�������ĺ�4λ��������ַ���Ӧbas464���ʮ�������ĺ�4λ�������  
		res[j + 2] = a[2] << 6 | a[3];	   //ȡ���������ַ���Ӧbase64���ʮ�������ĺ�2λ���4���ַ�������� 
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
//	//�������� hello
//	//aGVsbG8=
//	//aGVsbG82
//	//aGVsbG==
//	char str[100];
//	int flag;
//	printf("��ѡ����:\n");
//	printf("1.base64����\n");
//	printf("2.base64����\n");
//	scanf("%d", &flag);
//	printf("�������ַ���:\n");
//	scanf("%s", str);
//	if (flag == 1)
//		printf("���ܺ���ַ�����:%s", base64_encode(str));
//	else
//		printf("���ܺ���ַ�����:%s", base64_decode(str));
//	return 0;
//}