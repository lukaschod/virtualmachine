#include <OperationSystem\Processes\ProcessOutput.h>
#include <OperationSystem\OperationSystem.h>
#include <OperationSystem\Resource.h>
#include <OperationSystem\Processes\ProcessStartStop.h>
#include <VirtualMachine\CentralProcessingUnit.h>
#include <VirtualMachine\Output.h>

ProcessOutput::ProcessOutput(ProcessStartStop* parent, OperationSystem* operationSystem) :
	ProcessSystem("Output", parent, kProcessPriorityHigh, operationSystem)
{
}

void ProcessOutput::Execute(CentralProcessingUnitCore* core)
{
	startStop = operationSystem->Get_startStopProcess(); // Cache it
	resourcePlanner->RequestResourceElementAny(startStop->Get_resourceOutputRequest(), this);

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
			auto address = request->index2;
			auto size = request->index3; // TODO: Add size
			auto output = core->Get_output();
			output->PrintToScreen(core, address);

			// TODO: Add error handle
			if (false)
			{
				resourcePlanner->ProvideResourceElementAsResponse(operationSystem->Get_startStopProcess()->Get_resourceOutputRespond(),
					this, sender, kResourceRespondError, 0);
				break;
			}

			resourcePlanner->ProvideResourceElementAsResponse(operationSystem->Get_startStopProcess()->Get_resourceOutputRespond(),
				this, sender, kResourceRespondSuccess, 0);
			break;
		}
		}

		resourcePlanner->DestroyResourceElement(request, this);
	});
}

void ProcessOutput::PrintLine(CentralProcessingUnitCore* core, uint32_t address, uint32_t size, ProcessKernelInstructions callback)
{
	auto process = (Process*) core->Get_process();
	auto request = new ResourceElement(operationSystem->Get_startStopProcess()->Get_resourceProcessManagerRequest(), process);
	request->indexMode = 0;
	request->index2 = address;
	request->index3 = size;
	resourcePlanner->ProvideResourceElement(request, process);

	auto wait = ResourceRequest();
	wait.requestIndex = process->Get_fid();
	wait.requester = process;
	resourcePlanner->RequestResourceElement(operationSystem->Get_startStopProcess()->Get_resourceOutputRespond(), wait);

	if (callback != nullptr)
		process->ExecuteWhenRunning(callback);
}