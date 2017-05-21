#include <OperationSystem\Process.h>
#include <OperationSystem\OperationSystem.h>
#include <OperationSystem\ResourcePlanner.h>
#include <OperationSystem\ProcessPlanner.h>
#include <OperationSystem\Resource.h>
#include <VirtualMachine\CentralProcessingUnit.h>

Process::Process(const char* name, Process* parent, ProcessPriority priority, OperationSystem* operationSystem) :
	operationSystem(operationSystem),
	parent(parent),
	state(ProcessState::kProcessStateReady),
	priorityCalculator(priority),
	processor(nullptr),
	waitingForResource(nullptr)
{
	this->name.Set(name);
	fid = operationSystem->GenerateUniqueFid();
	processPlanner = operationSystem->Get_processPlanner();
	resourcePlanner = operationSystem->Get_resourcePlanner();
}

Process::~Process()
{
}

void Process::CallbackReady(CentralProcessingUnitCore* processor) {}
void Process::CallbackRunning(CentralProcessingUnitCore* processor) {}
void Process::CallbackBlock() {}
void Process::CallbackUnblock() {}

uint32_t Process::GetRequestedResourceElementReturn()
{
	auto element = GetRequestedResourceElement();
	return element->indexReturn;
}

uint32_t Process::GetRequestedResourceElementError()
{
	auto element = GetRequestedResourceElement();
	return element->indexError;
}

ResourceElement* Process::GetRequestedResourceElement() { return ownedResourceElements.back(); }

bool Process::ExecuteCallback()
{
	assert(state == kProcessStateRunning);
	assert(processor != nullptr);

	if (queuedKernelInstructions.size() == 0)
		return false;

	auto callback = queuedKernelInstructions.front();
	callback(processor);
	queuedKernelInstructions.erase(queuedKernelInstructions.begin());
	

	return true;
}

void Process::ExecuteWhenRunning(ProcessKernelInstructions callback)
{ 
	assert(processor != nullptr);
	callback(processor);

	/*if (state == kProcessStateRunning)
	{
		assert(processor != nullptr);
		callback(processor);
		return;
	}

	queuedKernelInstructions.push_back(callback);*/
}
