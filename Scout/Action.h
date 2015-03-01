#pragma once

#include <vector>

class SubAction;

class Action
{
public:
	Action(unsigned int* subActionStates, unsigned int length);
	virtual ~Action();
	virtual void Perform(SubAction** subActions, unsigned int length);
	
	std::vector<unsigned int>& SubActionStates();

private:
	std::vector<unsigned int> m_subActionStates;
};

