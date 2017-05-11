#include "CentralProcessingUnit.h"
#include "MemoryManagmentUnit.h"
#include "RandomAccessMemory.h"
#include <assert.h>
#include <sstream>
#include <iomanip>

MemoryManagmentUnit::MemoryManagmentUnit(RandomAccessMemory* ram)
{
	assert(ram != nullptr);
	this->ram = ram;
	this->pageSize = ram->Get_pageSize();
}

void MemoryManagmentUnit::WriteFromRealMemory(CentralProcessingUnit* core, void* srcPointer, uint32_t dstAddress, size_t size)
{
	assert(srcPointer != nullptr);
	assert(size != 0);
	auto currentSrcPointer = (char*) srcPointer;

	while (size != 0)
	{
		assert(size > 0);
		auto range = VirtualToPhysicalAddress(core, dstAddress, size);

		if (core->HandleInterupts())
			return;

		ram->WriteFromRealMemory(core, currentSrcPointer, range.address, range.size);
		dstAddress += range.size;
		currentSrcPointer += range.size;
		size -= range.size;
	}
}

void MemoryManagmentUnit::ReadToRealMemory(CentralProcessingUnit* core, uint32_t srcAddress, void* dstPointer, size_t size)
{
	assert(dstPointer != nullptr);
	assert(size != 0);
	auto currentDstPointer = (char*) dstPointer;

	while (size != 0)
	{
		assert(size > 0);
		auto range = VirtualToPhysicalAddress(core, srcAddress, size);

		if (core->HandleInterupts())
			return;

		ram->ReadToRealMemory(core, range.address, currentDstPointer, range.size);
		srcAddress += range.size;
		currentDstPointer += range.size;
		size -= range.size;
	}
}

void MemoryManagmentUnit::Write(CentralProcessingUnit* core, uint32_t srcAddress, uint32_t dstAddress, size_t size)
{
	assert(size != 0);
	while (size != 0)
	{
		assert(size > 0);
		auto srcRange = VirtualToPhysicalAddress(core, srcAddress, size);
		auto dstRange = VirtualToPhysicalAddress(core, dstAddress, size);
		auto copyRange = __min(srcRange.size, dstRange.size);

		if (core->HandleInterupts())
			return;

		ram->Write(core, srcRange.address, dstRange.address, copyRange);
		srcAddress += copyRange;
		dstAddress += copyRange;
		size -= copyRange;
	}
}

void MemoryManagmentUnit::Read(CentralProcessingUnit* core, uint32_t srcAddress, uint32_t dstAddress, size_t size)
{
	assert(size != 0);
	while (size != 0)
	{
		assert(size > 0);
		auto srcRange = VirtualToPhysicalAddress(core, srcAddress, size);
		auto dstRange = VirtualToPhysicalAddress(core, dstAddress, size);
		auto copyRange = __min(srcRange.size, dstRange.size);

		if (core->HandleInterupts())
			return;

		ram->Read(core, srcRange.address, dstRange.address, copyRange);
		srcAddress += copyRange;
		dstAddress += copyRange;
		size -= copyRange;
	}
}

PointerRange MemoryManagmentUnit::AddressToPointerRange(CentralProcessingUnit* core, uint32_t address, size_t size)
{
	auto range = VirtualToPhysicalAddress(core, address, size);
	return ram->AddressToPointerRange(core, range.address, range.size);
}

AddressRange MemoryManagmentUnit::VirtualToPhysicalAddress(CentralProcessingUnit* core,
	uint32_t virtualAddress, size_t virtualAddressRange)
{
	auto registerPS = core->Get_context()->registerPS;
	auto virtualPageAddress = virtualAddress / pageSize;
	auto physicalPageAddress = registerPS + sizeof(PageTable) + virtualPageAddress * sizeof(PageEntry);

	PageEntry pageEntry;
	ram->ReadToRealMemory(core, physicalPageAddress, &pageEntry, sizeof(PageEntry));

	// Check if memory is actually allocated in table
	if (!pageEntry.isAllocated)
	{
		core->SetInterupt(kInteruptCodeFailureMemoryException);
		return AddressRange();
	}

	// Check if virtual address resolved to physical
	if (!pageEntry.isPhysicalAddressValid)
	{
		core->Get_context()->registerGeneral = virtualPageAddress;
		core->SetInterupt(kInteruptCodeFailurePage);
		return AddressRange();
	}

	AddressRange addressRange;
	addressRange.address = pageEntry.physicalAddress + virtualAddress % pageSize;
	addressRange.size = __min(pageSize - (virtualAddress % pageSize), virtualAddressRange);
	return addressRange;
}

