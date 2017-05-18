#include <OperationSystem\Processes\ProcessInput.h>
#include <OperationSystem\OperationSystem.h>
#include <OperationSystem\Resource.h>
#include <OperationSystem\Processes\ProcessStartStop.h>
#include <VirtualMachine\CentralProcessingUnit.h>
#include <VirtualMachine\Input.h>

ProcessInput::ProcessInput(ProcessStartStop* parent, OperationSystem* operationSystem) :
	ProcessSystem("Input", parent, kProcessPriorityHigh, operationSystem)
{
}

void ProcessInput::Execute(CentralProcessingUnitCore* core)
{
	startStop = operationSystem->Get_startStopProcess(); // Cache it
	resourcePlanner->RequestResourceElementAny(startStop->Get_resourceInputRequest(), this);

	auto request = GetRequestedResourceElement();
	auto mode = request->indexMode;
	auto sender = request->sender;

	core->Get_context()->registerPS = sender->Get_context()->registerPS;

	switch (mode)
	{
	case 0:
	{
		auto address = request->index2;
		auto input = core->Get_input();
		input->ReadUntilEnter(core, address);

		// TODO: Add error handle
		if (false)
		{
			resourcePlanner->ProvideResourceElementAsResponse(operationSystem->Get_startStopProcess()->Get_resourceInputRespond(),
				this, sender, kResourceRespondError, 0);
			break;
		}

		resourcePlanner->ProvideResourceElementAsResponse(operationSystem->Get_startStopProcess()->Get_resourceInputRespond(),
			this, sender, kResourceRespondSuccess, 0);
		break;
	}
	}

	resourcePlanner->DestroyResourceElement(request, this);
}

void ProcessInput::ReadLine(CentralProcessingUnitCore* core, uint32_t address)
{
	auto process = (Process*) core->Get_process();
	auto request = new ResourceElement(operationSystem->Get_startStopProcess()->Get_resourceInputRequest(), process);
	request->indexMode = 0;
	request->index2 = address;
	resourcePlanner->ProvideResourceElement(request, process);

	auto wait = ResourceRequest();
	wait.requestIndex = process->Get_fid();
	wait.requester = process;
	resourcePlanner->RequestResourceElement(operationSystem->Get_startStopProcess()->Get_resourceInputRespond(), wait);
}