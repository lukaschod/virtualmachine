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
	size_t offset = 0;
	//scanf_s("%254s", buffer, 254);
	while (true)
	{
		auto letter = getchar();

		if (letter == '\n' || letter == '\r')
			break;

		buffer[offset++] = letter;
	}
	buffer[offset++] = 0;
	memory->WriteFromRealMemory(core, buffer, address, offset);
}
