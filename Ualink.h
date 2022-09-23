#pragma once
#include "resip/stack/Uri.hxx"


class UaLinkInfo
{
public:
	UaLinkInfo();
	~UaLinkInfo();
private:
	resip::Uri target;
};