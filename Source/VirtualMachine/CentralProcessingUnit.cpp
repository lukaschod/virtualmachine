#include "CentralProcessingUnit.h"
#include "RandomAccessMemory.h"
#include "MemoryManagmentUnit.h"
#include "ExternalMemory.h"
#include "Input.h"
#include "Output.h"
#include "Code.h"
#include <string.h>
#include <assert.h>
#include <sstream>
#include <iomanip>

#define START_SUPERVISOR_MODE \
	auto cachedUserMode = context.registerUserMode; \
	context.registerUserMode = false;

#define END_SUPERVISOR_MODE \
	context.registerUserMode = cachedUserMode;

CentralProcessingUnit::CentralProcessingUnit(RandomAccessMemory* ram, ExternalMemory* externalMemory, Input* input, Output* output)
{
	assert(externalMemory != nullptr);
	assert(input != nullptr);
	assert(output != nullptr);
	this->ram = ram;
	this->mmu = new MemoryManagmentUnit(ram);
	this->externalMemory = externalMemory;
	this->input = input;
	this->output = output;
	this->interuptHandler = nullptr;
	isStarted = false;

	process = nullptr;
	internalBuiltInDebug = true;
}

void CentralProcessingUnit::Start()
{
	isStarted = true;
	thread = new std::thread(&CentralProcessingUnit::Run, this);
}

void CentralProcessingUnit::Run()
{
	if (physicalContext == nullptr)
		physicalContext = new Context();
	while (isStarted)
	{
		// In this implementation supervisor code is not executed by instructions, but by hard code.
		// For this reason we execute here all the queued supervisor code.
		if (interuptHandler != nullptr && interuptHandler->ShouldSkipNextInstruction((CentralProcessingUnitCore*)this))
			continue;

		if (context.registerTimer == 0)
		{
			SetInterupt(kInteruptCodeTimer);
		}
		else
		{
			context.registerTimer--;

			// This code is only for testing, as in real OS debugger is a process that controls other process
			if (internalBuiltInDebug)
			{
				START_SUPERVISOR_MODE

				printf("%s\n", ram->ToString().c_str());
				printf("%s\n", mmu->ToString((CentralProcessingUnitCore*)this).c_str());
				printf("%s\n", ToStringStackSegment().c_str());
				printf("%s\n", ToStringDataSegment().c_str());

				printf("%s\n", ToString().c_str());
				printf("\n<PRESS ANY KEY TO CONTINUE>\n");
				getchar();

				END_SUPERVISOR_MODE
			}

			// Here we execute next instruction in CPU
			ExecuteInstruction();
		}

		// Here we handle the interupts that happened during instruction execution
		ExecuteInstructionTest();
	}
	delete physicalContext;
}

void CentralProcessingUnit::WaitTillFinishes()
{
	thread->join();
}

void CentralProcessingUnit::Stop()
{
	isStarted = false;
}

void CentralProcessingUnit::ExecuteInstruction()
{
	context.registerLastIC = context.registerIC; // TODO: maybe find a better way to habdle pagefailure instruction rollback

	auto instructionCode = GetNextInstruction();
	switch (instructionCode)
	{
	case kInstructionCodeADD:
	{
		auto value1 = ExecuteInstructionPop();
		auto value2 = ExecuteInstructionPop();
		ExecuteInstructionPush(value1 + value2);
		return;
	}

	case kInstructionCodeSUB:
	{
		auto value1 = ExecuteInstructionPop();
		auto value2 = ExecuteInstructionPop();
		ExecuteInstructionPush(value1 - value2);
		return;
	}

	case kInstructionCodeMUL:
	{
		auto value1 = ExecuteInstructionPop();
		auto value2 = ExecuteInstructionPop();
		ExecuteInstructionPush(value1 * value2);
		return;
	}

	case kInstructionCodeDIV:
	{
		auto value1 = ExecuteInstructionPop();
		auto value2 = ExecuteInstructionPop();
		ExecuteInstructionPush(value1 / value2);
		return;
	}

	case kInstructionCodeAND:
	{
		auto value1 = ExecuteInstructionPop();
		auto value2 = ExecuteInstructionPop();
		ExecuteInstructionPush(value1 & value2);
		return;
	}

	case kInstructionCodeOR:
	{
		auto value1 = ExecuteInstructionPop();
		auto value2 = ExecuteInstructionPop();
		ExecuteInstructionPush(value1 | value2);
		return;
	}

	case kInstructionCodeCMP:
	{
		auto value1 = ExecuteInstructionPop();
		auto value2 = ExecuteInstructionPop();
		ExecuteInstructionPush(value1 - value2);
		return;
	}

	case kInstructionCodeLDC:
	{
		auto value = GetNextInstruction();
		ExecuteInstructionPush(value);
		return;
	}

	case kInstructionCodeLDI:
	{
		auto value = AddressToValue(GetNextInstruction());
		ExecuteInstructionPush(value);
		return;
	}

	case kInstructionCodeSTI:
	{
		auto address = GetNextInstruction();
		auto value = ExecuteInstructionPop();
		mmu->WriteFromRealMemory((CentralProcessingUnitCore*)this, &value, address);
		
		return;
	}

	case kInstructionCodeINT:
	{
		auto interuptCode = GetNextInstruction();
		SetInterupt((InteruptCode) interuptCode);
		return;
	}

	case kInstructionCodeJMP:
	{
		auto address = ExecuteInstructionPop();
		context.registerIC = address;
		return;
	}

	case kInstructionCodeJMPE:
	{
		auto flag = ExecuteInstructionPop();
		auto address = ExecuteInstructionPop();
		if (HAS_FLAG(context.registerC, kFlagRegisterEqualBit))
			context.registerIC = address;
		return;
	}

	case kInstructionCodeJMPL:
	{
		auto flag = ExecuteInstructionPop();
		auto address = ExecuteInstructionPop();
		if (HAS_FLAG(context.registerC, kFlagRegisterLessBit))
			context.registerIC = address;
		return;
	}

	case kInstructionCodeJMPEL:
	{
		auto flag = ExecuteInstructionPop();
		auto address = ExecuteInstructionPop();
		if (HAS_FLAG(context.registerC, kFlagRegisterEqualBit) || HAS_FLAG(context.registerC, kFlagRegisterLessBit))
			context.registerIC = address;
		return;
	}

	case kInstructionCodeHALT:
		context.registerINT = kInteruptCodeHalt;
		context.registerC |= kFlagRegisterInterruptBit;
		return;

	default:
		break;
	}
}

