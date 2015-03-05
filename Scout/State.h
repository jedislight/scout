#pragma once

#include <map>
#include <set>
#include <vector>

class Action;

class State
{
public:
	typedef std::map<Action*, std::set<State*> > ActionHistoryMap;
	State(void* data, unsigned int length);
	State(unsigned int stateId, double rawValue, unsigned int observedCount, unsigned long long hash, std::vector<std::vector<unsigned int>> actionHistroyActions, std::vector<std::vector<unsigned int>>actionHistroyStateIds);

	virtual ~State();

	virtual double Value();
	virtual double ValueRaw();
	virtual unsigned int ObservedCount();
	virtual unsigned int NormalizeViewBlock( void* data, unsigned int length);
	unsigned long long Hash() const;
	unsigned int Id() const;

	void Finalize();
	void PatchActionHistoryStatePointers(std::map<State*, State*>& statePointersByIdPatchTable);
	void AddStateTransition(Action*, State*);
	ActionHistoryMap& ActionHistory();

	static void SetNextStateId(unsigned int nextId);
	
private:
	double m_value;
	unsigned int m_observedCount;
	ActionHistoryMap m_actionHistory;
	unsigned long long m_hash;
	unsigned int m_stateId;

	static unsigned int ms_nextStateId;
};

