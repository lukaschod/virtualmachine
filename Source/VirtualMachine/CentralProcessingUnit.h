#pragma once

#include <VirtualMachine\Helper.h>
#include <VirtualMachine\MemoryManagmentUnit.h>
#include <VirtualMachine\RandomAccessMemory.h>
#include <VirtualMachine\IInteruptHandler.h>
#include <VirtualMachine\Context.h>
#include <stdint.h>
#include <thread>

#define HAS_FLAG(flag1, flag2) (flag1 & flag2) != 0

class ExternalMemory;
class Input;
class Output;

enum FlagRegister
{
	kFlagRegisterNone = 0,
	kFlagRegisterInterruptBit = 2 << 0,
	kFlagRegisterEqualBit = 2 << 1,
	kFlagRegisterLessBit = 2 << 2,
};

struct Registers
{
public:
	// Instruction counter
	uint32_t registerIC;

	// Stack counter
	uint32_t registerSC;

	// Logic trigger flag
	uint32_t registerC;
	InteruptCode registerINT;

	uint32_t registerARG;
	uint32_t registerLOC;

	uint32_t registerTimer;

	bool registerUserMode;

	uint32_t registerGeneral;

	uint32_t registerPS; // Page segment
	uint32_t registerDS; // Data segment
	uint32_t registerSS; // Stack segment
	uint32_t registerCS; // Code segment
	uint32_t registerEN; // Indicate the end

public:
	Registers() { memset(this, 0, sizeof(Registers)); }
	bool IsInteruptHappened() { return (registerC & kFlagRegisterInterruptBit) != 0; }
	bool IsUserMode() { return registerUserMode; }
	void SetInterupt(InteruptCode code) { if (IsInteruptHappened()) return;  registerINT = code; registerC |= kFlagRegisterInterruptBit; }
};

class CentralProcessingUnit
{
public:
	CentralProcessingUnit(RandomAccessMemory* ram, ExternalMemory* externalMemory, Input* input, Output* output);

	void Start();
	void Stop();
	void WaitTillFinishes();

	void Run();

	void SetInterupt(InteruptCode code) { context.SetInterupt(code); }
	bool IsInteruptHappened() { return context.IsInteruptHappened(); }
	void ExecuteInstructionPush(uint32_t value, uint32_t size = 4);
	uint32_t ExecuteInstructionPop(uint32_t size = 4);
	bool HandleInterupts();
	
private:
	void ExecuteInstruction();
	void ExecuteInstructionTest();
	uint32_t GetNextInstruction();
	uint32_t AddressToValue(uint32_t address);

	bool IsInstruction(uint32_t instructionCode, const char* instructionName);

	std::string ToString();
	std::string ToStringStackSegment();
	std::string ToStringDataSegment();

public:
	inline bool IsStarted() const { return isStarted; }
	IMemory* Get_memory() { if (context.registerPS != 0) return mmu; else return ram; }

private:
	AUTOMATED_PROPERTY_GETSET(IInteruptHandler*, interuptHandler);
	AUTOMATED_PROPERTY_GETPTR(Registers, context);
	AUTOMATED_PROPERTY_GETSET(void*, process);
	AUTOMATED_PROPERTY_GET(Input*, input);
	AUTOMATED_PROPERTY_GET(Output*, output);
	AUTOMATED_PROPERTY_GET(ExternalMemory*, externalMemory);
	AUTOMATED_PROPERTY_GET(RandomAccessMemory*, ram);
	AUTOMATED_PROPERTY_GET(MemoryManagmentUnit*, mmu);
	std::thread* thread;
	bool isStarted;

	bool internalBuiltInDebug;

	AUTOMATED_PROPERTY_GETSET(Context*, physicalContext);
};

class CentralProcessingUnitCore : public CentralProcessingUnit
{
};

