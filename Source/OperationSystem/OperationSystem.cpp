#include <OperationSystem\OperationSystem.h>
#include <OperationSystem\ResourcePlanner.h>
#include <OperationSystem\ProcessPlanner.h>
#include <OperationSystem\Processes\ProcessStartStop.h>
#include <OperationSystem\Resource.h>
#include <VirtualMachine\RealMachine.h>
#include <VirtualMachine\CentralProcessingUnit.h>
#include <VirtualMachine\MemoryManagmentUnit.h>

OperationSystem::OperationSystem()
{
	realMachine = new RealMachine();
	resourcePlanner = new ResourcePlanner(this);
	processPlanner = new ProcessPlanner(this);
	resourcePlanner->Set_processPlanner(processPlanner);
	processPlanner->Set_resourcePlanner(resourcePlanner);
	realMachine->GetCpu()->Set_interuptHandler(this);
	pageSize = realMachine->GetRam()->Get_pageSize();
	pageCount = realMachine->GetCpu()->Get_ram()->Get_pageCount();

	startStopProcess = processPlanner->CreateProcessStartStop();
}

OperationSystem::~OperationSystem()
{
	delete processPlanner;
	delete resourcePlanner;
	delete realMachine;
}

void OperationSystem::Start()
{
	realMachine->Start();
}

void OperationSystem::Stop()
{
	realMachine->Stop();
}

void OperationSystem::WaitTillFinishes()
{
	realMachine->WaitTillFinishes();
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
	switch (core->Get_context()->registerINT)
	{
	case kInteruptCodeFailurePage:
	{
		auto memoryResource = startStopProcess->Get_memory();
		resourcePlanner->RequestResourceElementAny(startStopProcess->Get_memory(), process);

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

	case kInteruptCodeTimer:
	{
		processPlanner->SwitchContext(core);
		return true;
	}

	case kInteruptCodeHalt:
	{
		Stop();
		return true;
	}
	}
	return false;
}