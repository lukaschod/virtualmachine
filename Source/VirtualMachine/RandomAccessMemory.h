#pragma once

#include <stdint.h>
#include <string>
#include <vector>
#include "Helper.h"
#include "IMemory.h"

class CentralProcessingUnitCore;

class RandomAccessMemory : public IMemory
{
public:
	RandomAccessMemory(uint32_t pageCount, uint32_t pageSize);

	virtual void WriteFromRealMemory(CentralProcessingUnitCore* core, void* srcPointer, uint32_t dstAddress, size_t size = sizeof(uint32_t));
	virtual void ReadToRealMemory(CentralProcessingUnitCore* core, uint32_t srcAddress, void* dstPointer, size_t size = sizeof(uint32_t));
	virtual void Write(CentralProcessingUnitCore* core, uint32_t srcAddress, uint32_t dstAddress, size_t size = sizeof(uint32_t));
	virtual void Read(CentralProcessingUnitCore* core, uint32_t srcAddress, uint32_t dstAddress, size_t size = sizeof(uint32_t));
	virtual PointerRange AddressToPointerRange(CentralProcessingUnitCore* core, uint32_t address, size_t size = sizeof(uint32_t));

	void WriteZeros(CentralProcessingUnitCore* core, uint32_t address, size_t size = sizeof(uint32_t));
	std::string ToString(int columnCount = 8);

private:
	AUTOMATED_PROPERTY_GET(uint32_t, pageCount);
	AUTOMATED_PROPERTY_GET(uint32_t, pageSize);
	std::vector<uint8_t> memory;
};
