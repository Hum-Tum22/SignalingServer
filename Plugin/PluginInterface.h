#pragma once


class PluginInter
{
public:
	typedef enum
	{
		JSON_SDK,
	}InterProtocal;
	PluginInter(InterProtocal type) :pluginType(type) {};
	InterProtocal pluginType;
};