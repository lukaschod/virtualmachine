#pragma once

#include <VirtualMachine\Helper.h>
#include <OperationSystem\Processes\ProcessSystem.h>

class OperationSystem;
class Resource;
class ProcessStartStop;

class ProcessManager : public ProcessSystem
{
public:
	ProcessManager(ProcessStartStop* parent, OperationSystem* operationSystem);

	void CreateProcessUser(CentralProcessingUnitCore* core, uint32_t pathToFileAddress, uint32_t addressToName);
	void DestroyProcessUser(CentralProcessingUnitCore* core, uint32_t processHandle);

	ProcessUser* HandleToProcessUser(uint32_t processHandle) { return (ProcessUser*) processHandle; } // TODO: Make table lookup
	uint32_t ProcessUserToHandle(ProcessUser* process) { return (uint32_t) process; } // TODO: Make table lookup

protected:
	virtual void Execute(CentralProcessingUnitCore* core);

private:
	ProcessStartStop* startStop;
};