uint32_t MemoryManagmentUnit::AllocateMemory(CentralProcessingUnit* core, size_t size)
{
	auto registerPS = core->Get_context()->registerPS;
	PageTable pageTable;
	ram->ReadToRealMemory(core, registerPS, &pageTable, sizeof(PageTable));
	assert(pageTable.isValid == 1);
	auto addressToAllocate = pageTable.allocatedPageCount * sizeof(PageEntry);

	auto pageCountToAllocate = (size / pageSize) + ((size % pageSize != 0) ? 1 : 0);
	for (unsigned int i = 0; i < pageCountToAllocate; i++)
	{
		PageEntry pageEntry;
		pageEntry.physicalAddress = 0;
		pageEntry.isAllocated = true;
		pageEntry.isPhysicalAddressValid = false;
		ram->WriteFromRealMemory(core, &pageEntry, registerPS + sizeof(PageTable) + addressToAllocate, sizeof(PageEntry));
		addressToAllocate += sizeof(PageEntry);
	}

	pageTable.allocatedPageCount += pageCountToAllocate;
	ram->WriteFromRealMemory(core, &pageTable, registerPS, sizeof(PageTable));
	return addressToAllocate;
}

void MemoryManagmentUnit::AllocatePage(CentralProcessingUnit* core, uint32_t pageIndex, uint32_t physicalAddress)
{
	auto context = core->Get_context();
	auto physicalPageAddress = context->registerPS + sizeof(PageTable) + pageIndex * sizeof(PageEntry);

	PageEntry pageEntry;
	ram->ReadToRealMemory(core, physicalPageAddress, &pageEntry, sizeof(PageEntry));

	pageEntry.physicalAddress = physicalAddress;
	pageEntry.isPhysicalAddressValid = true;
	ram->WriteFromRealMemory(core, &pageEntry, physicalPageAddress, sizeof(PageEntry));
}

std::string MemoryManagmentUnit::ToString(CentralProcessingUnit* core)
{
	auto registerPS = core->Get_context()->registerPS;
	PageTable pageTable;
	ram->ReadToRealMemory(core, registerPS, &pageTable, sizeof(PageTable));
	assert(pageTable.isValid == 1);

	std::stringstream ss;
	ss << "Page Table" << std::endl;
	ss << std::right << std::setw(8 + 1) << "Address";
	ss << std::right << std::setw(16 ) << "IsAllocated";
	ss << std::right << std::setw(20) << "HasPhysicalAddress";
	ss << std::right << std::setw(16) << "PhysicalAddress";
	ss << std::endl;

	auto addressToAllocate = registerPS + sizeof(PageTable);
	for (unsigned int i = 0; i < pageTable.allocatedPageCount; i++)
	{
		PageEntry pageEntry;
		ram->ReadToRealMemory(core, addressToAllocate, &pageEntry, sizeof(PageEntry));
		ss << std::right << std::setw(8) << std::setfill('0') << addressToAllocate << ":";
		ss << std::right << std::setw(16) << std::setfill(' ') << (pageEntry.isAllocated ? "True" : "False");
		ss << std::right << std::setw(20) << std::setfill(' ') << (pageEntry.isPhysicalAddressValid ? "True" : "False");
		ss << std::right << std::setw(16) << std::setfill(' ') << pageEntry.physicalAddress;
		ss << std::endl;
		addressToAllocate += sizeof(PageEntry);
	}

	return ss.str();
}