#pragma once

#include <VirtualMachine\Helper.h>
#include <OperationSystem\Processes\ProcessSystem.h>

class OperationSystem;
class Resource;
class ProcessStartStop;

class ProcessInput : public ProcessSystem
{
public:
	ProcessInput(ProcessStartStop* parent, OperationSystem* operationSystem);

	void ReadLine(CentralProcessingUnitCore* core, uint32_t address, ProcessKernelInstructions callback = nullptr);

protected:
	virtual void Execute(CentralProcessingUnitCore* core);

private:
	ProcessStartStop* startStop;
};