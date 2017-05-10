#pragma once

#include <VirtualMachine\Helper.h>
#include <stdint.h>
#include <string>
#include <vector>

class CentralProcessingUnit;
struct Registers;

class IMemory
{
public:
	virtual void WriteFromRealMemory(CentralProcessingUnit* core, void* srcPointer, uint32_t dstAddress, size_t size = sizeof(uint32_t)) = 0;
	virtual void ReadToRealMemory(CentralProcessingUnit* core, uint32_t srcAddress, void* dstPointer, size_t size = sizeof(uint32_t)) = 0;
	virtual void Write(CentralProcessingUnit* core, uint32_t srcAddress, uint32_t dstAddress, size_t size = sizeof(uint32_t)) = 0;
	virtual void Read(CentralProcessingUnit* core, uint32_t srcAddress, uint32_t dstAddress, size_t size = sizeof(uint32_t)) = 0;
	virtual uint32_t AllocateMemory(CentralProcessingUnit* core, size_t size = sizeof(uint32_t)) = 0;
	virtual PointerRange AddressToPointerRange(CentralProcessingUnit* core, uint32_t address, size_t size = sizeof(uint32_t)) = 0;
};
