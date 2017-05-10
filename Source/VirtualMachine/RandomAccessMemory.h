#pragma once

#include <stdint.h>
#include <string>
#include <vector>
#include "Helper.h"
#include "IMemory.h"

class CentralProcessingUnit;

class RandomAccessMemory : public IMemory
{
public:
	RandomAccessMemory(uint32_t pageCount, uint32_t pageSize);

	virtual void WriteFromRealMemory(CentralProcessingUnit* core, void* srcPointer, uint32_t dstAddress, size_t size = sizeof(uint32_t));
	virtual void ReadToRealMemory(CentralProcessingUnit* core, uint32_t srcAddress, void* dstPointer, size_t size = sizeof(uint32_t));
	virtual void Write(CentralProcessingUnit* core, uint32_t srcAddress, uint32_t dstAddress, size_t size = sizeof(uint32_t));
	virtual void Read(CentralProcessingUnit* core, uint32_t srcAddress, uint32_t dstAddress, size_t size = sizeof(uint32_t));
	virtual PointerRange AddressToPointerRange(CentralProcessingUnit* core, uint32_t address, size_t size = sizeof(uint32_t));
	virtual uint32_t AllocateMemory(CentralProcessingUnit* core, size_t size = sizeof(uint32_t));

	void WriteZeros(CentralProcessingUnit* core, uint32_t address, size_t size = sizeof(uint32_t));
	std::string ToString(int columnCount = 8);

private:
	AUTOMATED_PROPERTY_GET(uint32_t, pageCount);
	AUTOMATED_PROPERTY_GET(uint32_t, pageSize);
	std::vector<uint8_t> memory;
};
