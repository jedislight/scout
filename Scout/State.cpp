#include "State.h"
#include "Action.h"
#include <math.h>
#include <functional>


unsigned int State::ms_nextStateId = 0;

State::State(void* data, unsigned int length)
	: m_value(0)
	, m_observedCount(0)
	, m_actionHistory()
	, m_hash(0)
{
	const std::hash<unsigned int> uintHash;
	const std::hash<unsigned long long> ulonglongHash;

	unsigned int view[256];
	memset(view, 0, sizeof(unsigned int) * 256);

	double stride = length / 256.0;
	std::vector<unsigned int> hashes;
	for (int i = 0; i < 256; ++i)
	{
		unsigned int beginBlock = static_cast<unsigned int>(floor(i * stride));
		unsigned int endBlock = static_cast<unsigned int>(ceil((i + 1) * stride));

		view[i] = NormalizeViewBlock(static_cast<char*>(data)+beginBlock, endBlock - beginBlock);
		hashes.push_back(uintHash(view[i]));
	}

	unsigned int simpleHashCombine = hashes[0];
	unsigned int hashReduceResult = hashes[0];
	for (int i = 1; i < 256; ++i)
	{
		simpleHashCombine += hashes[i];
		unsigned long long hashReduce = hashes[i] + (static_cast<unsigned long long>(hashReduceResult) << 32);
		hashReduceResult = ulonglongHash(hashReduce);
	}
	m_hash = simpleHashCombine + (static_cast<unsigned long long>(hashReduceResult) << 32);
}

State::State(unsigned int stateId, double rawValue, unsigned int observedCount, unsigned long long hash, std::vector<std::vector<unsigned int>> actionHistroyActions, std::vector<std::vector<unsigned int>>actionHistroyStateIds)
	: m_value(rawValue)
	, m_observedCount(observedCount)
	, m_actionHistory()
	, m_hash(hash)
	, m_stateId(stateId)
{
	for (unsigned int i = 0; i < actionHistroyActions.size(); ++i)
	{
		Action* action = new Action(actionHistroyActions[i].data(), actionHistroyActions[i].size());
		char* nullState = nullptr;

		for (unsigned int j = 0; j < actionHistroyStateIds[i].size(); j++)
		{
			m_actionHistory[action].insert(reinterpret_cast<State*>(nullState + actionHistroyStateIds[i][j]));
		}
	}
}

void State::PatchActionHistoryStatePointers(std::map<State*, State*>& statePointersByIdPatchTable)
{
	for (auto& actionHistoryMapIt : m_actionHistory)
	{
		std::vector<State*> toConvert;
		for (State* state : actionHistoryMapIt.second)
		{
			toConvert.push_back(state);
		}
		actionHistoryMapIt.second.clear();
		
		for (unsigned int i = 0; i < toConvert.size(); i++)
		{
			toConvert[i] = statePointersByIdPatchTable[toConvert[i]];
		}

		for (State* state : toConvert)
		{
			actionHistoryMapIt.second.insert(state);
			if (state == nullptr)
			{
				static int x = 0;
				++x;
			}
		}
	}
}

State::~State()
{
}

unsigned long long State::Hash() const
{
	return m_hash;
}

unsigned int State::Id() const
{
	return m_stateId;
}

void State::AddStateTransition(Action* action, State* state)
{
	if (state == nullptr)
	{
		static int x = 0;
		++x;
	}
	m_actionHistory[action].insert(state);
	++state->m_observedCount;
}

void State::Finalize()
{
	m_stateId = ms_nextStateId++;
}

State::ActionHistoryMap& State::ActionHistory()
{
	return m_actionHistory;
}

/* static */ void State::SetNextStateId(unsigned int nextId)
{
	ms_nextStateId = nextId;
}

/* virtual */ double State::Value()
{
	return ValueRaw() - m_observedCount - m_actionHistory.size();
}

/* virtual */ double State::ValueRaw()
{
	return 1.0;
}

/* virtual */ unsigned int State::ObservedCount()
{
	return m_observedCount;
}

/* virtual */ unsigned int State::NormalizeViewBlock(void* data, unsigned int length)
{
	//AVERAGE
	unsigned int bufferSize = length + (sizeof(unsigned int) - length % sizeof(unsigned int));
	char* buffer = new char[bufferSize]{};
	memcpy(buffer, data, length);

	double resultHighPrecsion = 0.0;
	double numElements = bufferSize / sizeof(unsigned int);
	unsigned int * dataValueBuffer = reinterpret_cast<unsigned int*>(buffer);
	for (unsigned int i = 0; i < bufferSize / sizeof(unsigned int); ++i)
	{
		unsigned int dataValue = dataValueBuffer[i];
		resultHighPrecsion += dataValue / numElements;
	}

	delete[] buffer;

	return static_cast<unsigned int>(round(resultHighPrecsion));
}