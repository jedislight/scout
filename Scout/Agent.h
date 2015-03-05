#pragma once

#include "SubAction.h"

#include <map>
#include <vector>
#include <random>

class State;
class Action;

class Agent
{
public:
	Agent(SubAction** subActions, unsigned int length);
	virtual ~Agent();

	State* CurrentState();
	virtual bool Observe(void* data, unsigned int length);
	virtual void Act(unsigned int lookAhead = 100);

	void SetExplorationFactor(double factor);
	unsigned int StateCount();
	unsigned int ActionsTaken();

	void LoadFromFolder(const char* folder);
	
private:
	double GetStateValueRecursive(State* state, unsigned int lookAhead);
	void SaveCurrentState();

	std::map<unsigned long long, State*> mStatesByHash;
	State* mCurrentState;
	Action* mPreviousAction;
	std::vector<SubAction*> mSubActions;
	unsigned int mActionsTaken;
	double mExplorationFactor;
	std::default_random_engine mGenerator;
};

