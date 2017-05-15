#pragma once

#include <stdint.h>
#include <vector>
#include <stdio.h>

class CentralProcessingUnitCore;
struct Registers;

class Input
{
public:
	Input();

	void ReadUntilEnter(CentralProcessingUnitCore* core, uint32_t address);
};
