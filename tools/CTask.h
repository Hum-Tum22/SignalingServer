#pragma once

#include "any.h"

namespace ownTask
{
class CTask  //����������
{
private:
	//ownAny::Any m_Any;
protected:
	//int Def;
public:
	CTask() {}
	virtual ~CTask() {}
	virtual bool TaskRun() = 0;
	virtual bool TaskClose() = 0;
	virtual void TaskDestory() {}
};
}