void CentralProcessingUnit::ExecuteInstructionTest()
{
	START_SUPERVISOR_MODE

	if (!context.IsInteruptHappened())
		return;

	context.registerC ^= kFlagRegisterInterruptBit;

	// Run overrided interupt handlers
	if (!interuptHandler || !interuptHandler->HandleInterupt((CentralProcessingUnitCore*)this))
	{
		switch (context.registerINT)
		{
		case kInteruptCodeInputReadUntilEnter:
		{
			auto address = ExecuteInstructionPop();
			input->ReadUntilEnter((CentralProcessingUnitCore*)this, address);
			break;
		}

		case kInteruptCodeOutputPrintToScreen:
		{
			auto address = ExecuteInstructionPop();
			output->PrintToScreen((CentralProcessingUnitCore*)this, address);
			break;
		}

		case kInteruptCodeExternalMemoryOpenFile:
		{
			auto accessFlag = ExecuteInstructionPop();
			auto filePathAddress = ExecuteInstructionPop();
			auto fileHandle = externalMemory->Open((CentralProcessingUnitCore*)this, filePathAddress, (FileAccessFlag) accessFlag);
			ExecuteInstructionPush(fileHandle);
			break;
		}

		case kInteruptCodeExternalMemoryCloseFile:
		{
			auto fileHandle = ExecuteInstructionPop();
			externalMemory->Close((CentralProcessingUnitCore*)this, fileHandle);
			break;
		}

		case kInteruptCodeExternalMemoryReadFile:
		{
			auto size = ExecuteInstructionPop();
			auto address = ExecuteInstructionPop();
			auto fileHandle = ExecuteInstructionPop();
			externalMemory->Read((CentralProcessingUnitCore*)this, fileHandle, address, size);
			break;
		}

		case kInteruptCodeExternalMemoryWriteFile:
		{
			auto size = ExecuteInstructionPop();
			auto address = ExecuteInstructionPop();
			auto fileHandle = ExecuteInstructionPop();
			externalMemory->Write((CentralProcessingUnitCore*)this, fileHandle, address, size);
			break;
		}

		case kInteruptCodeFailureGeneral:
		case kInteruptCodeFailureMemoryException:
		case kInteruptCodeHalt:
		{
			isStarted = false;
			break;
		}

		case kInteruptCodeFailurePage:
		{
			/*auto pageEntryAddress = ExecuteInstructionPop();
			auto address = ram->AllocateMemory(this, mmu->Get_pageSize());
			mmu->AllocatePage(this, pageEntryAddress, address);
			context.registerIC = context.registerLastIC; // Rollback the instructions*/
			break;
		}
		}
	}

	END_SUPERVISOR_MODE
}

bool CentralProcessingUnit::IsInstruction(uint32_t instructionCode, const char* instructionName)
{
	return memcmp((const char*)&instructionCode, instructionName, sizeof(char) * 4) == 0;
}

uint32_t CentralProcessingUnit::GetNextInstruction()
{
	uint32_t value;
	mmu->ReadToRealMemory((CentralProcessingUnitCore*)this, context.registerIC, &value);
	context.registerIC += 4;
	return value;
}

