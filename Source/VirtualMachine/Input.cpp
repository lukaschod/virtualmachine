#include "Input.h"
#include "MemoryManagmentUnit.h"
#include "CentralProcessingUnit.h"
#include <stdio.h>

Input::Input()
{
}

void Input::ReadUntilEnter(CentralProcessingUnitCore* core, uint32_t address)
{
	auto memory = core->Get_memory();
	// TODO: fix this inefficient shit
	static char buffer[255];
	scanf_s("%254s", buffer, sizeof buffer);
	memory->WriteFromRealMemory(core, buffer, address, strlen(buffer));
}
