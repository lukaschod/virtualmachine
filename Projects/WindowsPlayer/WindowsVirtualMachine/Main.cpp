#include <VirtualMachine\VirtualMachine.h>
#include <VirtualMachine\RealMachine.h>
#include <VirtualMachine\RandomAccessMemory.h>
#include <VirtualMachine\MemoryManagmentUnit.h>
#include <VirtualMachine\ExternalMemory.h>
#include <VirtualMachine\Code.h>
#include <iostream>
#include <conio.h>
#include <string>

int main(int argv, char* argc[])
{
    // We initialize the hardware layer that will simulate our real PC hardware components
    auto realMachineOptions = RealMachineCreateOptions();
    realMachineOptions.pathToMachine = "D:\\Source\\virtualmachine\\Demo\\MyVirtualMachine\\";
    realMachineOptions.pageCount = 60;
    auto realMachine = new RealMachine(realMachineOptions);

    auto core = (CentralProcessingUnitCore*)realMachine->GetCpu();
    auto ram = realMachine->GetRam();
    auto externalMemory = realMachine->Get_externalMemory();

    auto source =
        ".data message \"Hello I'm user code for BIOS\n\"\n"
        "ldc.i4 message\n"
        "int 1\n"
        "halt\n";
    auto program = new Program();
    program->CreateFromText(source, strlen(source));

    auto virtualMachine = new VirtualMachine(program);
    virtualMachine->WriteHeaderAndPageTable(core, (ram->Get_pageCount() - 10) * ram->Get_pageSize());

    *core->Get_context() = virtualMachine->Get_context();
    core->Get_context()->registerUserMode = false;

    // Allocate the segments
    virtualMachine->WriteDataSegment(core);
    virtualMachine->WriteCodeSegment(core);
    virtualMachine->WriteStackSegment(core);

    core->Get_context()->registerUserMode = true;

    //realMachine->GetCpu()->Set_interuptHandler(this); // Overrides cpu interupt handlers, its OS core.
    realMachine->GetCpu()->SetInterupt(kInteruptCodeOSStart);

    realMachine->Start(); // Start the real machine cores to run
    realMachine->WaitTillFinishes(); // Wat till real machine finishes

    // Cleanup everything
    delete realMachine;

	getchar();

	return 0;
}