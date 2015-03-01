#pragma once
#include "SubAction.h"
#include <string>
#include <Windows.h>
class RemoteKeyEmulationButton : public SubAction
{
public:
	RemoteKeyEmulationButton();
	virtual ~RemoteKeyEmulationButton();

	void SetKeyToPress(WORD keyToPress);

	virtual void Perform(unsigned int);
	virtual unsigned int GetModeCount();

private:
	WORD mKeyToPress;
	bool mKeystate;
};

