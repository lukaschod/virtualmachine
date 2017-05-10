#pragma once

#include <VirtualMachine\Helper.h>
#include <OperationSystem\Processes\ProcessSystem.h>
#include <OperationSystem\Process.h>

class OperationSystem;
class Resource;
class ProcessStartStop;
class CentralProcessingUnitCore;
enum FileAccessFlag;

class ProcessExternalMemory : public ProcessSystem
{
public:
	ProcessExternalMemory(ProcessStartStop* parent, OperationSystem* operationSystem);

	void OpenFile(CentralProcessingUnit* core, uint32_t filePathAddress, FileAccessFlag accessFlag, ProcessKernelInstructions callback = nullptr);
	void CloseFile(CentralProcessingUnit* core, uint32_t fileHandle, ProcessKernelInstructions callback = nullptr);
	void ReadFile(CentralProcessingUnit* core, uint32_t fileHandle, uint32_t address, size_t size, ProcessKernelInstructions callback = nullptr);
	void WriteFile(CentralProcessingUnit* core, uint32_t fileHandle, uint32_t address, size_t size, ProcessKernelInstructions callback = nullptr);

protected:
	virtual void Execute(CentralProcessingUnitCore* core);
};