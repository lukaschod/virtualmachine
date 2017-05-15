#include <OperationSystem\Processes\ProcessSystem.h>
#include <OperationSystem\Resource.h>
#include <OperationSystem\OperationSystem.h>
#include <VirtualMachine\CentralProcessingUnit.h>
#include <VirtualMachine\VirtualMachine.h>
#include <VirtualMachine\RealMachine.h>
#include <condition_variable>

ProcessSystem::ProcessSystem(const char* name, Process* parent, ProcessPriority priority, OperationSystem* operationSystem) :
	Process(name, parent, priority, operationSystem)
{
	context.registerUserMode = false;
}

void ProcessSystem::CallbackRunning(CentralProcessingUnitCore* core)
{
	// No point to queue work if its already queued
	if (queuedKernelInstructions.size() != 0)
		return;

	queuedKernelInstructions.push_back([this](CentralProcessingUnitCore* core)
	{
		Execute(core);
	});

	/*ExecuteWhenRunning([this](CentralProcessingUnitCore* core)
	{
		Execute(core);
	});*/
}