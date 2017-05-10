#pragma once

#include <VirtualMachine\Helper.h>
#include <VirtualMachine\Code.h>
#include <OperationSystem\Processes\ProcessSystem.h>

class OperationSystem;
class Resource;
class ProcessStartStop;

class ProcessProgram : public ProcessSystem
{
public:
	ProcessProgram(ProcessStartStop* parent, OperationSystem* operationSystem);

	void CreateProgramFromFile(CentralProcessingUnitCore* core, uint32_t pathToFileAddress, ProcessKernelInstructions callback);
	void SaveProgramToFile(CentralProcessingUnitCore* core, uint32_t pathToFileAddress, uint32_t programHandle, ProcessKernelInstructions callback);
	void DestroyProgram(CentralProcessingUnitCore* core, uint32_t programHandle, ProcessKernelInstructions callback);

	Program* HandleToProgram(uint32_t programHandle) { return (Program*) programHandle; } // TODO: make normal table lookup
	uint32_t ProgramToHandle(Program* program) { return (uint32_t) program; } // TODO: make normal table lookup

protected:
	virtual void Execute(CentralProcessingUnitCore* core);
	bool MakeSureReservedMemoryIsAllocated();

private:
	ProcessStartStop* startStop;
	AddressRange reservedMemoryRange;
	uint32_t reservedMemoryUsed;
	bool isReservedRangeValid;
};