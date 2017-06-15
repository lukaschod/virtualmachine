#include <OperationSystem\OperationSystem.h>
#include <OperationSystem\Processes\ProcessUser.h>
#include <VirtualMachine\RealMachine.h>
#include <VirtualMachine\RandomAccessMemory.h>
#include <VirtualMachine\MemoryManagmentUnit.h>
#include <VirtualMachine\Code.h>
#include <iostream>

int main(int argv, char* argc[])
{
	// We initialize the hardware layer that will simulate our real PC hardware components
	auto realMachineOptions = RealMachineCreateOptions();
	realMachineOptions.pathToMachine = "D:\\Source\\virtualmachine\\Demo\\MyVirtualMachine\\";
	realMachineOptions.pageCount = 60;
	auto realMachine = new RealMachine(realMachineOptions);

	// We create operation system:
	// Allocate supervisor memory and apply it on real machine memory
	auto operationSystem = new OperationSystem(realMachine); 

	realMachine->Start(); // Start the real machine cores to run
	realMachine->WaitTillFinishes(); // Wat till real machine finishes

	// Cleanup everything
	delete operationSystem;
	delete realMachine;

	return 0;
}