#include "Agent.h"

#include "State.h"
#include "Action.h"

#include <algorithm>
#include <time.h>
#include <queue>

Agent::Agent(SubAction** subActions, unsigned int length)
{
	for (size_t i = 0; i < length; i++)
	{
		mSubActions.push_back(subActions[i]);
	}

	mActionsTaken = 0;
	mExplorationFactor = 2.0;
	mGenerator.seed(static_cast<unsigned long>(time(nullptr)));
}


Agent::~Agent()
{
}

State* Agent::CurrentState()
{
	return mCurrentState;
}

void Agent::SetExplorationFactor(double factor)
{
	mExplorationFactor = factor+0.9999999;
}

unsigned int Agent::StateCount()
{
	return mStatesByHash.size();
}

unsigned int Agent::ActionsTaken()
{
	return mActionsTaken;
}

double Agent::GetStateValueRecursive(State* state, unsigned int lookAhead)
{
	double result = 0.0;

	std::queue<State*> stateQ;
	stateQ.push(state);

	unsigned int totalNodesPushed = 1;
	while (!stateQ.empty())
	{
		state = stateQ.front();
		stateQ.pop();
		if (totalNodesPushed < lookAhead)
		{
			for (auto stateActionHistoryIt : state->ActionHistory())
			{
				for (State* reachableState : stateActionHistoryIt.second)
				{
					stateQ.push(reachableState);
					totalNodesPushed += 1;
				}
			}
		}

		result += state->Value();
	}

	return result;
}

/* virtual */ void Agent::Observe(void* data, unsigned int length)
{
	State* newState = new State(data, length);
	auto existingStateIt = mStatesByHash.find(newState->Hash());
	if (existingStateIt == mStatesByHash.end())
	{
		mStatesByHash[newState->Hash()] = newState;
		newState->Finalize();
	}
	else
	{
		delete newState;
		newState = existingStateIt->second;
	}

	if (mCurrentState != nullptr)
	{
		mCurrentState->AddStateTransition(mPreviousAction, newState);
	}

	mCurrentState = newState;
}

/* virtual */ void Agent::Act(unsigned int lookAhead)
{
	State::ActionHistoryMap& currentStateActionHistroy = mCurrentState->ActionHistory();
	std::map<Action*, double> possibleActions;

	unsigned int* subActionStates = new unsigned int[mSubActions.size()];
	for (unsigned int i = 0; i < mSubActions.size(); ++i)
	{
		std::uniform_int_distribution<int> distribution(0, mSubActions[i]->GetModeCount() - 1);
		subActionStates[i] = distribution(mGenerator);
	}

	Action* somethingNew = new Action(subActionStates, mSubActions.size());
	delete subActionStates;

	double highestKnownValue = 0.0;
	for (auto pairIt : currentStateActionHistroy)
	{
		Action* action = pairIt.first;
		auto subActionStates = action->SubActionStates();

		if (somethingNew != nullptr && 0 == memcmp(&subActionStates[0], &somethingNew->SubActionStates()[0], sizeof(unsigned int) * mSubActions.size()))
		{
			delete somethingNew;
			somethingNew = nullptr;
		}

		double value = 0.0;
		for (State* state : pairIt.second)
		{
			value = GetStateValueRecursive(state, lookAhead);
		}

		possibleActions[action] = value;
		highestKnownValue = std::max(value, highestKnownValue);
	}

	if (somethingNew != nullptr)
	{
		std::uniform_real_distribution<double> distribution(0.0, highestKnownValue*mExplorationFactor);
		possibleActions[somethingNew] = distribution(mGenerator);
	}

	//select action
	Action* bestAction = possibleActions.begin()->first;
	double highestExpectedValue = possibleActions.begin()->second;
	for (auto possibleActionIt : possibleActions)
	{
		Action* action = possibleActionIt.first;
		double value = possibleActionIt.second;

		if (value > highestExpectedValue)
		{
			bestAction = action;
			highestExpectedValue = value;
		}
	}

	//do action
	bestAction->Perform(&mSubActions[0], mSubActions.size());
	mPreviousAction = bestAction;
	mActionsTaken += 1;
}
