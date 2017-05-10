#pragma once

#include <VirtualMachine\Helper.h>
#include <VirtualMachine\CentralProcessingUnit.h>
#include <OperationSystem\ProcessPlanner.h>
#include <OperationSystem\ProcessPriorityCalculator.h>
#include <vector>
#include <functional>

class OperationSystem;
class ProcessPlanner;
class ResourcePlanner;
class Resource;
class ResourceElement;

enum ProcessState
{
	kProcessStateRunning,
	kProcessStateReady,
	kProcessStateBlocked,
	kProcessStateReadyStoped,
	kProcessStateBlocketStoped,
};

typedef std::function<void(CentralProcessingUnitCore*)> ProcessKernelInstructions;

class Process
{
public:
	Process(const char* name, Process* parent, ProcessPriority priority, OperationSystem* operationSystem);
	virtual ~Process();

	virtual void CallbackReady(CentralProcessingUnitCore* processor);
	virtual void CallbackRunning(CentralProcessingUnitCore* processor);
	virtual void CallbackBlock();
	virtual void CallbackUnblock();

	bool ExecuteCallback();
	void ExecuteWhenRunning(ProcessKernelInstructions callback);

	virtual uint32_t GetPriority() { return priorityCalculator.GetPriority(); }
	virtual void ResetTimerToAfterContextSwitch() { context.registerTimer = 0; }

	ResourceElement* GetRequestedResourceElement();
	uint32_t GetRequestedResourceElementReturn();
	uint32_t GetRequestedResourceElementError();

protected:
	AUTOMATED_PROPERTY_GET(uint32_t, fid);
	AUTOMATED_PROPERTY_GET(const char*, name);
	AUTOMATED_PROPERTY_GETSET(ProcessState, state);
	AUTOMATED_PROPERTY_GET(OperationSystem*, operationSystem);
	AUTOMATED_PROPERTY_GETSET(CentralProcessingUnitCore*, processor);
	AUTOMATED_PROPERTY_GETPTR(Registers, context);
	AUTOMATED_PROPERTY_GETADR(std::vector<ProcessKernelInstructions>, queuedKernelInstructions);
	ProcessPriorityCalculator priorityCalculator;
	ProcessPlanner* processPlanner;
	ResourcePlanner* resourcePlanner;
	AUTOMATED_PROPERTY_GET(Process*, parent);
	AUTOMATED_PROPERTY_GETADR(std::vector<Process*>, children);
	AUTOMATED_PROPERTY_GETADR(std::vector<Resource*>, createdResources);
	AUTOMATED_PROPERTY_GETADR(std::vector<ResourceElement*>, ownedResourceElements);
	
};