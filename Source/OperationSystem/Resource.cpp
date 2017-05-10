#include <OperationSystem\Resource.h>
#include <OperationSystem\Process.h>
#include <OperationSystem\OperationSystem.h>
#include <OperationSystem\ResourcePlanner.h>

Resource::Resource(const char* name, Process* parent, OperationSystem* operationSystem) :
	name(name),
	parent(parent)
{
	resourcePlanner = operationSystem->Get_resourcePlanner();
	fid = operationSystem->GenerateUniqueFid();
}

Resource::~Resource() {}