#pragma once

#include <VirtualMachine\Helper.h>
#include <Windows.h> // TODO: Make platform independant or implement on all of them

typedef void (_stdcall *ContextEntryPoint)(void*);

class Context
{
public:
	Context();
	Context(ContextEntryPoint entryPoint, void* data);
	~Context();

	void Apply();

private:
	LPVOID fiber;
};