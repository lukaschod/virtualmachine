#include <OperationSystem\ResourcePlanner.h>
#include <OperationSystem\Resource.h>
#include <OperationSystem\OperationSystem.h>
#include <OperationSystem\ProcessPlanner.h>
#include <OperationSystem\Process.h>
#include <OperationSystem\Processes\ProcessStartStop.h>

ResourcePlanner::ResourcePlanner(OperationSystem* operationSystem) :
	operationSystem(operationSystem)
{
	processPlanner = operationSystem->Get_processPlanner();
}

Resource* ResourcePlanner::CreateResourceOSStop(ProcessStartStop* parent)
{
	auto resource = new Resource("OSStop", parent, operationSystem);
	AddResource(resource, parent);
	return resource;
}

Resource* ResourcePlanner::CreateResourceExternalMemoryWait(ProcessStartStop* parent)
{
	auto resource = new Resource("ExternalMemoryWait", parent, operationSystem);
	AddResource(resource, parent);
	return resource;
}

Resource* ResourcePlanner::CreateResourceExternalMemoryRespond(ProcessStartStop* parent)
{
	auto resource = new Resource("ExternalMemoryRespond", parent, operationSystem);
	AddResource(resource, parent);
	return resource;
}

Resource* ResourcePlanner::CreateResourceProcessManagerWait(ProcessStartStop* parent)
{
	auto resource = new Resource("ProcessManagerWait", parent, operationSystem);
	AddResource(resource, parent);
	return resource;
}

Resource* ResourcePlanner::CreateResourceMemory(ProcessStartStop* parent, uint32_t pageCount, uint32_t pageSize)
{ 
	auto resource = new Resource("Memory", parent, operationSystem);
	AddResource(resource, parent);
	for (unsigned int i = 0; i < pageCount; i++)
	{
		auto element = new ResourceElement(resource, parent);
		element->indexReturn = i * pageSize;
		ProvideResourceElement(element, parent);
	}
	return resource;
}

void ResourcePlanner::AddResource(Resource* resource, Process* parent)
{
	VECTOR_ADD_ITEM(parent->Get_createdResources(), resource);
	VECTOR_ADD_ITEM(resources, resource);
}

void ResourcePlanner::DestroyResource(Resource* resource)
{
	auto& requests = resource->Get_requests();
	FOR_EACH(requests, itr)
	{
		auto request = *itr;
		processPlanner->UnblockProcess(request.requester);
	}

	VECTOR_REMOVE_ITEM(resource->Get_parent()->Get_createdResources(), resource);
	VECTOR_REMOVE_ITEM(resources, resource);
	delete resource;
}

void ResourcePlanner::ProvideResourceElementAsResponse(Resource* resource, Process* from, Process* to, ResourceResponds response, uint32_t index, uint32_t indexReturn)
{
	auto responseElement = new ResourceElement(resource, from);
	responseElement->requestIndex = to->Get_fid();
	responseElement->index = index;
	responseElement->indexReturn = indexReturn;
	responseElement->indexError = response;
	ProvideResourceElement(responseElement, from);
}

void ResourcePlanner::ProvideResourceElement(ResourceElement* element, Process* process)
{
	element->sender = process;
	VECTOR_SAFE_REMOVE_ITEM(process->Get_ownedResourceElements(), element);
	auto resource = element->Get_resource();
	auto& elements = resource->Get_elements();
	elements.push_back(element);
	Replan(resource);
}

void ResourcePlanner::DestroyResourceElement(ResourceElement* element, Process* process)
{
	VECTOR_SAFE_REMOVE_ITEM(process->Get_ownedResourceElements(), element);
	delete element;
}

bool ResourcePlanner::RequestResourceElementAny(Resource* resource, Process* process)
{
	ResourceRequest request;
	request.count = 1;
	request.requester = process;
	return RequestResourceElement(resource, request);
}

bool ResourcePlanner::RequestResourceElement(Resource* resource, ResourceRequest& request)
{
	auto process = request.requester;
	assert(process != nullptr);
	if (!CanAquire(resource, request))
	{
		resource->Get_requests().push_back(request);
		process->Set_waitingForResource(resource);
		processPlanner->BlockProcess(request.requester);
		return false;
	}

	Aquire(resource, request);
	Replan(resource);
	return true;
}

void ResourcePlanner::Aquire(Resource* resource, ResourceRequest& request)
{
	auto process = request.requester;
	auto& elements = resource->Get_elements();
	for (unsigned int i = 0; i < request.count; i++)
	{
		for (unsigned int j = 0; j < elements.size(); j++)
		{
			auto element = elements[j];
			if (element->IsMeetRequest(request))
			{
				element->receiver = process;
				VECTOR_ADD_ITEM(process->Get_ownedResourceElements(), element);
				elements.erase(elements.begin() + j);
				j--;
				break;
			}
		}
	}
}

bool ResourcePlanner::CanAquire(Resource* resource, ResourceRequest& request)
{
	auto& elements = resource->Get_elements();
	if (request.count > elements.size())
		return false;

	auto process = request.requester;
	unsigned int count = 0;

	for (auto itr = elements.begin(); itr != elements.end() && count < request.count; itr++)
	{
		auto element = *itr;
		if (element->IsMeetRequest(request))
			count++;
	}

	return count >= request.count;
}

void ResourcePlanner::Replan(Resource* resource)
{
	auto& requests = resource->Get_requests();
	for (unsigned int i = 0; i < requests.size(); i++)
	{
		auto request = requests[i];
		auto process = request.requester;
		if (CanAquire(resource, request))
		{
			Aquire(resource, request);
			process->Set_waitingForResource(nullptr);
			processPlanner->UnblockProcess(process);
			requests.erase(requests.begin() + i);
			i--;
		}
	}
}

void ResourcePlanner::DestroyAllRequestsFromProcess(Resource* resource, Process* process)
{
	auto& requests = resource->Get_requests();
	for (unsigned int i = 0; i < requests.size(); i++)
	{
		auto request = requests[i];
		if (request.requester == process)
		{
			requests.erase(requests.begin() + i);
			i--;
		}
	}
}