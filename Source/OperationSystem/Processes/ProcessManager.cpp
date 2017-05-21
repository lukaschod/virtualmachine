#include <OperationSystem\Processes\ProcessManager.h>
#include <OperationSystem\OperationSystem.h>
#include <OperationSystem\Resource.h>
#include <OperationSystem\Processes\ProcessStartStop.h>
#include <OperationSystem\Processes\ProcessExternalMemory.h>
#include <OperationSystem\Processes\ProcessUser.h>
#include <OperationSystem\Processes\ProcessProgram.h>
#include <VirtualMachine\CentralProcessingUnit.h>
#include <VirtualMachine\VirtualMachine.h>
#include <VirtualMachine\ExternalMemory.h>
#include <VirtualMachine\RealMachine.h>

ProcessManager::ProcessManager(ProcessStartStop* parent, OperationSystem* operationSystem) :
	ProcessSystem("ProcessManager", parent, kProcessPriorityHigh, operationSystem)
{

}

void ProcessManager::Execute(CentralProcessingUnitCore* core)
{
	startStop = operationSystem->Get_startStopProcess(); // Cache it
	resourcePlanner->RequestResourceElementAny(startStop->Get_resourceProcessManagerRequest(), this);

	auto request = GetRequestedResourceElement();
	auto mode = request->indexMode;
	auto sender = request->sender;

	core->Get_context()->registerPS = sender->Get_context()->registerPS;

	switch (mode)
	{
	case 0:
	{
		// Read file and create program from it
		startStop->Get_processProgramManager()->LoadProgramFromFile(core, request->index2);
		auto error = GetRequestedResourceElementError();
		if (error == kResourceRespondError)
		{
			resourcePlanner->ProvideResourceElementAsResponse(operationSystem->Get_startStopProcess()->Get_resourceProcessManagerRespond(),
				this, sender, kResourceRespondError, 0);
			break;
		}
		auto program = startStop->Get_processProgramManager()->HandleToProgram(GetRequestedResourceElementReturn());

		auto elementsBeforeCreate = Get_ownedResourceElements().size();

		// Request ram
		ResourceRequest memoryRequest;
		auto requestMemoryCount = sizeof(VirtualMachineHeader) + sizeof(PageTable) + sizeof(PageEntry) * operationSystem->Get_pageCount();
		auto requestMemoryPageCount = DIVIDE_WITH_FRACTION_ADDED(requestMemoryCount, operationSystem->Get_pageSize());
		memoryRequest.count = requestMemoryPageCount;
		memoryRequest.requester = this;
		resourcePlanner->RequestResourceElement(operationSystem->Get_startStopProcess()->Get_resourceMemory(), memoryRequest);
		auto element = Get_ownedResourceElements()[Get_ownedResourceElements().size() - requestMemoryPageCount];

		// Create virtual machine with program and ram
		auto virtalMachine = new VirtualMachine(program);
		virtalMachine->WriteHeaderAndPageTable(core, element->indexReturn);

		// Read process name
		char processName[MAX_FILEPATH_SIZE];
		auto memory = core->Get_memory();
		memory->ReadToRealMemory(core, request->index3, processName, MAX_FILEPATH_SIZE);
		processName[MAX_FILEPATH_SIZE - 1] = 0;

		// Create user process from virtual machine
		auto processUser = processPlanner->CreateProcessUser(this, processName, kProcessPriorityMedium, virtalMachine);
		core->Get_context()->registerPS = processUser->Get_context()->registerPS;

		// Allocate the segments
		virtalMachine->WriteDataSegment(core);
		virtalMachine->WriteCodeSegment(core);
		virtalMachine->WriteStackSegment(core);

		// TODO: This is not clean, but we have to move all requested ram elements to process
		while (Get_ownedResourceElements().size() != elementsBeforeCreate)
		{
			auto element = Get_ownedResourceElements().back();
			Get_ownedResourceElements().pop_back();
			processUser->Get_ownedResourceElements().push_back(element);
		}

		resourcePlanner->ProvideResourceElementAsResponse(operationSystem->Get_startStopProcess()->Get_resourceProcessManagerRespond(),
			this, sender, kResourceRespondSuccess, 0, ProcessUserToHandle(processUser));

		break;
	}
	case 1:
	{
		auto process = HandleToProcessUser(request->index2);
		if (process == nullptr)
		{
			resourcePlanner->ProvideResourceElementAsResponse(operationSystem->Get_startStopProcess()->Get_resourceProcessManagerRespond(),
				this, sender, kResourceRespondError, 1);
			break;
		}
		processPlanner->KillProcess(process);

		resourcePlanner->ProvideResourceElementAsResponse(operationSystem->Get_startStopProcess()->Get_resourceProcessManagerRespond(),
			this, sender, kResourceRespondSuccess, 1);
		break;
	}
	}

	resourcePlanner->DestroyResourceElement(request, this);
}

void ProcessManager::CreateProcessUser(CentralProcessingUnitCore* core, uint32_t pathToFileAddress, uint32_t addressToName)
{
	auto process = (Process*) core->Get_process();
	auto request = new ResourceElement(operationSystem->Get_startStopProcess()->Get_resourceProcessManagerRequest(), process);
	request->indexMode = 0;
	request->index2 = pathToFileAddress;
	request->index3 = addressToName;
	resourcePlanner->ProvideResourceElement(request, process);

	auto wait = ResourceRequest();
	wait.requestIndex = process->Get_fid();
	wait.requester = process;
	resourcePlanner->RequestResourceElement(operationSystem->Get_startStopProcess()->Get_resourceProcessManagerRespond(), wait);
}

void ProcessManager::DestroyProcessUser(CentralProcessingUnitCore* core, uint32_t processHandle)
{
	auto process = (Process*) core->Get_process();
	auto request = new ResourceElement(operationSystem->Get_startStopProcess()->Get_resourceProcessManagerRequest(), process);
	request->indexMode = 1;
	request->index2 = processHandle;
	resourcePlanner->ProvideResourceElement(request, process);

	auto wait = ResourceRequest();
	wait.requestIndex = process->Get_fid();
	wait.requester = process;
	resourcePlanner->RequestResourceElement(operationSystem->Get_startStopProcess()->Get_resourceProcessManagerRespond(), wait);
}