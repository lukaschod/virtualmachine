#pragma once

#include <stdint.h>
#include <vector>
#include <stdio.h>

class CentralProcessingUnit;
struct Registers;

class Output
{
public:
	Output();

	void PrintToScreen(CentralProcessingUnit* core, uint32_t address);
};