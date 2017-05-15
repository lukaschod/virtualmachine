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
	if (!MakeSureReservedMemoryIsAllocated())
		return;

	resourcePlanner->RequestResourceElementAny(operationSystem->Get_startStopProcess()->Get_resourceProgramManagerRequest(), this);

	ExecuteWhenRunning([this](CentralProcessingUnitCore* core)
	{
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
			startStop->Get_processExternalMemory()->OpenFile(core, pathToFileAddress, kFileAccessReadBit);

			ExecuteWhenRunning([this, sender](CentralProcessingUnitCore* core)
			{
                if (GetRequestedResourceElementError() != kResourceRespondSuccess)
                {
                    resourcePlanner->ProvideResourceElementAsResponse(operationSystem->Get_startStopProcess()->Get_resourceProgramManagerRespond(),
                        this, sender, kResourceRespondError, 0);
                    return;
                }

				auto fileHandle = GetRequestedResourceElementReturn();

				reservedMemoryUsed = 0;
				core->Get_context()->registerPS = 0;
				startStop->Get_processExternalMemory()->ReadFile(core, fileHandle, reservedMemoryRange.address, reservedMemoryRange.size);
				ExecuteWhenRunning([this, sender, fileHandle](CentralProcessingUnitCore* core)
				{
					reservedMemoryUsed += GetRequestedResourceElementReturn();

					startStop->Get_processExternalMemory()->CloseFile(core, fileHandle);
					ExecuteWhenRunning([this, sender](CentralProcessingUnitCore* core)
					{
						auto program = new Program();
						if (!program->CreateFromMemory(core, reservedMemoryRange.address, reservedMemoryUsed))
						{
							assert(false);
							resourcePlanner->ProvideResourceElementAsResponse(operationSystem->Get_startStopProcess()->Get_resourceProgramManagerRespond(),
								this, sender, kResourceRespondError, 0);
							return;
						}

						resourcePlanner->ProvideResourceElementAsResponse(operationSystem->Get_startStopProcess()->Get_resourceProgramManagerRespond(),
							this, sender, kResourceRespondSuccess, 0, ProgramToHandle(program));
					});
				});
			});
			break;
		}

		case 1:
		{
			auto program = HandleToProgram(fileHandle);
			if (!program->SaveToMemory(core, reservedMemoryRange.address))
			{
				assert(false);
				resourcePlanner->ProvideResourceElementAsResponse(operationSystem->Get_startStopProcess()->Get_resourceProgramManagerRespond(),
					this, sender, kResourceRespondError, 1);
				break;
			}

			reservedMemoryUsed = program->GetTotalSize();
			startStop->Get_processExternalMemory()->OpenFile(core, pathToFileAddress, kFileAccessWriteBit);

			ExecuteWhenRunning([this, sender](CentralProcessingUnitCore* core)
			{
                if (GetRequestedResourceElementError() != kResourceRespondSuccess)
                {
                    resourcePlanner->ProvideResourceElementAsResponse(operationSystem->Get_startStopProcess()->Get_resourceProgramManagerRespond(),
                        this, sender, kResourceRespondError, 1);
                    return;
                }

				auto fileHandle = GetRequestedResourceElementReturn();

				startStop->Get_processExternalMemory()->WriteFile(core, fileHandle, reservedMemoryRange.address, reservedMemoryUsed);
				ExecuteWhenRunning([this, sender, fileHandle](CentralProcessingUnitCore* core)
				{
					startStop->Get_processExternalMemory()->CloseFile(core, fileHandle);
					ExecuteWhenRunning([this, sender](CentralProcessingUnitCore* core)
					{
						resourcePlanner->ProvideResourceElementAsResponse(operationSystem->Get_startStopProcess()->Get_resourceProgramManagerRespond(),
							this, sender, kResourceRespondSuccess, 1);
					});
				});
			});
			break;
		}

		case 2:
		{
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

		case 3:
		{
			auto pathToSource = request->index2;
			auto pathToOutput = request->index3;
			startStop->Get_processExternalMemory()->OpenFile(core, pathToSource, kFileAccessReadBit);

			ExecuteWhenRunning([this, sender](CentralProcessingUnitCore* core)
			{
				if (GetRequestedResourceElementError() != kResourceRespondSuccess)
				{
					resourcePlanner->ProvideResourceElementAsResponse(operationSystem->Get_startStopProcess()->Get_resourceProgramManagerRespond(),
						this, sender, kResourceRespondError, 3);
					return;
				}

				auto fileHandle = GetRequestedResourceElementReturn();

				reservedMemoryUsed = 0;
				core->Get_context()->registerPS = 0;
				startStop->Get_processExternalMemory()->ReadFile(core, fileHandle, reservedMemoryRange.address, reservedMemoryRange.size);
				ExecuteWhenRunning([this, sender, fileHandle](CentralProcessingUnitCore* core)
				{
					reservedMemoryUsed += GetRequestedResourceElementReturn();

					startStop->Get_processExternalMemory()->CloseFile(core, fileHandle);
					ExecuteWhenRunning([this, sender](CentralProcessingUnitCore* core)
					{
						auto program = new Program();
						if (!program->CompileFromMemoryAndCreate(core, reservedMemoryRange.address, reservedMemoryUsed))
						{
							assert(false);
							resourcePlanner->ProvideResourceElementAsResponse(operationSystem->Get_startStopProcess()->Get_resourceProgramManagerRespond(),
								this, sender, kResourceRespondError, 3);
							return;
						}

						resourcePlanner->ProvideResourceElementAsResponse(operationSystem->Get_startStopProcess()->Get_resourceProgramManagerRespond(),
							this, sender, kResourceRespondSuccess, 3, ProgramToHandle(program));
					});
				});
			});
			break;
		}

		}

		resourcePlanner->DestroyResourceElement(request, this);
	});
}


