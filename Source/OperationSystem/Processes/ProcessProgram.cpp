#include <OperationSystem\Processes\ProcessProgram.h>
#include <OperationSystem\OperationSystem.h>
#include <OperationSystem\Resource.h>
#include <OperationSystem\Processes\ProcessStartStop.h>
#include <OperationSystem\Processes\ProcessExternalMemory.h>
#include <OperationSystem\Processes\ProcessUser.h>
#include <VirtualMachine\CentralProcessingUnit.h>
#include <VirtualMachine\VirtualMachine.h>
#include <VirtualMachine\ExternalMemory.h>
#include <VirtualMachine\RealMachine.h>

ProcessProgram::ProcessProgram(ProcessStartStop* parent, OperationSystem* operationSystem) :
	ProcessSystem("ProgramManager", parent, kProcessPriorityHigh, operationSystem),
	isReservedRangeValid(false),
	startStop(parent),
	reservedMemoryUsed(0)
{
}

void ProcessProgram::Execute(CentralProcessingUnitCore* core)
{
	if (!MakeSureReservedMemoryIsAllocated(core))
		return;

	resourcePlanner->RequestResourceElementAny(operationSystem->Get_startStopProcess()->Get_resourceProgramManagerRequest(), this);

	auto request = GetRequestedResourceElement();
	auto mode = request->indexMode;
	auto sender = request->sender;
	auto pathToFileAddress = request->index2;
	auto fileHandle = request->index3;

	core->Get_context()->registerPS = sender->Get_context()->registerPS;

	switch (mode)
	{
	case 0:
	{
		// Open file
		startStop->Get_processExternalMemory()->OpenFile(core, pathToFileAddress, kFileAccessReadBit);
		if (GetRequestedResourceElementError() != kResourceRespondSuccess)
		{
			resourcePlanner->ProvideResourceElementAsResponse(operationSystem->Get_startStopProcess()->Get_resourceProgramManagerRespond(),
				this, sender, kResourceRespondError, 0);
			break;
		}
		auto fileHandle = GetRequestedResourceElementReturn();

		// Read file
		// TODO: It might require loop
		auto reservedMemoryUsed = 0;
		core->Get_context()->registerPS = 0; // We don;t want to read from mmu memory here
		startStop->Get_processExternalMemory()->ReadFile(core, fileHandle, reservedMemoryRange.address, reservedMemoryRange.size);
		reservedMemoryUsed += GetRequestedResourceElementReturn();

		// Close file
		startStop->Get_processExternalMemory()->CloseFile(core, fileHandle);

		// Create program from readed file assembly
		auto program = new Program();
		if (!program->CreateFromMemory(core, reservedMemoryRange.address, reservedMemoryUsed))
		{
			assert(false);
			resourcePlanner->ProvideResourceElementAsResponse(operationSystem->Get_startStopProcess()->Get_resourceProgramManagerRespond(),
				this, sender, kResourceRespondError, 0);
			break;
		}
		resourcePlanner->ProvideResourceElementAsResponse(operationSystem->Get_startStopProcess()->Get_resourceProgramManagerRespond(),
			this, sender, kResourceRespondSuccess, 0, ProgramToHandle(program));

		break;
	}

	case 1:
	{
		// Write program assembly to memory
		auto program = HandleToProgram(request->index3);
		core->Get_context()->registerPS = 0; // We don;t want to read from mmu memory here
		if (!program->SaveToMemory(core, reservedMemoryRange.address))
		{
			assert(false);
			resourcePlanner->ProvideResourceElementAsResponse(operationSystem->Get_startStopProcess()->Get_resourceProgramManagerRespond(),
				this, sender, kResourceRespondError, 1);
			break;
		}

		// Open file
		auto reservedMemoryUsed = program->GetTotalSize();
		core->Get_context()->registerPS = sender->Get_context()->registerPS;
		startStop->Get_processExternalMemory()->OpenFile(core, pathToFileAddress, kFileAccessWriteBit);
		if (GetRequestedResourceElementError() != kResourceRespondSuccess)
		{
			resourcePlanner->ProvideResourceElementAsResponse(operationSystem->Get_startStopProcess()->Get_resourceProgramManagerRespond(),
				this, sender, kResourceRespondError, 1);
			break;
		}
		auto fileHandle = GetRequestedResourceElementReturn();

		// Write program assembly to file
		core->Get_context()->registerPS = 0; // We don;t want to read from mmu memory here
		startStop->Get_processExternalMemory()->WriteFile(core, fileHandle, reservedMemoryRange.address, reservedMemoryUsed);

		// Close file
		startStop->Get_processExternalMemory()->CloseFile(core, fileHandle);
		resourcePlanner->ProvideResourceElementAsResponse(operationSystem->Get_startStopProcess()->Get_resourceProgramManagerRespond(),
			this, sender, kResourceRespondSuccess, 1);

		break;
	}

	case 2:
	{
		// Destroy program
		auto program = HandleToProgram(fileHandle);
		if (program == nullptr)
		{
			resourcePlanner->ProvideResourceElementAsResponse(operationSystem->Get_startStopProcess()->Get_resourceProgramManagerRespond(),
				this, sender, kResourceRespondError, 2);
			break;
		}
		delete program;
		resourcePlanner->ProvideResourceElementAsResponse(operationSystem->Get_startStopProcess()->Get_resourceProgramManagerRespond(),
			this, sender, kResourceRespondSuccess, 2);

		break;
	}

	case kInteruptCreateProgramFromSource:
	{
		// Open file
		auto pathToSource = request->index2;
		startStop->Get_processExternalMemory()->OpenFile(core, pathToSource, kFileAccessReadBit);
		if (GetRequestedResourceElementError() != kResourceRespondSuccess)
		{
			resourcePlanner->ProvideResourceElementAsResponse(operationSystem->Get_startStopProcess()->Get_resourceProgramManagerRespond(),
				this, sender, kResourceRespondError, kInteruptCreateProgramFromSource);
			return;
		}
		auto fileHandle = GetRequestedResourceElementReturn();

		// Read file
		auto reservedMemoryUsed = 0;
		core->Get_context()->registerPS = 0;
		startStop->Get_processExternalMemory()->ReadFile(core, fileHandle, reservedMemoryRange.address, reservedMemoryRange.size);
		reservedMemoryUsed += GetRequestedResourceElementReturn();

		// Lets add end symbol, it will be easier for compiler to deal
		auto endOfFileSymbol = 0;
		core->Get_ram()->WriteFromRealMemory(core, &endOfFileSymbol, reservedMemoryRange.address + reservedMemoryUsed, 1);
		reservedMemoryUsed++;

		// Close file
		startStop->Get_processExternalMemory()->CloseFile(core, fileHandle);

		// Create program from readed file source and compile that source here
		auto program = new Program();
		if (!program->CompileFromMemoryAndCreate(core, reservedMemoryRange.address, reservedMemoryUsed))
		{
			assert(false);
			resourcePlanner->ProvideResourceElementAsResponse(operationSystem->Get_startStopProcess()->Get_resourceProgramManagerRespond(),
				this, sender, kResourceRespondError, kInteruptCreateProgramFromSource);
			return;
		}
		resourcePlanner->ProvideResourceElementAsResponse(operationSystem->Get_startStopProcess()->Get_resourceProgramManagerRespond(),
			this, sender, kResourceRespondSuccess, kInteruptCreateProgramFromSource, ProgramToHandle(program));

		break;
	}
	}

	resourcePlanner->DestroyResourceElement(request, this);
}


