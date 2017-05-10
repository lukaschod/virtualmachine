#pragma once

#include <VirtualMachine\Helper.h>

enum ProcessPriority
{
	kProcessPriorityLow,
	kProcessPriorityMedium,
	kProcessPriorityHigh,
};

class ProcessPriorityCalculator
{
private:
	ProcessPriority priorityState;
	uint32_t priority;

public:
	ProcessPriorityCalculator(ProcessPriority priorityState) :
		priorityState(priorityState)
	{
		switch (priorityState)
		{
		case ProcessPriority::kProcessPriorityLow:
			priority = 50;
			break;
		case ProcessPriority::kProcessPriorityMedium:
			priority = 75;
			break;
		case ProcessPriority::kProcessPriorityHigh:
			priority = 100;
			break;
		}
	}

	void MakeOlder()
	{
		switch (priorityState)
		{
		case ProcessPriority::kProcessPriorityLow:
			priority--;
			break;
		case ProcessPriority::kProcessPriorityMedium:
			priority--;
			break;
		case ProcessPriority::kProcessPriorityHigh:
			priority--;
			break;
		}
		priority = CLAMP(priority, 0, 500);
	}

	void MakeYounger()
	{
		switch (priorityState)
		{
		case ProcessPriority::kProcessPriorityLow:
			priority += 5;
			break;
		case ProcessPriority::kProcessPriorityMedium:
			priority += 7;
			break;
		case ProcessPriority::kProcessPriorityHigh:
			priority += 9;
			break;
		}
		priority = CLAMP(priority, 0, 500);
	}

	uint32_t GetPriority() { return priority; }
};