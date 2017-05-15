#pragma once

#include <VirtualMachine\Helper.h>
#include <OperationSystem\Processes\ProcessSystem.h>

class OperationSystem;
class Resource;
class ProcessExternalMemory;
class ProcessManager;
class ProcessIdle;
class ProcessProgram;
class ProcessInput;
class ProcessOutput;

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
	AUTOMATED_PROPERTY_GET(ProcessInput*, processInput);
	AUTOMATED_PROPERTY_GET(ProcessOutput*, processOutput);

	AUTOMATED_PROPERTY_GET(Resource*, resourceOSStopRequest);
	AUTOMATED_PROPERTY_GET(Resource*, resourceMemory);
	AUTOMATED_PROPERTY_GET(Resource*, resourceProcessManagerRequest);
	AUTOMATED_PROPERTY_GET(Resource*, resourceProcessManagerRespond);
	AUTOMATED_PROPERTY_GET(Resource*, resourceExternalMemoryRequest);
	AUTOMATED_PROPERTY_GET(Resource*, resourceExternalMemoryRespond);
	AUTOMATED_PROPERTY_GET(Resource*, resourceProgramManagerRequest);
	AUTOMATED_PROPERTY_GET(Resource*, resourceProgramManagerRespond);
	AUTOMATED_PROPERTY_GET(Resource*, resourceOutputRequest);
	AUTOMATED_PROPERTY_GET(Resource*, resourceOutputRespond);
	AUTOMATED_PROPERTY_GET(Resource*, resourceInputRequest);
	AUTOMATED_PROPERTY_GET(Resource*, resourceInputRespond);
};