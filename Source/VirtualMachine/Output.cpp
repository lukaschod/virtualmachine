#include "Output.h"
#include "MemoryManagmentUnit.h"
#include "CentralProcessingUnit.h"
#include <stdio.h>

Output::Output()
{
}

void Output::PrintToScreen(CentralProcessingUnit* core, uint32_t address)
{
	auto memory = core->Get_memory();
	// TODO: fix this inefficient shit
	while (true)
	{
		char letter;
		memory->ReadToRealMemory(core, address, &letter, sizeof(char));
		address++;

		if (letter == '\0')
			break;

		printf("%c", letter);
	}
}