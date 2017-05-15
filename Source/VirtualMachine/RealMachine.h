#pragma once

#include <VirtualMachine\Helper.h>
#include <vector>

class MemoryManagmentUnit;
class RandomAccessMemory;
class CentralProcessingUnit;
class VirtualMachine;
class ExternalMemory;
class Input;
class Output;

struct RealMachineCreateOptions
{
	const char* pathToMachine;
	size_t maximumOpenedFileCount = 20;
	size_t pageCount = 40;
	size_t pageSize = 128;
};

// Fake real machine that simulates PC components
class RealMachine
{
public:
	RealMachine(RealMachineCreateOptions& options);
	~RealMachine();

	void Start();
	void Stop();
	void WaitTillFinishes();

public:
	inline RandomAccessMemory* GetRam() const { return ram; }
	inline CentralProcessingUnit* GetCpu() const { return cpu; }

private:
	RandomAccessMemory* ram;
	CentralProcessingUnit* cpu;
	AUTOMATED_PROPERTY_GET(ExternalMemory*, externalMemory);
	Input* input;
	Output* output;
	bool isStarted;
};