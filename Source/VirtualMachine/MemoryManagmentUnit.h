#pragma once

#include "IMemory.h"
#include "Helper.h"
#include <string>

class CentralProcessingUnitCore;
class RandomAccessMemory;

struct PageTable
{
	uint32_t allocatedPageCount;
	bool isValid;
};

struct PageEntry
{
	uint32_t physicalAddress;
	bool isAllocated;
	bool isPhysicalAddressValid;
};

class MemoryManagmentUnit : public IMemory
{
public:
	MemoryManagmentUnit(RandomAccessMemory* ram);

	virtual void WriteFromRealMemory(CentralProcessingUnitCore* core, void* srcPointer, uint32_t dstAddress, size_t size = sizeof(uint32_t));
	virtual void ReadToRealMemory(CentralProcessingUnitCore* core, uint32_t srcAddress, void* dstPointer, size_t size = sizeof(uint32_t));
	virtual void Write(CentralProcessingUnitCore* core, uint32_t srcAddress, uint32_t dstAddress, size_t size = sizeof(uint32_t));
	virtual void Read(CentralProcessingUnitCore* core, uint32_t srcAddress, uint32_t dstAddress, size_t size = sizeof(uint32_t));
	virtual PointerRange AddressToPointerRange(CentralProcessingUnitCore* core, uint32_t address, size_t size = sizeof(uint32_t));
	virtual uint32_t AllocateMemory(CentralProcessingUnitCore* core, size_t size = sizeof(uint32_t));

	void AllocatePage(CentralProcessingUnitCore* core, uint32_t pageIndex, uint32_t physicalAddress);

	std::string ToString(CentralProcessingUnitCore* core);

private:
	AddressRange VirtualToPhysicalAddress(CentralProcessingUnitCore* core, uint32_t virtualAddress, size_t size);

private:
	RandomAccessMemory* ram;
	AUTOMATED_PROPERTY_GET(uint32_t, pageSize);
};