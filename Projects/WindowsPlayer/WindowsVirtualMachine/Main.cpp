#include <VirtualMachine\VirtualMachine.h>
#include <VirtualMachine\RealMachine.h>
#include <VirtualMachine\RandomAccessMemory.h>
#include <VirtualMachine\MemoryManagmentUnit.h>
#include <VirtualMachine\Code.h>
#include <iostream>

VirtualMachine* CreateOtherVirtualMachine(RealMachine* realMachine)
{
	const char* source =
		"DATA FileHandle 0\n"
		"DATA FilePath &C:\\Users\\Lukas-PC\\Source\\random2.txt&\n"
		"DATA DataToWrite &Writing some stuff for fun...&\n"

		"INTE 2 FilePath 2\n"
		"STRM FileHandle\n"
		"INTE 5 FileHandle DataToWrite 20\n"
		"INTE 3 FileHandle\n"

		"HALT";
	auto code = new Program();
	assert(code->CreateFromText(source));

	auto virtualMachine = new VirtualMachine(realMachine);
	virtualMachine->Allocate(code);

	delete code;

	return virtualMachine;
}

int main(int argv, char* argc[])
{
	auto realMachine = new RealMachine(16);

	const char* source = 
		"DATA FileHandle 0\n"
		"DATA FilePath &C:\\Users\\Lukas-PC\\Source\\random.txt&\n"
		"DATA DataToWrite &Writing some stuff for fun...&\n"
		"DATA SomeRandomSum 0\n"

		"LDC JumpHereMyFriend\n"
		"JMP\n"

		"LDI SomeRandomSum\n"
		"LDC 1\n"
		"ADD\n"
		"STI SomeRandomSum\n"

		"#LABEL JumpHereMyFriend\n"

		// [FileHandle] + [FilePath] = [SomeRandomSum]
		"LDC FileHandle\n"
		"LDC FilePath\n"
		"ADD\n"
		"STI SomeRandomSum\n"

		

		// Open file
		"LDC FilePath\n"
		"LDC 2\n"
		"INT 2\n"
		"STI FileHandle\n"

		// Write to file
		"LDI FileHandle\n"
		"LDC DataToWrite\n"
		"LDI SomeRandomSum\n"
		"INT 5\n"

		// Close file
		"LDI FileHandle\n"
		"INT 3\n"
		
		"HALT";

	auto code = new Program();
	assert(code->CreateFromText(source));

	auto virtualMachine = new VirtualMachine(realMachine);
	virtualMachine->Allocate(code);

	delete code;

	realMachine->GetCpu()->SetContext(virtualMachine->GetContext());
	realMachine->Start();
	while (realMachine->GetCpu()->IsStarted());
	realMachine->Stop();

	/*auto vm2 = CreateOtherVirtualMachine(realMachine);
	realMachine->GetCpu()->SetContext(vm2->GetContext());
	realMachine->Start();
	while (realMachine->GetCpu()->IsStarted());
	realMachine->Stop();*/

	delete virtualMachine;
	delete realMachine;

	getchar();

	return 0;
}