bool ProcessProgram::MakeSureReservedMemoryIsAllocated(CentralProcessingUnitCore* core)
{
	if (isReservedRangeValid)
		return true;

	ResourceRequest memoryRequest;
	auto requestMemoryPageCount = 30;
	memoryRequest.count = requestMemoryPageCount;
	memoryRequest.requester = this;
	resourcePlanner->RequestResourceElement(operationSystem->Get_startStopProcess()->Get_resourceMemory(), memoryRequest);

	auto element = Get_ownedResourceElements()[Get_ownedResourceElements().size() - requestMemoryPageCount];
	reservedMemoryRange.address = element->indexReturn;
	reservedMemoryRange.size = requestMemoryPageCount * core->Get_ram()->Get_pageSize();
	isReservedRangeValid = true;

	return isReservedRangeValid;
}

#define AUTOMATED_PROGRAM_REQUEST_WAIT(IndexMode, Index2, Index3) \
	auto process = (Process*) core->Get_process(); \
	auto request = new ResourceElement(operationSystem->Get_startStopProcess()->Get_resourceProgramManagerRequest(), process); \
	request->indexMode = IndexMode; \
	request->index2 = Index2; \
	request->index3 = Index3; \
	resourcePlanner->ProvideResourceElement(request, process); \
	\
	auto wait = ResourceRequest(); \
	wait.requestIndex = process->Get_fid(); \
	wait.requester = process; \
	resourcePlanner->RequestResourceElement(operationSystem->Get_startStopProcess()->Get_resourceProgramManagerRespond(), wait);

void ProcessProgram::LoadProgramFromFile(CentralProcessingUnitCore* core, uint32_t pathToFileAddress)
{
	AUTOMATED_PROGRAM_REQUEST_WAIT(0, pathToFileAddress, 0);
}

void ProcessProgram::SaveProgramToFile(CentralProcessingUnitCore* core, uint32_t pathToFileAddress, uint32_t programHandle)
{
	AUTOMATED_PROGRAM_REQUEST_WAIT(1, pathToFileAddress, programHandle);
}

void ProcessProgram::DestroyProgram(CentralProcessingUnitCore* core, uint32_t programHandle)
{
	AUTOMATED_PROGRAM_REQUEST_WAIT(2, programHandle, 0);
}

void ProcessProgram::CreateProgramFromSource(CentralProcessingUnitCore* core, uint32_t pathToFileAddress)
{
	AUTOMATED_PROGRAM_REQUEST_WAIT(kInteruptCreateProgramFromSource, pathToFileAddress, 0);
}