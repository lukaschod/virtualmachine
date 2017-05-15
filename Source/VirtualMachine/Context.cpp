#include <VirtualMachine\Context.h>

Context::Context(ContextEntryPoint entryPoint, void* data)
{
	fiber = CreateFiber(400, entryPoint, data);
}

Context::Context()
{
	fiber = ConvertThreadToFiber(nullptr);
}

Context::~Context()
{
	DeleteFiber(fiber);
}

void Context::Apply()
{
	SwitchToFiber(fiber);
}