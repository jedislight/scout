#include "Agent.h"

#include "State.h"
#include "Action.h"

#include <algorithm>
#include <time.h>
#include <queue>
#include <iostream>
#include <fstream>
#include <cfloat>

#include <Windows.h>

Agent::Agent(SubAction** subActions, unsigned int length)
{
	CreateDirectory("Agent", NULL);
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
			for (auto& stateActionHistoryIt : state->ActionHistory())
			{
				for (State* reachableState : stateActionHistoryIt.second)
				{
					if (reachableState == nullptr)
					{
						static int x = 0;
						++x;
					}
					stateQ.push(reachableState);
					totalNodesPushed += 1;
				}
			}
		}

		result += state->Value();
	}

	return result;
}

void Agent::SaveCurrentState()
{
	unsigned int stateId = mCurrentState->Id();
	char fileName[1024]{};
	sprintf(fileName, "Agent\\%u.state", stateId);
	std::ofstream stateFile(fileName, std::ios::out);
	stateFile << stateId << std::endl;
	stateFile << mCurrentState->ValueRaw() << std::endl;
	stateFile << mCurrentState->ObservedCount() << std::endl;
	stateFile << mCurrentState->Hash() << std::endl;
	stateFile << mCurrentState->ActionHistory().size() << std::endl;
	for (auto& actionMapIt : mCurrentState->ActionHistory())
	{
		Action* action = actionMapIt.first;
		for (unsigned int subActionState : action->SubActionStates())
		{
			stateFile << subActionState << " ";
		}

		stateFile << actionMapIt.second.size();
		for (State* state : actionMapIt.second)
		{
			stateFile << " " << state->Id();
		}
		stateFile << std::endl;
	}
	stateFile.close();
}

void Agent::LoadFromFolder(const char* folder)
{
	std::map<State*, State*> statePointersByIdPatchTable;
	std::cout << "Loading in data from folder: " << folder << std::endl;
	for (unsigned int i = 0; i < UINT_MAX; ++i)
	{
		char filename[1024];
		sprintf(filename, "Agent\\%u.state", i);
		std::ifstream inStateFile(filename, std::ios::in);
		if (!inStateFile.good())
		{
			State::SetNextStateId(i);
			break;
		}

		if (i % 1000 == 0)
		{
			std::cout << "Loading State " << i << std::endl;
		}

		unsigned int stateId;
		double rawValue;
		unsigned int observedCount;
		unsigned long long hash;
		unsigned int expectedActionCount;
		
		inStateFile >> stateId;
		inStateFile >> rawValue;
		inStateFile >> observedCount;
		inStateFile >> hash;
		inStateFile >> expectedActionCount;

		std::vector<std::vector<unsigned int>> actionHistroyActions;
		std::vector<std::vector<unsigned int>> actionHistroyStateIds;
		unsigned int actionsRead = 0;
		for (unsigned int a = 0; a < expectedActionCount; ++a)
		{
			actionHistroyActions.push_back(std::vector<unsigned int>());
			for (unsigned int i = 0; i < this->mSubActions.size(); ++i)
			{
				unsigned int subActionState;
				inStateFile >> subActionState;
				actionHistroyActions[actionsRead].push_back(subActionState);
			}
			unsigned int stateTransitionCount;
			inStateFile >> stateTransitionCount;
			actionHistroyStateIds.push_back(std::vector<unsigned int>());
			for (unsigned int i = 0; i < stateTransitionCount; ++i)
			{
				unsigned int stateId;
				inStateFile >> stateId;
				actionHistroyStateIds[actionsRead].push_back(stateId);
			}
			++actionsRead;
		}

		State* state = new State(stateId, rawValue, observedCount, hash, actionHistroyActions, actionHistroyStateIds);
		char* nullState = nullptr;
		statePointersByIdPatchTable[reinterpret_cast<State*>(nullState + stateId)] = state;
		mStatesByHash[hash] = state;
	}

	unsigned int numStates = mStatesByHash.size();
	if (numStates > 0)
	{
		unsigned int stateCount = 0;

		std::cout << "Patching " << numStates << " State's Pointers";
		for (auto& stateByHashIt : mStatesByHash)
		{
			State* state = stateByHashIt.second;
			++stateCount;
			if (stateCount % 1000 == 0)
			{
				std::cout << "Patching State " << stateCount << "/" << numStates << " Pointers" << std::endl;
			}
			state->PatchActionHistoryStatePointers(statePointersByIdPatchTable);
		}
	}
}

/* virtual */ bool Agent::Observe(void* data, unsigned int length)
{
	bool foundNewState = false;
	State* newState = new State(data, length);
	auto existingStateIt = mStatesByHash.find(newState->Hash());
	if (existingStateIt == mStatesByHash.end())
	{
		mStatesByHash[newState->Hash()] = newState;
		newState->Finalize();
		foundNewState = true;
	}
	else
	{
		delete newState;
		newState = existingStateIt->second;
	}

	if (mCurrentState != nullptr && mPreviousAction != nullptr)
	{
		mCurrentState->AddStateTransition(mPreviousAction, newState);
		SaveCurrentState();
	}

	mCurrentState = newState;

	SaveCurrentState();

	return foundNewState;
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

	double highestKnownValue = -DBL_MAX;
	double lowestKnownValue = DBL_MAX;
	for (auto& pairIt : currentStateActionHistroy)
	{
		Action* action = pairIt.first;
		auto subActionStates = action->SubActionStates();

		if (somethingNew != nullptr && 0 == memcmp(&subActionStates[0], &somethingNew->SubActionStates()[0], sizeof(unsigned int) * mSubActions.size()))
		{
			delete somethingNew;
			somethingNew = nullptr;
		}

		double value = 0.0;
		unsigned int lookAheadPerAction = lookAhead / pairIt.second.size();
		for (State* state : pairIt.second)
		{
			value = GetStateValueRecursive(state, lookAheadPerAction);
		}

		possibleActions[action] = value;
		highestKnownValue = value > highestKnownValue ? value : highestKnownValue;
		lowestKnownValue = value < lowestKnownValue ? value : lowestKnownValue;
	}

	if (highestKnownValue < lowestKnownValue)
	{
		highestKnownValue = 1.0;
		lowestKnownValue = 0.0;
	}

	if (somethingNew != nullptr)
	{
		std::uniform_real_distribution<double> distribution(lowestKnownValue, highestKnownValue);
		double randomValue = distribution(mGenerator);
		double weightedPosativeValue = abs(randomValue) * mExplorationFactor;
		double finalValue = randomValue + weightedPosativeValue;
		possibleActions[somethingNew] = finalValue;
	}

	//select action
	Action* bestAction = possibleActions.begin()->first;
	double highestExpectedValue = possibleActions.begin()->second;
	for (auto& possibleActionIt : possibleActions)
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
