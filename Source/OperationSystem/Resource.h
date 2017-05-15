#pragma once

#include <VirtualMachine\Helper.h>
#include <OperationSystem\ResourcePlanner.h>
#include <vector>
#include <functional>

class ResourcePlanner;
class OperationSystem;
class Process;
class Resource;
class ResourceElement;
class CentralProcessingUnit;

struct ResourceRequest
{
	Process* requester;
	uint32_t count;
	uint32_t returnPoint;
	uint32_t requestIndex;

	ResourceRequest() { count = 1; requestIndex = 0; }
	ResourceRequest(uint32_t count) { this->count = count; requestIndex = 0; }
};

// TODO: Make resources generic
class ResourceElement
{
public:
	Process* receiver;
	Process* sender;
	uint32_t indexMode;
	uint32_t index2;
	uint32_t index3;
	uint32_t index4;
	uint32_t indexReturn;
	uint32_t indexError;
	uint32_t requestIndex;

protected:
	AUTOMATED_PROPERTY_GET(Resource*, resource);

public:
	ResourceElement(Resource* resource, Process* sender) :
		resource(resource),
		receiver(nullptr),
		sender(sender),
		requestIndex(0)
	{
	}

	bool IsMeetRequest(ResourceRequest& request)
	{
		return request.requestIndex == requestIndex;
	}
};

class Resource
{
public:
	Resource(const char* name, Process* parent, OperationSystem* operationSystem);
	virtual ~Resource();

protected:
	AUTOMATED_PROPERTY_GET(ResourcePlanner*, resourcePlanner);
	AUTOMATED_PROPERTY_GET(uint32_t, fid);
	AUTOMATED_PROPERTY_GET(const char*, name);
	AUTOMATED_PROPERTY_GETADR(std::vector<ResourceRequest>, requests);
	AUTOMATED_PROPERTY_GETADR(std::vector<ResourceElement*>, elements);
	AUTOMATED_PROPERTY_GET(Process*, parent);
};