#pragma once

#include "IMemory.h"
#include "Helper.h"
#include <string>

class CentralProcessingUnit;
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

	virtual void WriteFromRealMemory(CentralProcessingUnit* core, void* srcPointer, uint32_t dstAddress, size_t size = sizeof(uint32_t));
	virtual void ReadToRealMemory(CentralProcessingUnit* core, uint32_t srcAddress, void* dstPointer, size_t size = sizeof(uint32_t));
	virtual void Write(CentralProcessingUnit* core, uint32_t srcAddress, uint32_t dstAddress, size_t size = sizeof(uint32_t));
	virtual void Read(CentralProcessingUnit* core, uint32_t srcAddress, uint32_t dstAddress, size_t size = sizeof(uint32_t));
	virtual PointerRange AddressToPointerRange(CentralProcessingUnit* core, uint32_t address, size_t size = sizeof(uint32_t));
	virtual uint32_t AllocateMemory(CentralProcessingUnit* core, size_t size = sizeof(uint32_t));

	void AllocatePage(CentralProcessingUnit* core, uint32_t pageIndex, uint32_t physicalAddress);

	std::string ToString(CentralProcessingUnit* core);

private:
	AddressRange VirtualToPhysicalAddress(CentralProcessingUnit* core, uint32_t virtualAddress, size_t size);

private:
	RandomAccessMemory* ram;
	AUTOMATED_PROPERTY_GET(uint32_t, pageSize);
};