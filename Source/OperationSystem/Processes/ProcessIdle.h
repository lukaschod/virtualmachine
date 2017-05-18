#pragma once

#include <VirtualMachine\Helper.h>
#include <OperationSystem\Processes\ProcessSystem.h>

class OperationSystem;
class Resource;
class ProcessStartStop;

class ProcessIdle : public ProcessSystem
{
public:
	ProcessIdle(ProcessStartStop* parent, OperationSystem* operationSystem);

protected:
	virtual void Execute(CentralProcessingUnitCore* core);

private:
	bool isStartProcessCreated;
};