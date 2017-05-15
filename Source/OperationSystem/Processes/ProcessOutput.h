#pragma once

#include <VirtualMachine\Helper.h>
#include <OperationSystem\Processes\ProcessSystem.h>

class OperationSystem;
class Resource;
class ProcessStartStop;

class ProcessOutput : public ProcessSystem
{
public:
	ProcessOutput(ProcessStartStop* parent, OperationSystem* operationSystem);

	void PrintLine(CentralProcessingUnitCore* core, uint32_t address, uint32_t size, ProcessKernelInstructions callback = nullptr);

protected:
	virtual void Execute(CentralProcessingUnitCore* core);

private:
	ProcessStartStop* startStop;
};