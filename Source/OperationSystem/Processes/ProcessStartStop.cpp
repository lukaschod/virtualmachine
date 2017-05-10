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
	// Create resources
	waitForStop = resourcePlanner->CreateResourceOSStop(this);
	resourceProcessManager = resourcePlanner->CreateResourceProcessManagerWait(this);
	resourceProcessManagerRespond = resourcePlanner->CreateResourceProcessManagerWait(this);
	auto ram = operationSystem->Get_realMachine()->GetRam();
	memory = resourcePlanner->CreateResourceMemory(this, ram->Get_pageCount(), ram->Get_pageSize());
	resourceExternalMemoryRequest = resourcePlanner->CreateResourceExternalMemoryWait(this);
	resourceExternalMemoryRespond = resourcePlanner->CreateResourceExternalMemoryRespond(this);
	resourceProgramManagerRequest = resourcePlanner->CreateResourceExternalMemoryWait(this);
	resourceProgramManagerRespond = resourcePlanner->CreateResourceExternalMemoryRespond(this);

	// Create processes
	processManager = processPlanner->CreateProcessManager(this);
	processExternalMemory = processPlanner->CreateProcessExternalMemory(this);
	processIdle = processPlanner->CreateProcessIdle(this);
	processProgramManager = processPlanner->CreateProcessProgram(this);
}

void ProcessStartStop::Execute(CentralProcessingUnitCore* core)
{
	resourcePlanner->RequestResourceElementAny(waitForStop, this);

	ExecuteWhenRunning([this](CentralProcessingUnit* core)
	{
		auto request = GetRequestedResourceElement();
		resourcePlanner->DestroyResourceElement(request, this);
		
		processPlanner->KillProcess(this);
	});
}

void ProcessStartStop::StopOperationSystem(CentralProcessingUnitCore* core, ProcessKernelInstructions callback)
{
	auto process = (Process*) core->Get_process();
	auto response = new ResourceElement(operationSystem->Get_startStopProcess()->Get_waitForStop(), process);
	resourcePlanner->ProvideResourceElement(response, process);

	if (callback != nullptr)
		process->ExecuteWhenRunning(callback);
}