#include <OperationSystem\Processes\ProcessIdle.h>
#include <OperationSystem\OperationSystem.h>
#include <OperationSystem\Resource.h>
#include <OperationSystem\Processes\ProcessStartStop.h>
#include <OperationSystem\Processes\ProcessManager.h>
#include <OperationSystem\Processes\ProcessProgram.h>
#include <VirtualMachine\CentralProcessingUnit.h>
#include <VirtualMachine\ExternalMemory.h>

ProcessIdle::ProcessIdle(ProcessStartStop* parent, OperationSystem* operationSystem) :
	ProcessSystem("ProcessIdle", parent, kProcessPriorityLow, operationSystem) 
{ 
	isStartProcessCreated = false;
}

void ProcessIdle::Execute(CentralProcessingUnitCore* core)
{
	if (isStartProcessCreated)
		return;
	isStartProcessCreated = true;

	uint32_t pathToFileSourceAddress;
	uint32_t pathToFileAssemblyAddress;

	auto startStop = operationSystem->Get_startStopProcess();
	auto processManager = startStop->Get_processManager();
	auto programManager = startStop->Get_processProgramManager();

	// Request ram for storing temporary information
	ResourceRequest memoryRequest;
	memoryRequest.count = 1;
	memoryRequest.requester = this;
	resourcePlanner->RequestResourceElement(operationSystem->Get_startStopProcess()->Get_resourceMemory(), memoryRequest);

	// Write path from where startup process is compiled
	auto element = GetRequestedResourceElement();
	pathToFileSourceAddress = GetRequestedResourceElementReturn();
	const char* pathToSource = "startup.asm";
	auto size = strlen(pathToSource) + 1;
	core->Get_ram()->WriteFromRealMemory(core, (void*) pathToSource, pathToFileSourceAddress, size);

	// Write path where startup proesses source is converted into executable
	pathToFileAssemblyAddress = pathToFileSourceAddress + size;
	const char* pathToAssembly = "startup.exe";
	core->Get_ram()->WriteFromRealMemory(core, (void*) pathToAssembly, pathToFileAssemblyAddress, size);

	// Compile source
	programManager->CreateProgramFromSource(core, pathToFileSourceAddress);
	auto error = GetRequestedResourceElementError();
	assert(error == kResourceRespondSuccess);

	// Start process
	auto programHanlde = GetRequestedResourceElementReturn();
	programManager->SaveProgramToFile(core, pathToFileAssemblyAddress, programHanlde);
	error = GetRequestedResourceElementError();
	assert(error == kResourceRespondSuccess);
	processManager->CreateProcessUser(core, pathToFileAssemblyAddress);
	error = GetRequestedResourceElementError();
	assert(error == kResourceRespondSuccess);

	// Release requested memory
	resourcePlanner->ProvideResourceElement(element, this);
}