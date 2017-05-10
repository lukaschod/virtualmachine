#include <OperationSystem\ProcessPlanner.h>
#include <VirtualMachine\CentralProcessingUnit.h>
#include <OperationSystem\ResourcePlanner.h>
#include <OperationSystem\Resource.h>
#include <algorithm>

// Including the process types
#include <OperationSystem\Processes\ProcessStartStop.h>
#include <OperationSystem\Processes\ProcessIdle.h>
#include <OperationSystem\Processes\ProcessExternalMemory.h>
#include <OperationSystem\Processes\ProcessManager.h>
#include <OperationSystem\Processes\ProcessUser.h>
#include <OperationSystem\Processes\ProcessProgram.h>

ProcessPlanner::ProcessPlanner(OperationSystem* operationSystem) :
	operationSystem(operationSystem) {}

ProcessPlanner::~ProcessPlanner()
{
	while (processes.size() != 0)
	{
		auto process = processes.front();
		KillProcess(process);
	}
}

ProcessUser* ProcessPlanner::CreateProcessUser(ProcessManager* parent, const char* name, ProcessPriority priority, VirtualMachine* virtualMachine)
{
	auto process = new ProcessUser(name, parent, priority, operationSystem, virtualMachine);
	AddProcess(process, parent);
	return process;
}

ProcessExternalMemory* ProcessPlanner::CreateProcessExternalMemory(ProcessStartStop* parent)
{ 
	auto process = new ProcessExternalMemory(parent, operationSystem);
	AddProcess(process, parent);
	return process;
}

ProcessManager* ProcessPlanner::CreateProcessManager(ProcessStartStop* parent)
{
	auto process = new ProcessManager(parent, operationSystem);
	AddProcess(process, parent);
	return process;
}

ProcessStartStop* ProcessPlanner::CreateProcessStartStop()
{
	auto process = new ProcessStartStop(operationSystem);
	AddProcess(process, nullptr);
	return process;
}

ProcessIdle* ProcessPlanner::CreateProcessIdle(ProcessStartStop* parent)
{
	auto process = new ProcessIdle(parent, operationSystem);
	AddProcess(process, parent);
	return process;
}

ProcessProgram* ProcessPlanner::CreateProcessProgram(ProcessStartStop* parent)
{
	auto process = new ProcessProgram(parent, operationSystem);
	AddProcess(process, parent);
	return process;
}

void ProcessPlanner::AddProcess(Process* process, Process* parent)
{
	if (parent != nullptr)
		parent->Get_children().push_back(process);
	VECTOR_ADD_ITEM(processes, process);
	VECTOR_ADD_ITEM(readyProcesses, process);
}

void ProcessPlanner::KillProcess(Process* process)
{
	ReleaseProcessAllElementsRecursive(process);
	KillProcessRecursive(process);
}

void ProcessPlanner::KillProcessRecursive(Process* process)
{
	// Kill all childrens
	auto& children = process->Get_children();
	while (children.size() != 0)
	{
		KillProcessRecursive(children.back());
	}

	// TODO: Change it once the process stoping exists
	switch (process->Get_state())
	{
	case kProcessStateRunning:
		BlockProcess(process);
		break;

	case kProcessStateReady:
		VECTOR_REMOVE_ITEM(readyProcesses, process);
		break;
	}

	if (process->Get_parent() != nullptr)
		VECTOR_REMOVE_ITEM(process->Get_parent()->Get_children(), process);

	VECTOR_REMOVE_ITEM(processes, process);

	/*auto& ownedResourceElements = process->Get_ownedResourceElements();
	while (ownedResourceElements.size() != 0)
	{
	auto resourceElement = ownedResourceElements.back();
	auto resource = resourceElement->Get_resource();
	resourcePlanner->ProvideResourceElement(resourceElement, process);
	}*/

	/*auto& createdResources = process->Get_createdResources();
	while (createdResources.size() != 0)
	{
		resourcePlanner->DestroyResource(createdResources.back());
	}*/

	delete process;
}

void ProcessPlanner::ReleaseProcessAllElementsRecursive(Process* process)
{
	// Kill all childrens
	auto& children = process->Get_children();
	FOR_EACH(children, itr)
	{
		ReleaseProcessAllElementsRecursive(*itr);
	}

	auto& ownedResourceElements = process->Get_ownedResourceElements();
	while (ownedResourceElements.size() != 0)
	{
		auto resourceElement = ownedResourceElements.back();
		auto resource = resourceElement->Get_resource();
		resourcePlanner->ProvideResourceElement(resourceElement, process);
	}

	auto& createdResources = process->Get_createdResources();
	while (createdResources.size() != 0)
	{
		resourcePlanner->DestroyResource(createdResources.back());
	}
}

static bool SortProcessesByPriority(Process* first, Process* second)
{
	return second->GetPriority() < first->GetPriority();
}

void ProcessPlanner::SwitchContext(CentralProcessingUnitCore* core)
{
	// Make all the process older, except the one that was executing
	auto lastProcess = static_cast<Process*>(core->Get_process());
	if (lastProcess != nullptr && lastProcess->Get_state() == kProcessStateRunning)
		ReadyProcess(lastProcess, core);

	// This should never happen, very least there is always idle process
	if (readyProcesses.size() == 0)
	{
		core->SetInterupt(kInteruptCodeHalt);
		return;
	}

	// Re-sort processes according the changed priorities
	std::sort(readyProcesses.begin(), readyProcesses.end(), SortProcessesByPriority);

	// Set the process with highest priority
	auto process = readyProcesses.front();
	RunningProcess(process, core);
}

void ProcessPlanner::ReadyProcess(Process* process, CentralProcessingUnitCore* core)
{
	assert(process->Get_state() == ProcessState::kProcessStateRunning);
	process->Set_state(ProcessState::kProcessStateReady);

	process->Set_processor(nullptr);
	core->Set_process(nullptr);
	*process->Get_context() = *core->Get_context();

	VECTOR_REMOVE_ITEM(runningProcesses, process);
	VECTOR_ADD_ITEM(readyProcesses, process);

	process->CallbackRunning(core);
}

void ProcessPlanner::RunningProcess(Process* process, CentralProcessingUnitCore* core)
{
	printf("Loading context for process named %s\n", process->Get_name());
	assert(process->Get_state() == ProcessState::kProcessStateReady);
	process->Set_state(ProcessState::kProcessStateRunning);

	process->Set_processor(core);
	core->Set_process(process);
	process->ResetTimerToAfterContextSwitch();
	*core->Get_context() = *process->Get_context();

	VECTOR_REMOVE_ITEM(readyProcesses, process);
	VECTOR_ADD_ITEM(runningProcesses, process);

	process->CallbackReady(core);
}

void ProcessPlanner::BlockProcess(Process* process)
{
	assert(process->Get_state() == ProcessState::kProcessStateRunning);
	process->Set_state(ProcessState::kProcessStateBlocked);

	VECTOR_REMOVE_ITEM(runningProcesses, process);
	SwitchContext(process->Get_processor());

	process->CallbackBlock();
}

void ProcessPlanner::UnblockProcess(Process* process)
{
	assert(process->Get_state() == ProcessState::kProcessStateBlocked);
	process->Set_state(ProcessState::kProcessStateReady);

	VECTOR_ADD_ITEM(readyProcesses, process);

	process->CallbackUnblock();
}