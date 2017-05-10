#pragma once

#include <stdint.h>
#include <assert.h>

#define DIVIDE_WITH_FRACTION_ADDED(Value, Divider) Value / Divider + ((Value & Divider) == 0 ? 0 : 1)

#define CLAMP(Value, Min, Max) Value < Min ? Min : (Value > Max ? Max : Value)

#define VECTOR_ADD_ITEM(Vector, ItemName) \
	{ \
	assert(std::find(Vector.begin(), Vector.end(), ItemName) == Vector.end()); \
	Vector.push_back(ItemName); \
	} 

#define VECTOR_REMOVE_ITEM(Vector, ItemName) \
	{ \
	auto _##ItemName = std::find(Vector.begin(), Vector.end(), ItemName); \
	assert(_##ItemName != Vector.end()); \
	Vector.erase(_##ItemName); \
	}

#define VECTOR_SAFE_REMOVE_ITEM(Vector, ItemName) \
	{ \
	auto _##ItemName = std::find(Vector.begin(), Vector.end(), ItemName); \
	if (_##ItemName != Vector.end()) \
		Vector.erase(_##ItemName); \
	}

#define FOR_EACH(Vector, ItemName) \
	for (auto ItemName = Vector.begin(); ItemName != Vector.end(); ItemName++)

#define AUTOMATED_PROPERTY_GETPTR(Type, Name) \
	protected: Type Name; \
	public: inline Type* Get_##Name() { return &Name; } \
	protected:

#define AUTOMATED_PROPERTY_GETADR(Type, Name) \
	protected: Type Name; \
	public: inline Type& Get_##Name() { return Name; } \
	protected:

#define AUTOMATED_PROPERTY_GET(Type, Name) \
	protected: Type Name; \
	public: GETTER(Type, Name) \
	protected:

#define AUTOMATED_PROPERTY_GETSET(Type, Name) \
	protected: Type Name; \
	public: GETTER(Type, Name) SETTER(Type, Name) \
	protected:

#define SETTER(Type, Name) inline void Set_##Name(Type value) { Name = value; }
#define GETTER(Type, Name) inline Type Get_##Name() const { return Name; }

struct AddressRange
{
	uint32_t address;
	size_t size;
};

struct PointerRange
{
	void* pointer;
	size_t size;

public:
	PointerRange() { this->pointer = 0; this->size = 0; }
	PointerRange(void* pointer, size_t size) { this->pointer = pointer; this->size = size; }
};

class Helper
{
public:
	static uint32_t MemoryRangeToPages(size_t size, size_t pageSize)
	{
		return 0;
	}

	static uint32_t MemoryRangeToPages(uint32_t address, size_t size, size_t pageSize)
	{
		return 0;
	}
};