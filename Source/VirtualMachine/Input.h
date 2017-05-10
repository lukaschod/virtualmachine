#pragma once

#include <stdint.h>
#include <vector>
#include <stdio.h>

class CentralProcessingUnit;
struct Registers;

class Input
{
public:
	Input();

	void ReadUntilEnter(CentralProcessingUnit* core, uint32_t address);
};
