#include "Agent.h"
#include "SubAction.h"
#include "RemoteKeyEmulationButton.h"
#include "State.h"

#include "WindowsScreenCapture.h"

#include <vector>
#include <stdio.h>
#include <iostream>
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
	agent.SetExplorationFactor(100.0);

	const double tickGoal = 1.0 / 30.0;
	unsigned int lookAhead = 100;
	bool hasFoundWindow = false;
	while (true)
	{
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
		delete[] image;
		std::cout << "StatesFound:" << agent.StateCount() << "\tSteps:" << agent.ActionsTaken() << "\tLookAhead:" << lookAhead << "\tCurrentState:" << agent.CurrentState()->Id() << (newState ? "*" : "");
		std::cout << std::endl;
		agent.Act(lookAhead);

		double tickActual = (std::clock() - start) / (double)CLOCKS_PER_SEC;
		if (tickActual < tickGoal*.95)
		{
			Sleep((tickGoal - tickActual)*1000);
			++lookAhead;
		}
		else if (tickActual > tickGoal * 1.05)
		{
			if (lookAhead > 10)
			{
				--lookAhead;
			}
		}
	}

	return 0;
}