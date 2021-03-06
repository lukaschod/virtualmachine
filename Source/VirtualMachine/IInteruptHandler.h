#pragma once

#include <VirtualMachine\Code.h>
#include <functional>

class CentralProcessingUnitCore;

class IInteruptHandler
{
public:
	virtual bool HandleInterupt(CentralProcessingUnitCore* core) = 0;
	virtual bool ShouldSkipNextInstruction(CentralProcessingUnitCore* core) = 0;
};