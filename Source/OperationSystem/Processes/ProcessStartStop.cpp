#include <OperationSystem\Processes\ProcessStartStop.h>
#include <OperationSystem\Processes\ProcessExternalMemory.h>
#include <OperationSystem\Processes\ProcessManager.h>
#include <OperationSystem\Processes\ProcessIdle.h>
#include <OperationSystem\OperationSystem.h>
#include <OperationSystem\Resource.h>
#include <VirtualMachine\CentralProcessingUnit.h>
#include <VirtualMachine\RealMachine.h>
#include <VirtualMachine\RandomAccessMemory.h>

ProcessStartStop::ProcessStartStop(OperationSystem* operationSystem) : 
	ProcessSystem("StartStop", nullptr, kProcessPriorityHigh, operationSystem)
{
	// Create resources:
	auto ram = operationSystem->Get_realMachine()->GetRam();
	resourceMemory = resourcePlanner->CreateResourceMemory(this, ram->Get_pageCount(), ram->Get_pageSize());
	resourceOSStopRequest = resourcePlanner->CreateResourceRequest(this, "resourceOSStopRequest");
	resourceExternalMemoryRequest = resourcePlanner->CreateResourceRequest(this, "resourceExternalMemoryRequest");
	resourceExternalMemoryRespond = resourcePlanner->CreateResourceRespond(this, "resourceExternalMemoryRespond");
	resourceProgramManagerRequest = resourcePlanner->CreateResourceRequest(this, "resourceProgramManagerRequest");
	resourceProgramManagerRespond = resourcePlanner->CreateResourceRespond(this, "resourceProgramManagerRespond");
	resourceProcessManagerRequest = resourcePlanner->CreateResourceRequest(this, "resourceProcessManagerRequest");
	resourceProcessManagerRespond = resourcePlanner->CreateResourceRespond(this, "resourceProcessManagerRespond");
	resourceOutputRequest = resourcePlanner->CreateResourceRequest(this, "resourceOutputRequest");
	resourceOutputRespond = resourcePlanner->CreateResourceRespond(this, "resourceOutputRespond");
	resourceInputRequest = resourcePlanner->CreateResourceRequest(this, "resourceInputRequest");
	resourceInputRespond = resourcePlanner->CreateResourceRespond(this, "resourceInputRespond");

	// Create processes
	processManager = processPlanner->CreateProcessManager(this);
	processExternalMemory = processPlanner->CreateProcessExternalMemory(this);
	processIdle = processPlanner->CreateProcessIdle(this);
	processProgramManager = processPlanner->CreateProcessProgram(this);
	processOutput = processPlanner->CreateProcessOutput(this);
	processInput = processPlanner->CreateProcessInput(this);
}

void ProcessStartStop::Execute(CentralProcessingUnitCore* core)
{
	resourcePlanner->RequestResourceElementAny(resourceOSStopRequest, this);

	auto request = GetRequestedResourceElement();
	resourcePlanner->DestroyResourceElement(request, this);
	operationSystem->Get_realMachine()->Stop();
	processPlanner->KillProcess(this);
}

void ProcessStartStop::StopOperationSystem(CentralProcessingUnitCore* core)
{
	auto process = (Process*) core->Get_process();
	auto response = new ResourceElement(operationSystem->Get_startStopProcess()->Get_resourceOSStopRequest(), process);
	resourcePlanner->ProvideResourceElement(response, process);
}