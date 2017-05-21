#include <OperationSystem\Processes\ProcessExternalMemory.h>
#include <OperationSystem\OperationSystem.h>
#include <OperationSystem\Resource.h>
#include <OperationSystem\Processes\ProcessStartStop.h>
#include <OperationSystem\ResourcePlanner.h>
#include <VirtualMachine\CentralProcessingUnit.h>
#include <VirtualMachine\ExternalMemory.h>

ProcessExternalMemory::ProcessExternalMemory(ProcessStartStop* parent, OperationSystem* operationSystem) :
	ProcessSystem("ProcessExternalMemory", parent, kProcessPriorityHigh, operationSystem) { }

void ProcessExternalMemory::Execute(CentralProcessingUnitCore* core)
{
	resourcePlanner->RequestResourceElementAny(operationSystem->Get_startStopProcess()->Get_resourceExternalMemoryRequest(), this);

	auto request = GetRequestedResourceElement();
	auto mode = request->indexMode;
	auto sender = request->sender;

	core->Get_context()->registerPS = sender->Get_context()->registerPS;

	switch (mode)
	{
	case 0:
	{
		auto filePathAddress = request->index2;
		auto accessFlag = (FileAccessFlag) request->index3;
		auto fileHandle = core->Get_externalMemory()->Open(core, filePathAddress, accessFlag);

		if (fileHandle == 0)
		{
			resourcePlanner->ProvideResourceElementAsResponse(operationSystem->Get_startStopProcess()->Get_resourceExternalMemoryRespond(),
				this, sender, kResourceRespondError, 0);
			return;
		}

		resourcePlanner->ProvideResourceElementAsResponse(operationSystem->Get_startStopProcess()->Get_resourceExternalMemoryRespond(),
			this, sender, kResourceRespondSuccess, 0, fileHandle);

		return;
	}

	case 1:
	{
		auto fileHandle = request->index2;
		core->Get_externalMemory()->Close(core, fileHandle);

		resourcePlanner->ProvideResourceElementAsResponse(operationSystem->Get_startStopProcess()->Get_resourceExternalMemoryRespond(),
			this, sender, kResourceRespondSuccess, 1);

		break;
	}

	case 2:
	{
		auto filePathAddress = request->index2;
		auto address = request->index3;
		auto size = request->index4;
		auto readSize = core->Get_externalMemory()->Read(core, filePathAddress, address, size);

		resourcePlanner->ProvideResourceElementAsResponse(operationSystem->Get_startStopProcess()->Get_resourceExternalMemoryRespond(),
			this, sender, kResourceRespondSuccess, 1, readSize);

		break;
	}

	case 3:
	{
		auto filePathAddress = request->index2;
		auto address = request->index3;
		auto size = request->index4;
		auto writeSize = core->Get_externalMemory()->Write(core, filePathAddress, address, size);

		resourcePlanner->ProvideResourceElementAsResponse(operationSystem->Get_startStopProcess()->Get_resourceExternalMemoryRespond(),
			this, sender, kResourceRespondSuccess, 1, writeSize);

		break;
	}

	}

	resourcePlanner->DestroyResourceElement(request, this);
}

void ProcessExternalMemory::OpenFile(CentralProcessingUnit* core, uint32_t filePathAddress, FileAccessFlag accessFlag)
{
	auto process = (Process*)core->Get_process();

	auto request = new ResourceElement(operationSystem->Get_startStopProcess()->Get_resourceExternalMemoryRequest(), process);
	request->indexMode = 0;
	request->index2 = filePathAddress;
	request->index3 = accessFlag;
	resourcePlanner->ProvideResourceElement(request, process);

	auto wait = ResourceRequest();
	wait.requestIndex = process->Get_fid();
	wait.requester = process;
	resourcePlanner->RequestResourceElement(operationSystem->Get_startStopProcess()->Get_resourceExternalMemoryRespond(), wait);
}

void ProcessExternalMemory::CloseFile(CentralProcessingUnit* core, uint32_t fileHandle)
{
	auto process = (Process*) core->Get_process();

	auto request = new ResourceElement(operationSystem->Get_startStopProcess()->Get_resourceExternalMemoryRequest(), process);
	request->indexMode = 1;
	request->index2 = fileHandle;
	resourcePlanner->ProvideResourceElement(request, process);

	auto wait = ResourceRequest();
	wait.requestIndex = process->Get_fid();
	wait.requester = process;
	resourcePlanner->RequestResourceElement(operationSystem->Get_startStopProcess()->Get_resourceExternalMemoryRespond(), wait);
}

void ProcessExternalMemory::ReadFile(CentralProcessingUnit* core, uint32_t fileHandle, uint32_t address, size_t size)
{
	auto process = (Process*) core->Get_process();

	auto requestRead = new ResourceElement(operationSystem->Get_startStopProcess()->Get_resourceExternalMemoryRequest(), process);
	requestRead->indexMode = 2;
	requestRead->index2 = fileHandle;
	requestRead->index3 = address;
	requestRead->index4 = size;
	resourcePlanner->ProvideResourceElement(requestRead, process);

	auto wait = ResourceRequest();
	wait.requestIndex = process->Get_fid();
	wait.requester = process;
	resourcePlanner->RequestResourceElement(operationSystem->Get_startStopProcess()->Get_resourceExternalMemoryRespond(), wait);
}

void ProcessExternalMemory::WriteFile(CentralProcessingUnit* core, uint32_t fileHandle, uint32_t address, size_t size)
{
	auto process = (Process*) core->Get_process();

	auto requestRead = new ResourceElement(operationSystem->Get_startStopProcess()->Get_resourceExternalMemoryRequest(), process);
	requestRead->indexMode = 3;
	requestRead->index2 = fileHandle;
	requestRead->index3 = address;
	requestRead->index4 = size;
	resourcePlanner->ProvideResourceElement(requestRead, process);

	auto wait = ResourceRequest();
	wait.requestIndex = process->Get_fid();
	wait.requester = process;
	resourcePlanner->RequestResourceElement(operationSystem->Get_startStopProcess()->Get_resourceExternalMemoryRespond(), wait);
}