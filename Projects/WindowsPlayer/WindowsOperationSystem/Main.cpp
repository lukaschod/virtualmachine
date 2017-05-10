#include <OperationSystem\OperationSystem.h>
#include <OperationSystem\Processes\ProcessUser.h>
#include <VirtualMachine\RealMachine.h>
#include <VirtualMachine\RandomAccessMemory.h>
#include <VirtualMachine\MemoryManagmentUnit.h>
#include <VirtualMachine\Code.h>
#include <iostream>

/*Process* CreateProcess1(OperationSystem* operationSystem)
{
	const char* source =
		"DATA FileHandle 0\n"
		"DATA FilePath &C:\\Users\\Lukas-PC\\Desktop\\random.txt&\n"
		"DATA DataToWrite &Writing some stuff for fun...&\n"

		// Open file
		"LDC FilePath\n"
		"LDC 2\n"
		"INT 2\n"
		"STI FileHandle\n"

		// Write to file
		"LDI FileHandle\n"
		"LDC DataToWrite\n"
		"LDC 8\n"
		"INT 5\n"

		// Close file
		"LDI FileHandle\n"
		"INT 3\n"

		"HALT";

	auto code = new Code();
	assert(code->Compile(source));

	auto process = new ProcessUser("CreateFile", operationSystem->Get_startStopProcess(), ProcessPriority::kProcessPrioritykProcessPriorityLow, operationSystem, code);
	return process;
}

Process* CreateProcess2(OperationSystem* operationSystem)
{
	const char* source =
		"DATA FileHandle 0\n"
		"DATA FilePath &C:\\Users\\Lukas-PC\\Desktop\\random2.txt&\n"
		"DATA DataToWrite &Writing some stuff for fun...&\n"

		// Open file
		"LDC FilePath\n"
		"LDC 2\n"
		"INT 2\n"
		"STI FileHandle\n"

		// Write to file
		"LDI FileHandle\n"
		"LDC DataToWrite\n"
		"LDC 8\n"
		"INT 5\n"

		// Close file
		"LDI FileHandle\n"
		"INT 3\n"

		"HALT";

	auto code = new Code();
	assert(code->Compile(source));

	auto process = new ProcessUser("CreateFile", operationSystem->Get_startStopProcess(), ProcessPriority::kProcessPrioritykProcessPriorityLow, operationSystem, code);
	return process;
}*/

int main(int argv, char* argc[])
{
	auto operationSystem = new OperationSystem();

	//CreateProcess1(operationSystem);
	//CreateProcess2(operationSystem);

	operationSystem->Start();
	operationSystem->WaitTillFinishes();

	delete operationSystem;

	return 0;
}