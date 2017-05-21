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

	auto startStop = operationSystem->Get_startStopProcess();
	auto processManager = startStop->Get_processManager();
	auto programManager = startStop->Get_processProgramManager();

	auto pathToSource = "startup.asm";
	auto pathToAssembly = "startup.exe";
	auto name = "Startup";

	auto pathToSourceSize = strlen(pathToSource) + 1;
	auto pathToAssemblySize = strlen(pathToAssembly) + 1;
	auto nameSize = strlen(name) + 1;

	// Request ram
	ResourceRequest memoryRequest;
	auto requestMemoryCount = pathToSourceSize + pathToAssemblySize + nameSize;
	auto requestMemoryPageCount = 1;
	memoryRequest.count = requestMemoryPageCount;
	memoryRequest.requester = this;
	resourcePlanner->RequestResourceElement(operationSystem->Get_startStopProcess()->Get_resourceMemory(), memoryRequest);
	auto element = Get_ownedResourceElements()[Get_ownedResourceElements().size() - requestMemoryPageCount];
	auto address = element->indexReturn;

	uint32_t pathToSourceAddress = address;
	uint32_t pathToAssemblyAddress = pathToSourceAddress + pathToSourceSize;
	uint32_t nameAddress = pathToSourceAddress + pathToAssemblySize;

	auto ram = core->Get_ram();
	ram->WriteFromRealMemory(core, (void*) pathToSource, pathToSourceAddress, pathToSourceSize);
	ram->WriteFromRealMemory(core, (void*) pathToAssembly, pathToAssemblyAddress, pathToAssemblySize);
	ram->WriteFromRealMemory(core, (void*) name, nameAddress, nameSize);

	// Compile source
	programManager->CreateProgramFromSource(core, pathToSourceAddress);
	auto error = GetRequestedResourceElementError();
	assert(error == kResourceRespondSuccess);

	// Start process
	auto programHanlde = GetRequestedResourceElementReturn();
	programManager->SaveProgramToFile(core, pathToAssemblyAddress, programHanlde);
	error = GetRequestedResourceElementError();
	assert(error == kResourceRespondSuccess);
	processManager->CreateProcessUser(core, pathToAssemblyAddress, nameAddress);
	error = GetRequestedResourceElementError();
	assert(error == kResourceRespondSuccess);

	// Release requested memory
	resourcePlanner->ProvideResourceElement(element, this);
}