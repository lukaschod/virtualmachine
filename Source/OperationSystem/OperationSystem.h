#pragma once

#include <VirtualMachine\Helper.h>
#include <VirtualMachine\IInteruptHandler.h>
#include <VirtualMachine\Code.h>

class RealMachine;
class ResourcePlanner;
class ProcessPlanner;
class Process;
class ProcessStartStop;

class FidGenerator
{
public:
	FidGenerator() { lastFid = 1; }
	uint32_t GenerateUniqueFid() { return lastFid++; }

private:
	uint32_t lastFid;
};

class OperationSystem : public IInteruptHandler
{
public:
	OperationSystem(RealMachine* realMachine);
	~OperationSystem();

	virtual bool HandleInterupt(CentralProcessingUnitCore* core);
	virtual bool ShouldSkipNextInstruction(CentralProcessingUnitCore* core);

	uint32_t GenerateUniqueFid() { return fidGenerator.GenerateUniqueFid(); }

private:
	AUTOMATED_PROPERTY_GET(RealMachine*, realMachine);
	AUTOMATED_PROPERTY_GET(ResourcePlanner*, resourcePlanner);
	AUTOMATED_PROPERTY_GET(ProcessPlanner*, processPlanner);
	AUTOMATED_PROPERTY_GET(ProcessStartStop*, startStopProcess);
	AUTOMATED_PROPERTY_GET(uint32_t, pageSize);
	AUTOMATED_PROPERTY_GET(uint32_t, pageCount);
	FidGenerator fidGenerator;
};