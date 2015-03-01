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
	agent.SetExplorationFactor(100.0);

	const double tickGoal = 1.0 / 10.0;
	unsigned int lookAhead = 100;
	while (true)
	{
		std::clock_t start;
		start = std::clock();

		//screen grab observe

		int size = 0;
		char* image = reinterpret_cast<char*>(ScreenGrabBlt("Jnes 1.1", size));
		if (image == nullptr)
		{
			std::cout << "waiting on window..." << std::endl;
			continue;
		}

		agent.Observe(image, size);
		delete[] image;
		std::cout << "StatesFound:" << agent.StateCount() << "\tSteps:" << agent.ActionsTaken() << "\tLookAhead:" << lookAhead << "\tCurrentState:" << agent.CurrentState()->Id();
		std::cout << std::endl;
		agent.Act(lookAhead);

		double tickActual = (std::clock() - start) / (double)CLOCKS_PER_SEC;
		if (tickActual < tickGoal*.95)
		{
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