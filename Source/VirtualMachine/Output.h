#pragma once

#include <stdint.h>
#include <vector>
#include <stdio.h>

class CentralProcessingUnitCore;
struct Registers;

class Output
{
public:
	Output();

	void PrintToScreen(CentralProcessingUnitCore* core, uint32_t address);
};