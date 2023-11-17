#pragma once
#include <string>
//<Item>
//<!--虚拟组织标识, 编码采用 D.1中的20位ID格式, 扩展216类型代表虚拟组织-->
//< DeviceID>65010200002160000002 < / DeviceID >
//<Name>虚拟组织名称< / Name>
//<!--父节点虚拟组织ID, 用于标识虚拟组织间的层级关系, 若有父节点虚拟组织则填写此
//	字段-->
//	< ParentID>65010200002160000001 < / ParentID >
//	<!--虚拟组织所属的业务分组ID-->
//	< BusinessGroupID>65010200002150000001 < / BusinessGroupID >
//	< / Item>
typedef struct
{
	std::string DeviceID;
	std::string Name;
	std::string ParentID;
	std::string BusinessGroupID;
}VirtualOrganization;