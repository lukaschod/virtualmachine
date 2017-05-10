#include "RealMachine.h"
#include "MemoryManagmentUnit.h"
#include "RandomAccessMemory.h"
#include "CentralProcessingUnit.h"
#include "VirtualMachine.h"
#include "ExternalMemory.h"
#include "Input.h"
#include "Output.h"

RealMachine::RealMachine()
{
	isStarted = false;
	ram = new RandomAccessMemory(40, 128);
	externalMemory = new ExternalMemory(20);
	input = new Input();
	output = new Output();
	cpu = new CentralProcessingUnit(ram, externalMemory, input, output);
}

RealMachine::~RealMachine()
{
	delete ram;
	delete externalMemory;
	delete input;
	delete output;
	delete cpu;
}

void RealMachine::Start()
{
	cpu->Start();
	isStarted = true;
}

void RealMachine::Stop()
{
	cpu->Stop();
	isStarted = false;
}

void RealMachine::WaitTillFinishes()
{
	cpu->WaitTillFinishes();
}