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

	void OpenFile(CentralProcessingUnit* core, uint32_t filePathAddress, FileAccessFlag accessFlag);
	void CloseFile(CentralProcessingUnit* core, uint32_t fileHandle);
	void ReadFile(CentralProcessingUnit* core, uint32_t fileHandle, uint32_t address, size_t size);
	void WriteFile(CentralProcessingUnit* core, uint32_t fileHandle, uint32_t address, size_t size);

protected:
	virtual void Execute(CentralProcessingUnitCore* core);
};