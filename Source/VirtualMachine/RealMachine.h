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

// Fake real machine that simulates PC components
class RealMachine
{
public:
	RealMachine();
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