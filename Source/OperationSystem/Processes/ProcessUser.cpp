#include <OperationSystem\Processes\ProcessUser.h>
#include <OperationSystem\Resource.h>
#include <OperationSystem\OperationSystem.h>
#include <VirtualMachine\CentralProcessingUnit.h>
#include <VirtualMachine\VirtualMachine.h>
#include <VirtualMachine\RealMachine.h>

ProcessUser::ProcessUser(const char* name, Process* parent, ProcessPriority priority, OperationSystem* operationSystem, VirtualMachine* virtualMachine) :
	Process(name, parent, priority, operationSystem),
	virtualMachine(virtualMachine)
{
	context = virtualMachine->Get_context();
	/*context.registerUserMode = false;
	
	ExecuteWhenRunning([this](CentralProcessingUnitCore* core)
	{
		this->virtualMachine->WriteDataSegment(core);
	});

	ExecuteWhenRunning([this](CentralProcessingUnitCore* core)
	{
		this->virtualMachine->WriteCodeSegment(core);
	});

	ExecuteWhenRunning([this](CentralProcessingUnitCore* core)
	{
		this->virtualMachine->WriteStackSegment(core);
	});

	ExecuteWhenRunning([this](CentralProcessingUnitCore* core)
	{
		core->Get_context()->registerUserMode = true;
	});*/
}

ProcessUser::~ProcessUser()
{
	delete virtualMachine;
}