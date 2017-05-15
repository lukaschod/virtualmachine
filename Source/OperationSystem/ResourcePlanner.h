#pragma once

#include <VirtualMachine\Helper.h>
#include <vector>

class OperationSystem;
class ProcessPlanner;
class Process;
class ProcessStartStop;
class Resource;
class ResourceElement;
struct ResourceRequest;

enum ResourceResponds
{
	kResourceRespondSuccess,
	kResourceRespondError,
};

class ResourcePlanner
{
public:
	ResourcePlanner(OperationSystem* operationSystem);

	Resource* CreateResourceMemory(ProcessStartStop* parent, uint32_t pageCount, uint32_t pageSize);
	Resource* CreateResourceRequest(ProcessStartStop* parent, const char* name);
	Resource* CreateResourceRespond(ProcessStartStop* parent, const char* name);

	void DestroyResource(Resource* resource);

	bool RequestResourceElement(Resource* resource, ResourceRequest& request);
	bool RequestResourceElementAny(Resource* resource, Process* process);
	void ProvideResourceElement(ResourceElement* element, Process* process);
	void ProvideResourceElementAsResponse(Resource* resource, Process* from, Process* to, ResourceResponds response, uint32_t index, uint32_t indexReturn = 0);
	void DestroyResourceElement(ResourceElement* element, Process* process);
	void DestroyAllRequestsFromProcess(Resource* resource, Process* process);

private:
	bool CanAquire(Resource* resource, ResourceRequest& request);
	void Aquire(Resource* resource, ResourceRequest& request);
	void Replan(Resource* resource);
	void AddResource(Resource* resource, Process* parent);

private:
	AUTOMATED_PROPERTY_GET(std::vector<Resource*>, resources);
	OperationSystem* operationSystem;
	AUTOMATED_PROPERTY_GETSET(ProcessPlanner*, processPlanner);
};