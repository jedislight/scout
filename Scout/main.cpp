#include "Agent.h"
#include "SubAction.h"
#include "RemoteKeyEmulationButton.h"
#include "State.h"

#include "WindowsScreenCapture.h"

#include <vector>
#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <Windows.h>

int main()
{
	char buttonMap[]{'Z', 'X', 'W', 'A', 'S', 'D'};//, 'Q', 'E', 'C', 'V'};
	const int buttonCount = sizeof(buttonMap);
	RemoteKeyEmulationButton* subActionButtons = new RemoteKeyEmulationButton[buttonCount];
	std::vector<SubAction*> subActions;
	for (size_t i = 0; i < buttonCount; i++)
	{
		subActionButtons[i].SetKeyToPress(buttonMap[i]);
		subActions.push_back(subActionButtons+i);
	}

	Agent agent(&subActions[0], buttonCount);
	agent.LoadFromFolder(".\\Agent");
	agent.SetExplorationFactor(1.0);

	const double tickGoal = 1.0 / 30.0;
	const int actEvery = 4;
	unsigned int lookAhead = 100;
	bool hasFoundWindow = false;
	unsigned int sessionStatesFound = 0;
	unsigned int ticks = 0;
	while (true)
	{
		++ticks;
		std::clock_t start;
		start = std::clock();

		//screen grab observe

		int size = 0;
		double leadingIgnore = 0.15;
		double trailingIgnore = 0.07;
		char* image = reinterpret_cast<char*>(ScreenGrabBlt("Jnes 1.1", size, leadingIgnore, trailingIgnore));
		if (image == nullptr)
		{
			if (hasFoundWindow)
			{
				break;
			}
			std::cout << "waiting on window..." << std::endl;
			continue;
		}
		hasFoundWindow = true;

		bool newState = agent.Observe(image, size);
		if (newState)
		{
			++sessionStatesFound;
		}
		delete[] image;
		std::cout << "StatesFound:" << agent.StateCount() << "\tTicks:" << ticks << "\tLookAhead:" << lookAhead << "\tCurrentState:" << agent.CurrentState()->Id() << (newState ? "*" : "") << "\tFindRate:" << static_cast<unsigned int>(static_cast<double>(sessionStatesFound) / ticks*100) << "%";
		std::cout << std::endl;
		if (ticks % actEvery == 0)
		{
			agent.Act(lookAhead);
		}

		double tickActual = (std::clock() - start) / (double)CLOCKS_PER_SEC;
		if (tickActual < tickGoal*.95)
		{
			Sleep(static_cast<DWORD>((tickGoal - tickActual)*1000));
			if (!newState && ticks % actEvery == 0)
			{
				lookAhead = static_cast<unsigned int>(lookAhead * 1.05 + 1);
			}
		}
		else if (tickActual > tickGoal * 1.05)
		{
			lookAhead = static_cast<unsigned int>(lookAhead * .95 - 1);
			if (lookAhead > 100000 || lookAhead < 10)
			{
				lookAhead = 100;
			}
		}
	}

	return 0;
}