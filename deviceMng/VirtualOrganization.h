#pragma once
#include <string>
//<Item>
//<!--������֯��ʶ, ������� D.1�е�20λID��ʽ, ��չ216���ʹ���������֯-->
//< DeviceID>65010200002160000002 < / DeviceID >
//<Name>������֯����< / Name>
//<!--���ڵ�������֯ID, ���ڱ�ʶ������֯��Ĳ㼶��ϵ, ���и��ڵ�������֯����д��
//	�ֶ�-->
//	< ParentID>65010200002160000001 < / ParentID >
//	<!--������֯������ҵ�����ID-->
//	< BusinessGroupID>65010200002150000001 < / BusinessGroupID >
//	< / Item>
typedef struct
{
	std::string DeviceID;
	std::string Name;
	std::string ParentID;
	std::string BusinessGroupID;
}VirtualOrganization;