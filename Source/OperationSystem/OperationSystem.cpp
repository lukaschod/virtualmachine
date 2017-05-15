#include <OperationSystem\OperationSystem.h>
#include <OperationSystem\ResourcePlanner.h>
#include <OperationSystem\ProcessPlanner.h>
#include <OperationSystem\Processes\ProcessStartStop.h>
#include <OperationSystem\Processes\ProcessExternalMemory.h>
#include <OperationSystem\Processes\ProcessManager.h>
#include <OperationSystem\Processes\ProcessOutput.h>
#include <OperationSystem\Processes\ProcessInput.h>
#include <OperationSystem\Processes\ProcessProgram.h>
#include <OperationSystem\Resource.h>
#include <VirtualMachine\RealMachine.h>
#include <VirtualMachine\CentralProcessingUnit.h>
#include <VirtualMachine\MemoryManagmentUnit.h>

OperationSystem::OperationSystem(RealMachine* realMachine)
{
	this->realMachine = realMachine;
	resourcePlanner = new ResourcePlanner(this);
	processPlanner = new ProcessPlanner(this);
	resourcePlanner->Set_processPlanner(processPlanner);
	processPlanner->Set_resourcePlanner(resourcePlanner);
	realMachine->GetCpu()->Set_interuptHandler(this);
	pageSize = realMachine->GetRam()->Get_pageSize();
	pageCount = realMachine->GetCpu()->Get_ram()->Get_pageCount();

	realMachine->GetCpu()->Set_interuptHandler(this); // Overrides cpu interupt handlers, its OS core.
	realMachine->GetCpu()->SetInterupt(kInteruptCodeCPUStart);
}

OperationSystem::~OperationSystem()
{
	delete processPlanner;
	delete resourcePlanner;
}

bool OperationSystem::ShouldSkipNextInstruction(CentralProcessingUnitCore* core)
{
	auto process = (Process*) core->Get_process();
	if (process == nullptr)
		return false;

	// Execute queued kernel instructions before handling the 
	return process->ExecuteCallback();
}