bool ProcessProgram::MakeSureReservedMemoryIsAllocated()
{
	if (isReservedRangeValid)
		return true;

	ResourceRequest memoryRequest;
	auto requestMemoryPageCount = 20;
	memoryRequest.count = requestMemoryPageCount;
	memoryRequest.requester = this;
	resourcePlanner->RequestResourceElement(operationSystem->Get_startStopProcess()->Get_resourceMemory(), memoryRequest);

	ExecuteWhenRunning([this, requestMemoryPageCount](CentralProcessingUnitCore* core)
	{
		auto element = Get_ownedResourceElements()[Get_ownedResourceElements().size() - requestMemoryPageCount];
		reservedMemoryRange.address = element->indexReturn;
		reservedMemoryRange.size = requestMemoryPageCount * core->Get_ram()->Get_pageSize();
		isReservedRangeValid = true;
	});
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
	resourcePlanner->RequestResourceElement(operationSystem->Get_startStopProcess()->Get_resourceProgramManagerRespond(), wait); \
	\
	if (callback != nullptr) \
		process->ExecuteWhenRunning(callback);

void ProcessProgram::LoadProgramFromFile(CentralProcessingUnitCore* core, uint32_t pathToFileAddress, ProcessKernelInstructions callback)
{
	AUTOMATED_PROGRAM_REQUEST_WAIT(0, pathToFileAddress, 0);
}

void ProcessProgram::SaveProgramToFile(CentralProcessingUnitCore* core, uint32_t pathToFileAddress, uint32_t programHandle, ProcessKernelInstructions callback)
{
	AUTOMATED_PROGRAM_REQUEST_WAIT(1, pathToFileAddress, programHandle);
}

void ProcessProgram::DestroyProgram(CentralProcessingUnitCore* core, uint32_t programHandle, ProcessKernelInstructions callback)
{
	AUTOMATED_PROGRAM_REQUEST_WAIT(2, programHandle, 0);
}

void ProcessProgram::CreateProgramFromSource(CentralProcessingUnitCore* core, uint32_t pathToFileAddress, ProcessKernelInstructions callback)
{
	AUTOMATED_PROGRAM_REQUEST_WAIT(3, pathToFileAddress, 0);
}