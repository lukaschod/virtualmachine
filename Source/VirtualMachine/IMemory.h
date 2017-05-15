#pragma once

#include <VirtualMachine\Helper.h>
#include <stdint.h>
#include <string>
#include <vector>

class CentralProcessingUnitCore;
struct Registers;

class IMemory
{
public:
	virtual void WriteFromRealMemory(CentralProcessingUnitCore* core, void* srcPointer, uint32_t dstAddress, size_t size = sizeof(uint32_t)) = 0;
	virtual void ReadToRealMemory(CentralProcessingUnitCore* core, uint32_t srcAddress, void* dstPointer, size_t size = sizeof(uint32_t)) = 0;
	virtual void Write(CentralProcessingUnitCore* core, uint32_t srcAddress, uint32_t dstAddress, size_t size = sizeof(uint32_t)) = 0;
	virtual void Read(CentralProcessingUnitCore* core, uint32_t srcAddress, uint32_t dstAddress, size_t size = sizeof(uint32_t)) = 0;
	virtual PointerRange AddressToPointerRange(CentralProcessingUnitCore* core, uint32_t address, size_t size = sizeof(uint32_t)) = 0;
};