uint32_t CentralProcessingUnit::AddressToValue(uint32_t address)
{
	uint32_t value;
	mmu->ReadToRealMemory((CentralProcessingUnitCore*)this, address, &value);
	return value;
}

uint32_t CentralProcessingUnit::ExecuteInstructionPop()
{
	uint32_t currentStackValue;
	context.registerSC -= 4;
	mmu->ReadToRealMemory((CentralProcessingUnitCore*)this, context.registerSC, &currentStackValue);
	return currentStackValue;
}

void CentralProcessingUnit::ExecuteInstructionPush(uint32_t value)
{
	mmu->WriteFromRealMemory((CentralProcessingUnitCore*)this, &value, context.registerSC);
	context.registerSC += 4;
}

std::string CentralProcessingUnit::ToString()
{
	std::stringstream ss;
	ss << "Assembly" << std::endl;
	ss << std::right << std::setw(8 + 1) << "Address";
	ss << std::right << std::setw(4) << "Pos";
	ss << std::right << std::setw(16) << "Value";
	ss << std::endl;

	auto codeStart = context.registerCS;
	auto codeEnd = codeStart + (context.registerSS - context.registerCS);
	for (unsigned int i = codeStart; i < codeEnd; )
	{
		ss << std::right << std::setw(8) << std::setfill('0') << i << ":";

		uint32_t memory;
		mmu->ReadToRealMemory((CentralProcessingUnitCore*)this, i, &memory);
		ss << std::right << std::setw(4) << std::setfill(' ') << ((i == context.registerIC) ? " > " : "  ");
		ss << std::right << std::setw(16) << std::setfill(' ') << memory;
		ss << std::endl;
		i += sizeof(uint32_t);
	}

	return ss.str();
}

std::string CentralProcessingUnit::ToStringStackSegment()
{
	std::stringstream ss;
	ss << "Stack" << std::endl;
	ss << std::right << std::setw(8 + 1) << "Address";
	ss << std::right << std::setw(4) << "Pos";
	ss << std::right << std::setw(8) << "Value";
	ss << std::endl;

	auto codeStart = context.registerSS;
	auto codeEnd = codeStart + (context.registerEN - context.registerSS);
	for (unsigned int i = codeStart; i < codeEnd; )
	{
		ss << std::right << std::setw(8) << std::setfill('0') << i << ":";

		uint32_t memory;
		mmu->ReadToRealMemory((CentralProcessingUnitCore*)this, i, &memory);
		ss << std::right << std::setw(4) << std::setfill(' ') << ((i == context.registerSC) ? " > " : "  ");
		if (context.IsInteruptHappened())
		{
			ss << std::right << std::setw(8) << std::setfill(' ') << "?";
			context.registerC ^= kFlagRegisterInterruptBit;
		}
		else
		{
			ss << std::right << std::setw(8) << std::setfill(' ') << memory;
		}
		ss << std::endl;
		i += sizeof(uint32_t);
	}

	return ss.str();
}

std::string CentralProcessingUnit::ToStringDataSegment()
{
	std::stringstream ss;
	ss << "Data" << std::endl;
	ss << std::right << std::setw(8 + 1) << "Address";
	ss << std::right << std::setw(8) << "Value";
	ss << std::endl;

	auto codeStart = context.registerDS;
	auto codeEnd = codeStart + (context.registerCS - context.registerDS);
	for (unsigned int i = codeStart; i < codeEnd; )
	{
		ss << std::right << std::setw(8) << std::setfill('0') << i << ":";

		uint32_t memory;
		mmu->ReadToRealMemory((CentralProcessingUnitCore*)this, i, &memory);
		if (context.IsInteruptHappened())
		{
			ss << std::right << std::setw(8) << std::setfill(' ') << "?";
			ss << std::right << std::setw(8) << std::setfill(' ') << "?";
			ss << std::right << std::setw(8) << std::setfill(' ') << "?";
			ss << std::right << std::setw(8) << std::setfill(' ') << "?";
			context.registerC ^= kFlagRegisterInterruptBit;
		}
		else
		{
			ss << std::right << std::setw(8) << std::setfill(' ') << (memory & 255);
			ss << std::right << std::setw(8) << std::setfill(' ') << ((memory >> 8) & 255);
			ss << std::right << std::setw(8) << std::setfill(' ') << ((memory >> 16) & 255);
			ss << std::right << std::setw(8) << std::setfill(' ') << ((memory >> 24) & 255);
		}
		
		ss << std::endl;
		i += sizeof(uint32_t);
	}

	return ss.str();
}

bool CentralProcessingUnit::HandleInterupts()
{
	if (!context.IsInteruptHappened())
		return false;

	if (context.IsUserMode())
		return true;

	// On supervisor code we need to handle interupts immediatly
	// Because supervisor code is not executed here by instruction
	ExecuteInstructionTest();
	return true;
}