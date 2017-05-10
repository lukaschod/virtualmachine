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
		auto mode = request->index;
		auto sender = request->sender;
		auto pathToFileAddress = request->index2;
		auto fileHandle = request->index3;

		resourcePlanner->DestroyResourceElement(request, this);

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
				startStop->Get_processExternalMemory()->ReadFile(core, fileHandle, reservedMemoryRange.address, reservedMemoryRange.size);
				ExecuteWhenRunning([this, sender, fileHandle](CentralProcessingUnitCore* core)
				{
					auto size = GetRequestedResourceElementReturn();
					size += reservedMemoryUsed;

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
			return;
		}

		case 3:
		{
			auto program = HandleToProgram(fileHandle);
			if (!program->SaveToMemory(core, reservedMemoryRange.address))
			{
				assert(false);
				resourcePlanner->ProvideResourceElementAsResponse(operationSystem->Get_startStopProcess()->Get_resourceProgramManagerRespond(),
					this, sender, kResourceRespondError, 3);
				return;
			}

			reservedMemoryUsed = program->GetTotalSize();
			startStop->Get_processExternalMemory()->OpenFile(core, pathToFileAddress, kFileAccessWriteBit);

			ExecuteWhenRunning([this, sender](CentralProcessingUnitCore* core)
			{
                if (GetRequestedResourceElementError() != kResourceRespondSuccess)
                {
                    resourcePlanner->ProvideResourceElementAsResponse(operationSystem->Get_startStopProcess()->Get_resourceProgramManagerRespond(),
                        this, sender, kResourceRespondError, 0);
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
							this, sender, kResourceRespondSuccess, 3);
					});
				});
			});
			return;
		}

		case 1:
		{
			auto program = HandleToProgram(fileHandle);
			if (program == nullptr)
			{
				resourcePlanner->ProvideResourceElementAsResponse(operationSystem->Get_startStopProcess()->Get_resourceProgramManagerRespond(),
					this, sender, kResourceRespondError, 1);
				return;
			}

			delete program;
			resourcePlanner->ProvideResourceElementAsResponse(operationSystem->Get_startStopProcess()->Get_resourceProgramManagerRespond(),
				this, sender, kResourceRespondSuccess, 1);
			return;
		}

		}
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
	resourcePlanner->RequestResourceElement(operationSystem->Get_startStopProcess()->Get_memory(), memoryRequest);

	ExecuteWhenRunning([this, requestMemoryPageCount](CentralProcessingUnitCore* core)
	{
		auto element = Get_ownedResourceElements()[Get_ownedResourceElements().size() - requestMemoryPageCount];
		reservedMemoryRange.address = element->indexReturn;
		reservedMemoryRange.size = 20 * core->Get_ram()->Get_pageSize();
		isReservedRangeValid = true;
	});
	return isReservedRangeValid;
}

void ProcessProgram::CreateProgramFromFile(CentralProcessingUnitCore* core, uint32_t pathToFileAddress, ProcessKernelInstructions callback)
{
	auto process = (Process*) core->Get_process();
	auto request = new ResourceElement(operationSystem->Get_startStopProcess()->Get_resourceProgramManagerRequest(), process);
	request->index = 0;
	request->index2 = pathToFileAddress;
	resourcePlanner->ProvideResourceElement(request, process);

	auto wait = ResourceRequest();
	wait.requestIndex = process->Get_fid();
	wait.requester = process;
	resourcePlanner->RequestResourceElement(operationSystem->Get_startStopProcess()->Get_resourceProgramManagerRespond(), wait);

	if (callback != nullptr)
		process->ExecuteWhenRunning(callback);
}

void ProcessProgram::SaveProgramToFile(CentralProcessingUnitCore* core, uint32_t pathToFileAddress, uint32_t programHandle, ProcessKernelInstructions callback)
{
	auto process = (Process*) core->Get_process();
	auto request = new ResourceElement(operationSystem->Get_startStopProcess()->Get_resourceProgramManagerRequest(), process);
	request->index = 3;
	request->index2 = pathToFileAddress;
	request->index3 = programHandle;
	resourcePlanner->ProvideResourceElement(request, process);

	auto wait = ResourceRequest();
	wait.requestIndex = process->Get_fid();
	wait.requester = process;
	resourcePlanner->RequestResourceElement(operationSystem->Get_startStopProcess()->Get_resourceProgramManagerRespond(), wait);

	if (callback != nullptr)
		process->ExecuteWhenRunning(callback);
}

void ProcessProgram::DestroyProgram(CentralProcessingUnitCore* core, uint32_t programHandle, ProcessKernelInstructions callback)
{
	auto process = (Process*) core->Get_process();
	auto request = new ResourceElement(operationSystem->Get_startStopProcess()->Get_resourceProgramManagerRequest(), process);
	request->index = 2;
	request->index2 = programHandle;
	resourcePlanner->ProvideResourceElement(request, process);

	auto wait = ResourceRequest();
	wait.requestIndex = process->Get_fid();
	wait.requester = process;
	resourcePlanner->RequestResourceElement(operationSystem->Get_startStopProcess()->Get_resourceProgramManagerRespond(), wait);

	if (callback != nullptr)
		process->ExecuteWhenRunning(callback);
}