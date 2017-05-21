#include "RandomAccessMemory.h"
#include "CentralProcessingUnit.h"
#include <assert.h>
#include <sstream>
#include <iomanip>

RandomAccessMemory::RandomAccessMemory(uint32_t pageCount, uint32_t pageSize) :
	pageCount(pageCount),
	pageSize(pageSize),
	memory(pageCount * pageSize)
{
}

void RandomAccessMemory::WriteFromRealMemory(CentralProcessingUnitCore* core, void* srcPointer, uint32_t dstAddress, size_t size)
{
	auto dstPointer = AddressToPointerRange(core, dstAddress, size);
	if (dstPointer.size == 0)
		return;
	memcpy(dstPointer.pointer, srcPointer, size);
}

void RandomAccessMemory::ReadToRealMemory(CentralProcessingUnitCore* core, uint32_t srcAddress, void* dstPointer, size_t size)
{
	auto srcPointer = AddressToPointerRange(core, srcAddress, size);
	if (srcPointer.size == 0)
		return;
	memcpy(dstPointer, srcPointer.pointer, size);
}

void RandomAccessMemory::Write(CentralProcessingUnitCore* core, uint32_t srcAddress, uint32_t dstAddress, size_t size)
{
	auto srcPointer = AddressToPointerRange(core, srcAddress, size);
	auto dstPointer = AddressToPointerRange(core, dstAddress, size);
	if (srcPointer.size == 0 || dstPointer.size == 0)
		return;
	memcpy(dstPointer.pointer, srcPointer.pointer, size);
}

void RandomAccessMemory::Read(CentralProcessingUnitCore* core, uint32_t srcAddress, uint32_t dstAddress, size_t size)
{
	auto srcPointer = AddressToPointerRange(core, srcAddress, size);
	auto dstPointer = AddressToPointerRange(core, dstAddress, size);
	if (srcPointer.size == 0 || dstPointer.size == 0)
		return;
	memcpy(dstPointer.pointer, srcPointer.pointer, size);
}

void RandomAccessMemory::WriteZeros(CentralProcessingUnitCore* core, uint32_t address, size_t size)
{
	auto pointer = AddressToPointerRange(core, address, size);
	if (pointer.size == 0)
		return;
	memset(pointer.pointer, 0, size);
}

PointerRange RandomAccessMemory::AddressToPointerRange(CentralProcessingUnitCore* core, uint32_t address, size_t size)
{
	assert(size != 0);

	if (address + size > memory.size())
	{
		return PointerRange();
	}

	PointerRange range;
	range.pointer = (void*) ((char*) memory.data() + address);
	range.size = size;
	return range;
}

std::string RandomAccessMemory::ToString(int columnCount)
{
	std::stringstream ss;
	ss << "RAM" << std::endl;
	ss << std::right << std::setw(8 + 1) << "Address";
	ss << std::right << std::setw(columnCount * 8) << "Values";
	ss << std::endl;

	int index = 0;
	for (unsigned int i = 0; i < memory.size() / columnCount; i++)
	{
		ss << std::right << std::setw(8) << std::setfill('0') << index << ":";
		for (int j = 0; j < columnCount; j++)
		{
			ss << std::right << std::setw(8) << std::setfill(' ') << (int)memory.at(index);
			index++;
		}
		ss << std::endl;
	}

	return ss.str();
}