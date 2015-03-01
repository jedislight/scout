#include "Action.h"
#include "SubAction.h"

Action::Action(unsigned int* subActionStates, unsigned int length)
{
	for (unsigned int i = 0; i < length; ++i)
	{
		m_subActionStates.push_back(subActionStates[i]);
	}
}


Action::~Action()
{
}

std::vector<unsigned int>& Action::SubActionStates()
{
	return m_subActionStates;
}

/* virtual */ void Action::Perform(SubAction** subActions, unsigned int length)
{
	for (unsigned int i = 0; i < length; ++i)
	{
		subActions[i]->Perform(m_subActionStates[i]);
	}
}