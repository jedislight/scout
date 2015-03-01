#include "State.h"
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

	memset(m_view, 0, sizeof(unsigned int) * 255);

	double stride = length / 256.0;
	for (int i = 0; i < 255; ++i)
	{
		unsigned int beginBlock = static_cast<unsigned int>(floor(i * stride));
		unsigned int endBlock = static_cast<unsigned int>(ceil((i + 1) * stride));

		m_view[i] = NormalizeViewBlock(static_cast<char*>(data)+beginBlock, endBlock - beginBlock);
		m_hash += uintHash(m_view[i]);
	}	
}


State::~State()
{
}

unsigned int State::Data(unsigned int index) const
{
	return m_view[index];
}

unsigned int State::Hash() const
{
	return m_hash;
}

unsigned int State::Id() const
{
	return m_stateId;
}

void State::AddStateTransition(Action* action, State* state)
{
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

/* virtual */ double State::Value()
{
	return 1.0/m_observedCount;
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