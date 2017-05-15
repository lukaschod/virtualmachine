#pragma once

#include <VirtualMachine\Helper.h>
#include <vector>

class OperationSystem;
class Process;
class ResourcePlanner;
enum ProcessState;
enum ProcessPriority;
class CentralProcessingUnitCore;

class ProcessStartStop;
class ProcessIdle;
class ProcessExternalMemory;
class ProcessManager;
class ProcessUser;
class ProcessProgram;
class ProcessInput;
class ProcessOutput;
class VirtualMachine;

class ProcessPlanner
{
public:
	ProcessPlanner(OperationSystem* operationSystem);
	~ProcessPlanner();

	ProcessStartStop* CreateProcessStartStop();
	ProcessIdle* CreateProcessIdle(ProcessStartStop* parent);
	ProcessExternalMemory* CreateProcessExternalMemory(ProcessStartStop* parent);
	ProcessManager* CreateProcessManager(ProcessStartStop* parent);
	ProcessProgram* CreateProcessProgram(ProcessStartStop* parent);
	ProcessInput* CreateProcessInput(ProcessStartStop* parent);
	ProcessOutput* CreateProcessOutput(ProcessStartStop* parent);
	ProcessUser* CreateProcessUser(ProcessManager* parent, const char* name, ProcessPriority priority, VirtualMachine* virtualMachine);

	void KillProcess(Process* process);
	void BlockProcess(Process* process);
	void UnblockProcess(Process* process);
	void SwitchContext(CentralProcessingUnitCore* core);

private:
	void ReadyProcess(Process* process, CentralProcessingUnitCore* core);
	void RunningProcess(Process* process, CentralProcessingUnitCore* core);
	void AddProcess(Process* process, Process* parent);
	void KillProcessRecursive(Process* process);
	void ReleaseProcessAllElementsRecursive(Process* process);

private:
	AUTOMATED_PROPERTY_GET(OperationSystem*, operationSystem);
	AUTOMATED_PROPERTY_GETSET(ResourcePlanner*, resourcePlanner);
	std::vector<Process*> runningProcesses;
	std::vector<Process*> readyProcesses;
	std::vector<Process*> processes;
};