#pragma once

#include <VirtualMachine\Helper.h>
#include <VirtualMachine\Code.h>
#include <OperationSystem\Process.h>

class VirtualMachine;
class OperationSystem;

class ProcessSystem : public Process
{
public:
	ProcessSystem(const char* name, Process* parent, ProcessPriority priority, OperationSystem* operationSystem);
	virtual void CallbackRunning(CentralProcessingUnitCore* core);
	virtual void ResetTimerToAfterContextSwitch() { context.registerTimer = 0; }

protected:
	virtual void Execute(CentralProcessingUnitCore* core) = 0;
};