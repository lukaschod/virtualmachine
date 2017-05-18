#include "VirtualMachine.h"
#include "RealMachine.h"
#include "RandomAccessMemory.h"
#include "MemoryManagmentUnit.h"
#include "Code.h"
#include <string.h>

VirtualMachine::VirtualMachine(Program* program) :
	program(program)
{
}

void VirtualMachine::WriteHeaderAndPageTable(CentralProcessingUnitCore* core, uint32_t address)
{
	auto ram = core->Get_ram();

	// Allocate physical address for it
	auto pageCount = ram->Get_pageCount();
	auto pagerSize = sizeof(PageEntry) * pageCount;
	auto virtualMachineSize = sizeof(VirtualMachineHeader) + sizeof(PageTable) + pagerSize;

	// Write the header of machine
	VirtualMachineHeader virtualMachineHeader;
	virtualMachineHeader.isValid = 1;
	ram->WriteFromRealMemory(core, &virtualMachineHeader, address, sizeof(VirtualMachineHeader));
	address += sizeof(VirtualMachineHeader);

	// Write the header of page table
	context.registerPS = address;
	PageTable pageTableHeader;
	pageTableHeader.isValid = 1;
	pageTableHeader.allocatedPageCount = 0;
	ram->WriteFromRealMemory(core, &pageTableHeader, address, sizeof(PageTable));
	address += sizeof(PageTable);

	// Reset page entries to zeroes
	ram->WriteZeros(core, address, pagerSize);
	address += pagerSize;

	context.registerDS = 0;
	context.registerCS = context.registerDS + program->GetDataSegmentSize();
	context.registerIC = context.registerCS;
	context.registerSS = context.registerCS + program->GetCodeSegmentSize();
	context.registerSC = context.registerSS;
	context.registerEN = context.registerSS + program->GetStackSegmentSize();
}

void VirtualMachine::WriteDataSegment(CentralProcessingUnitCore* core)
{
	auto mmu = core->Get_mmu();
	mmu->AllocateMemory(core, program->GetDataSegmentSize());
	if (program->GetDataSegmentSize() > 0)
		mmu->WriteFromRealMemory(core, program->GetDataSegment(), context.registerDS, program->GetDataSegmentSize());
}

void VirtualMachine::WriteCodeSegment(CentralProcessingUnitCore* core)
{
	auto mmu = core->Get_mmu();
	mmu->AllocateMemory(core, program->GetCodeSegmentSize());
	if (program->GetCodeSegmentSize() > 0)
		mmu->WriteFromRealMemory(core, program->GetCodeSegment(), context.registerCS, program->GetCodeSegmentSize());
}

void VirtualMachine::WriteStackSegment(CentralProcessingUnitCore* core)
{
	auto mmu = core->Get_mmu();
	mmu->AllocateMemory(core, program->GetStackSegmentSize());
}

/*uint32_t VirtualMachine::Allocate(CentralProcessingUnitCore* core, Program* code)
{
	auto ram = core->Get_ram();
	
	// Allocate physical address for it
	auto pageEntrySize = sizeof(PageEntry) * code->GetPageSegmentSize();
	auto virtualMachineSize = sizeof(VirtualMachineHeader) + sizeof(PageTable) + pageEntrySize;
	auto physicalMachineAddress = ram->AllocateMemory(core, virtualMachineSize);
	
	// Write the header of machine
	VirtualMachineHeader virtualMachineHeader;
	virtualMachineHeader.isValid = 1;
	ram->WriteFromRealMemory(core, &virtualMachineHeader, physicalMachineAddress, sizeof(VirtualMachineHeader));
	physicalMachineAddress += sizeof(VirtualMachineHeader);

	// Write the header of page table
	context.registerPS = physicalMachineAddress;
	PageTable pageTableHeader;
	pageTableHeader.isValid = 1;
	pageTableHeader.allocatedPageCount = 0;
	ram->WriteFromRealMemory(core, &pageTableHeader, physicalMachineAddress, sizeof(PageTable));
	physicalMachineAddress += sizeof(PageTable);

	// Reset page entries to zeroes
	ram->WriteZeros(core, physicalMachineAddress, pageEntrySize);
	physicalMachineAddress += pageEntrySize;

	auto mmu = core->Get_mmu();
	uint32_t virtualMachineAddress = 0;

	context.registerDS = virtualMachineAddress;
	mmu->AllocateMemory(core, code->GetDataSegmentSize());
	if (code->GetDataSegmentSize() > 0)
		mmu->WriteFromRealMemory(core, code->GetDataSegment(), virtualMachineAddress, code->GetDataSegmentSize());
	virtualMachineAddress += code->GetDataSegmentSize();

	context.registerCS = virtualMachineAddress;
	context.registerIC = context.registerCS;
	mmu->AllocateMemory(core, code->GetCodeSegmentSize());
	if (code->GetCodeSegmentSize() > 0)
		mmu->WriteFromRealMemory(core, code->GetCodeSegment(), virtualMachineAddress, code->GetCodeSegmentSize());
	virtualMachineAddress += code->GetCodeSegmentSize();

	context.registerSS = virtualMachineAddress;
	context.registerSC = context.registerSS;
	mmu->AllocateMemory(core, code->GetStackSegmentSize());
	virtualMachineAddress += code->GetStackSegmentSize();
	
	context.registerEN = virtualMachineAddress;

	return 0;
}*/