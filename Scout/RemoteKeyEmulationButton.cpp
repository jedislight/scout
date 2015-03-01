#include "RemoteKeyEmulationButton.h"

#include <Windows.h>

RemoteKeyEmulationButton::RemoteKeyEmulationButton()
	: SubAction()
	, mKeyToPress(0)
	, mKeystate(false)
{
}


RemoteKeyEmulationButton::~RemoteKeyEmulationButton()
{
}

void RemoteKeyEmulationButton::SetKeyToPress(WORD keyToPress)
{
	mKeyToPress = keyToPress;
}

/* virtual */ void RemoteKeyEmulationButton::Perform(unsigned int mode)
{
	INPUT ip;

	// Set up a generic keyboard event.
	ip.type = INPUT_KEYBOARD;
	ip.ki.wScan = 0; // hardware scan code for key
	ip.ki.time = 0;
	ip.ki.dwExtraInfo = 0;
	ip.ki.wVk = mKeyToPress; 

	if (mode > 0)
	{
		if (mKeystate == false)
		{
			// Press the key
			ip.ki.dwFlags = 0; // 0 for key press
			SendInput(1, &ip, sizeof(INPUT));
			mKeystate = true;
		}
	}
	else
	{
		if (mKeystate == true)
		{
			// Release the key
			ip.ki.dwFlags = KEYEVENTF_KEYUP; // KEYEVENTF_KEYUP for key release
			SendInput(1, &ip, sizeof(INPUT));
			mKeystate = false;
		}
	}
}

/* virtual */ unsigned int RemoteKeyEmulationButton::GetModeCount()
{
	return 2;
}
