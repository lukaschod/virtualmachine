#pragma once

#include "Helper.h"
#include "CentralProcessingUnit.h"
#include <stdlib.h>

class Program;

struct VirtualMachineHeader
{
	bool isValid;
};

class VirtualMachine
{
public:
	VirtualMachine(Program* program);

	void WriteHeaderAndPageTable(CentralProcessingUnitCore* core, uint32_t address);
	void WriteCodeSegment(CentralProcessingUnitCore* core);
	void WriteDataSegment(CentralProcessingUnitCore* core);
	void WriteStackSegment(CentralProcessingUnitCore* core);
	//uint32_t Allocate(CentralProcessingUnitCore* core, Program* code);

private:
	AUTOMATED_PROPERTY_GETSET(Registers, context);
	Program* program;
};