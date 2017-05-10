#pragma once

#include <VirtualMachine\Helper.h>
#include <VirtualMachine\Code.h>
#include <OperationSystem\Process.h>

class VirtualMachine;
class OperationSystem;

class ProcessUser : public Process
{
public:
	ProcessUser(const char* name, Process* parent, ProcessPriority priority, OperationSystem* operationSystem, VirtualMachine* virtualMachine);
	virtual ~ProcessUser();

protected:
	virtual void ResetTimerToAfterContextSwitch() { context.registerTimer = 3; }

private:
	VirtualMachine* virtualMachine;
};