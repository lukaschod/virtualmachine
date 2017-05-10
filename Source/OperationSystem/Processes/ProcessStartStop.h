#pragma once

#include <VirtualMachine\Helper.h>
#include <OperationSystem\Processes\ProcessSystem.h>

class OperationSystem;
class Resource;
class ProcessExternalMemory;
class ProcessManager;
class ProcessIdle;
class ProcessProgram;

class ProcessStartStop : public ProcessSystem
{
public:
	ProcessStartStop(OperationSystem* operationSystem);

	void StopOperationSystem(CentralProcessingUnitCore* core, ProcessKernelInstructions callback);

protected:
	virtual void Execute(CentralProcessingUnitCore* core);

private:
	AUTOMATED_PROPERTY_GET(ProcessExternalMemory*, processExternalMemory);
	AUTOMATED_PROPERTY_GET(ProcessManager*, processManager);
	AUTOMATED_PROPERTY_GET(ProcessIdle*, processIdle);
	AUTOMATED_PROPERTY_GET(ProcessProgram*, processProgramManager);

	AUTOMATED_PROPERTY_GET(Resource*, waitForStop);
	AUTOMATED_PROPERTY_GET(Resource*, memory);
	AUTOMATED_PROPERTY_GET(Resource*, resourceProcessManager);
	AUTOMATED_PROPERTY_GET(Resource*, resourceProcessManagerRespond);
	AUTOMATED_PROPERTY_GET(Resource*, resourceExternalMemoryRequest);
	AUTOMATED_PROPERTY_GET(Resource*, resourceExternalMemoryRespond);
	AUTOMATED_PROPERTY_GET(Resource*, resourceProgramManagerRequest);
	AUTOMATED_PROPERTY_GET(Resource*, resourceProgramManagerRespond);
};