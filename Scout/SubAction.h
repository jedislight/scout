#pragma once
class SubAction
{
public:
	SubAction();
	virtual ~SubAction();

	virtual void Perform(unsigned int) = 0;
	virtual unsigned int GetModeCount() = 0;
};

