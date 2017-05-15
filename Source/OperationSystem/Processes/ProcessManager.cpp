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

	ExecuteWhenRunning([this](CentralProcessingUnitCore* core)
	{
		auto request = GetRequestedResourceElement();
		auto mode = request->indexMode;
		auto sender = request->sender;

		core->Get_context()->registerPS = sender->Get_context()->registerPS;

		switch (mode)
		{
		case 0:
		{
			startStop->Get_processProgramManager()->LoadProgramFromFile(core, request->index2, [this, sender](CentralProcessingUnitCore* core)
			{
				auto error = GetRequestedResourceElementError();
				if (error == kResourceRespondError)
				{
					resourcePlanner->ProvideResourceElementAsResponse(operationSystem->Get_startStopProcess()->Get_resourceProcessManagerRespond(),
						this, sender, kResourceRespondError, 0);
					return;
				}

				auto program = startStop->Get_processProgramManager()->HandleToProgram(GetRequestedResourceElementReturn());

				ResourceRequest memoryRequest;
				auto requestMemoryCount = sizeof(VirtualMachineHeader) + sizeof(PageTable) + sizeof(PageEntry) * operationSystem->Get_pageCount();
				auto requestMemoryPageCount = DIVIDE_WITH_FRACTION_ADDED(requestMemoryCount, operationSystem->Get_pageSize());
				memoryRequest.count = requestMemoryPageCount;
				memoryRequest.requester = this;
				resourcePlanner->RequestResourceElement(operationSystem->Get_startStopProcess()->Get_resourceMemory(), memoryRequest);

				ExecuteWhenRunning([this, sender, requestMemoryPageCount, program](CentralProcessingUnitCore* core)
				{
					auto element = Get_ownedResourceElements()[Get_ownedResourceElements().size() - requestMemoryPageCount];

					auto virtalMachine = new VirtualMachine(program);
					virtalMachine->WriteHeaderAndPageTable(core, element->indexReturn);

					ExecuteWhenRunning([this, sender, virtalMachine](CentralProcessingUnitCore* core)
					{
						auto processUser = processPlanner->CreateProcessUser(this, "TODO ADD NAME", kProcessPriorityMedium, virtalMachine);
						resourcePlanner->ProvideResourceElementAsResponse(operationSystem->Get_startStopProcess()->Get_resourceProcessManagerRespond(),
							this, sender, kResourceRespondSuccess, 0, ProcessUserToHandle(processUser));

						core->Get_context()->registerPS = processUser->Get_context()->registerPS;

						virtalMachine->WriteDataSegment(core);
						virtalMachine->WriteCodeSegment(core);
						virtalMachine->WriteStackSegment(core);
					});
				});

			});
			return;
		}
		case 1:
		{
			auto process = HandleToProcessUser(request->index2);
			if (process == nullptr)
			{
				resourcePlanner->ProvideResourceElementAsResponse(operationSystem->Get_startStopProcess()->Get_resourceProcessManagerRespond(),
					this, sender, kResourceRespondError, 1);
				return;
			}
			processPlanner->KillProcess(process);

			resourcePlanner->ProvideResourceElementAsResponse(operationSystem->Get_startStopProcess()->Get_resourceProcessManagerRespond(),
				this, sender, kResourceRespondSuccess, 1);
			return;
		}

		}

		resourcePlanner->DestroyResourceElement(request, this);
	});
}

void ProcessManager::CreateProcessUser(CentralProcessingUnitCore* core, uint32_t pathToFileAddress, ProcessKernelInstructions callback)
{
	auto process = (Process*) core->Get_process();
	auto request = new ResourceElement(operationSystem->Get_startStopProcess()->Get_resourceProcessManagerRequest(), process);
	request->indexMode = 0;
	request->index2 = pathToFileAddress;
	resourcePlanner->ProvideResourceElement(request, process);

	auto wait = ResourceRequest();
	wait.requestIndex = process->Get_fid();
	wait.requester = process;
	resourcePlanner->RequestResourceElement(operationSystem->Get_startStopProcess()->Get_resourceProcessManagerRespond(), wait);

	if (callback != nullptr)
		process->ExecuteWhenRunning(callback);
}

void ProcessManager::DestroyProcessUser(CentralProcessingUnitCore* core, uint32_t processHandle, ProcessKernelInstructions callback)
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

	if (callback != nullptr)
		process->ExecuteWhenRunning(callback);
}