bool OperationSystem::HandleInterupt(CentralProcessingUnitCore* core)
{
	auto process = (Process*) core->Get_process();
	auto mmu = core->Get_mmu();
	switch (core->Get_context()->registerINT)
	{
	case kInteruptCreateProgramFromSource:
	{
		auto address = core->ExecuteInstructionPop();
		startStopProcess->Get_processProgramManager()->CreateProgramFromSource(core, address, [process](CentralProcessingUnitCore* core)
		{
			core->ExecuteInstructionPush(process->GetRequestedResourceElementReturn());
		});
		break;
	}

	case kInteruptLoadProgramFromFile:
	{
		auto address = core->ExecuteInstructionPop();
		startStopProcess->Get_processProgramManager()->LoadProgramFromFile(core, address, [process](CentralProcessingUnitCore* core)
		{
			core->ExecuteInstructionPush(process->GetRequestedResourceElementReturn());
		});
		break;
	}

	case kInteruptSaveProgramToFile:
	{
		auto programHandle = core->ExecuteInstructionPop();
		auto address = core->ExecuteInstructionPop();
		startStopProcess->Get_processProgramManager()->SaveProgramToFile(core, address, programHandle,[process](CentralProcessingUnitCore* core)
		{
			core->ExecuteInstructionPush(process->GetRequestedResourceElementReturn());
		});
		break;
	}

	case kInteruptDestroyProgram:
	{
		auto programHandle = core->ExecuteInstructionPop();
		startStopProcess->Get_processProgramManager()->DestroyProgram(core, programHandle);
		break;
	}

	case kInteruptCodeInputReadUntilEnter:
	{
		auto address = core->ExecuteInstructionPop();
		startStopProcess->Get_processInput()->ReadLine(core, address);
		break;
	}

	case kInteruptCodeOutputPrintToScreen:
	{
		auto address = core->ExecuteInstructionPop();
		startStopProcess->Get_processOutput()->PrintLine(core, address, 0);
		break;
	}

    case kInteruptCodeExternalMemoryOpenFile:
    {
        auto accessFlag = (FileAccessFlag)core->ExecuteInstructionPop();
        auto filePathAddress = core->ExecuteInstructionPop();
        startStopProcess->Get_processExternalMemory()->OpenFile(core, filePathAddress, accessFlag, [process](CentralProcessingUnitCore* core)
        {
            core->ExecuteInstructionPush(process->GetRequestedResourceElementReturn());
        });
		return true;
    }

    case kInteruptCodeExternalMemoryCloseFile:
    {
        auto fileHandle = core->ExecuteInstructionPop();
        startStopProcess->Get_processExternalMemory()->CloseFile(core, fileHandle);
		return true;
    }

    case kInteruptCodeExternalMemoryReadFile:
    {
        auto size = core->ExecuteInstructionPop();
        auto address = core->ExecuteInstructionPop();
        auto fileHandle = core->ExecuteInstructionPop();
        startStopProcess->Get_processExternalMemory()->ReadFile(core, fileHandle, address, size, [process](CentralProcessingUnitCore* core)
        {
            core->ExecuteInstructionPush(process->GetRequestedResourceElementReturn());
        });
		return true;
    }

    case kInteruptCodeExternalMemoryWriteFile:
    {
        auto size = core->ExecuteInstructionPop();
        auto address = core->ExecuteInstructionPop();
        auto fileHandle = core->ExecuteInstructionPop();
        startStopProcess->Get_processExternalMemory()->WriteFile(core, fileHandle, address, size, [process](CentralProcessingUnitCore* core)
        {
            core->ExecuteInstructionPush(process->GetRequestedResourceElementReturn());
        });
		return true;
    }

	case kInteruptCodeFailurePage:
	{
		resourcePlanner->RequestResourceElementAny(startStopProcess->Get_resourceMemory(), process);

		auto& queuedInstructions = process->Get_queuedKernelInstructions();
		if (queuedInstructions.size() != 0)
		{
			queuedInstructions.insert(queuedInstructions.begin(), queuedInstructions.front());
		}
		else
		{
			core->Get_context()->registerIC = core->Get_context()->registerLastIC; // Rollback the instructions
		}

		process->ExecuteWhenRunning([process](CentralProcessingUnitCore* core)
		{
			auto pageEntryAddress = core->Get_context()->registerGeneral;
			auto mmu = core->Get_mmu();
			auto physicalAddress = process->GetRequestedResourceElementReturn();
			mmu->AllocatePage(core, pageEntryAddress, physicalAddress);
			
		});

		return true;
	}

	case kInteruptCodeCreateProcess:
	{
		//auto size = core->ExecuteInstructionPop();
		//auto address = core->ExecuteInstructionPop();
		auto fileHandle = core->ExecuteInstructionPop();
		startStopProcess->Get_processManager()->CreateProcessUser(core, fileHandle, [process](CentralProcessingUnitCore* core)
		{
			core->ExecuteInstructionPush(process->GetRequestedResourceElementReturn());
		});
		return true;
	}

	case kInteruptCodeCPUStart:
	{
		startStopProcess = processPlanner->CreateProcessStartStop();
		return true;
	}

	case kInteruptCodeTimer:
	{
		processPlanner->SwitchContext(core);
		return true;
	}

	// TODO: This should throw error to process and process itself should decided if it wants abort
	case kInteruptCodeFailureGeneral:
	case kInteruptCodeFailureMemoryException:

	case kInteruptCodeHalt:
	case kInteruptCodeAbort:
	{
		auto processHandle = startStopProcess->Get_processManager()->ProcessUserToHandle((ProcessUser*) process);
		startStopProcess->Get_processManager()->DestroyProcessUser(core, processHandle);
		core->ExecuteInstructionPush(process->GetRequestedResourceElementReturn());
		return true;
	}
	}
	return false;
}