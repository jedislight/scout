#pragma once

#include <map>
#include <set>

class Action;

class State
{
public:
	typedef std::map<Action*, std::set<State*> > ActionHistoryMap;
	State(void* data, unsigned int length);
	virtual ~State();

	virtual double Value();
	virtual unsigned int ObservedCount();
	virtual unsigned int NormalizeViewBlock( void* data, unsigned int length);
	unsigned int Data(unsigned int index) const;
	unsigned int Hash() const;
	unsigned int Id() const;

	void Finalize();
	void AddStateTransition(Action*, State*);
	ActionHistoryMap& ActionHistory();
	
private:
	unsigned int m_view[255];
	double m_value;
	unsigned int m_observedCount;
	ActionHistoryMap m_actionHistory;
	unsigned int m_hash;
	unsigned int m_stateId;

	static unsigned int ms_nextStateId;
};

