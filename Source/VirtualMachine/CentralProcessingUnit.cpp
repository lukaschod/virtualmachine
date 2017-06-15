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
	internalBuiltInDebug = false;
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

				//printf("%s\n", ram->ToString().c_str());
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
	auto instructionCode = GetNextInstruction();
	switch (instructionCode)
	{
	case kInstructionCodeNOP:
	{
		return;
	}

	case kInstructionCodeADD:
	{
		auto value1 = ExecuteInstructionPop();
		auto value2 = ExecuteInstructionPop();
		ExecuteInstructionPush(value1 + value2);
		return;
	}

	case kInstructionCodeINC:
	{
		auto value = ExecuteInstructionPop() + 1;
		ExecuteInstructionPush(value);
		return;
	}

	case kInstructionCodeDEC:
	{
		auto value = ExecuteInstructionPop() - 1;
		ExecuteInstructionPush(value);
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

	case kInstructionCodeCEQ:
	{
		auto value1 = ExecuteInstructionPop();
		auto value2 = ExecuteInstructionPop();
		ExecuteInstructionPush(value1 == value2);
		return;
	}

	case kInstructionCodeCNEQ:
	{
		auto value1 = ExecuteInstructionPop();
		auto value2 = ExecuteInstructionPop();
		ExecuteInstructionPush(value1 != value2);
		return;
	}

	case kInstructionCodeLDC:
	{
		auto index = GetNextInstruction();
		auto value = GetNextInstruction();
		ExecuteInstructionPush(value);
		return;
	}

	case kInstructionCodeLDA:
	{
		auto index = GetNextInstruction();
		auto address = ExecuteInstructionPop();
		uint32_t value = 0;
		mmu->ReadToRealMemory((CentralProcessingUnitCore*)this, address, &value, index);
		ExecuteInstructionPush(value);
		return;
	}

	case kInstructionCodeSTA:
	{
		auto index = GetNextInstruction();
		auto value = ExecuteInstructionPop();
		auto address = ExecuteInstructionPop();
		mmu->WriteFromRealMemory((CentralProcessingUnitCore*)this, &value, address, index);
		return;
	}

	case kInstructionCodeINT:
	{
		auto interuptCode = GetNextInstruction();
		SetInterupt((InteruptCode) interuptCode);
		return;
	}

	case kInstructionCodeBR:
	{
		auto address = GetNextInstruction();
		context.registerIC = address;
		return;
	}

	case kInstructionCodeBRFalse:
	{
		auto address = GetNextInstruction();
		auto condition = ExecuteInstructionPop();
		if (condition == 0)
			context.registerIC = address;
		return;
	}

	case kInstructionCodeBRTrue:
	{
		auto address = GetNextInstruction();
		auto condition = ExecuteInstructionPop();
		if (condition != 0)
			context.registerIC = address;
		return;
	}

	case kInstructionCodeHALT:
	{
		context.registerINT = kInteruptCodeHalt;
		context.registerC |= kFlagRegisterInterruptBit;
		return;
	}

	case kInstructionCodeCALL:
	{
		auto address = GetNextInstruction();
		auto argumentCount = GetNextInstruction();
		auto localCount = GetNextInstruction();
		auto argumentsAddress = context.registerSC - argumentCount * sizeof(uint32_t);
        ExecuteInstructionPush(argumentsAddress);
		ExecuteInstructionPush(context.registerIC);
		ExecuteInstructionPush(context.registerARG);
        ExecuteInstructionPush(context.registerLOC);
		context.registerARG = argumentsAddress;
		context.registerLOC = context.registerSC;
		context.registerSC += argumentCount * sizeof(uint32_t);
		context.registerIC = address;
		return;
	}

	case kInstructionCodeEND:
	{
		context.registerSC = context.registerLOC;
		context.registerLOC = ExecuteInstructionPop();
		context.registerARG = ExecuteInstructionPop();
		context.registerIC = ExecuteInstructionPop();
        context.registerSC = ExecuteInstructionPop();
		return;
	}

	case kInstructionCodeRET:
	{
		auto returnValue = ExecuteInstructionPop();
		context.registerSC = context.registerLOC;
		context.registerLOC = ExecuteInstructionPop();
		context.registerARG = ExecuteInstructionPop();
		context.registerIC = ExecuteInstructionPop();
        context.registerSC = ExecuteInstructionPop();
		ExecuteInstructionPush(returnValue);
		return;
	}

	case kInstructionCodeBREAK:
	{
		START_SUPERVISOR_MODE

		//printf("%s\n", ram->ToString().c_str());
		printf("%s\n", mmu->ToString((CentralProcessingUnitCore*)this).c_str());
		printf("%s\n", ToStringStackSegment().c_str());
		printf("%s\n", ToStringDataSegment().c_str());

		printf("%s\n", ToString().c_str());
		printf("\n<PRESS ANY KEY TO CONTINUE>\n");
		getchar();

		END_SUPERVISOR_MODE
		return;
	}

	case kInstructionCodeLDLOC:
	{
		auto index = GetNextInstruction();
		uint32_t value;
		mmu->ReadToRealMemory((CentralProcessingUnitCore*)this, context.registerLOC + index * sizeof(uint32_t), &value);
		ExecuteInstructionPush(value);
		return;
	}

	case kInstructionCodeSTLOC:
	{
		auto index = GetNextInstruction();
		auto value = ExecuteInstructionPop();
		mmu->WriteFromRealMemory((CentralProcessingUnitCore*)this, &value, context.registerLOC + index * sizeof(uint32_t));
		return;
	}

	case kInstructionCodeLDARG:
	{
		auto index = GetNextInstruction();
		uint32_t value;
		mmu->ReadToRealMemory((CentralProcessingUnitCore*)this, context.registerARG + index * sizeof(uint32_t), &value);
		ExecuteInstructionPush(value);
		return;
	}

	case kInstructionCodeSTARG:
	{
		auto index = GetNextInstruction();
		auto value = ExecuteInstructionPop();
		mmu->WriteFromRealMemory((CentralProcessingUnitCore*)this, &value, context.registerARG + index * sizeof(uint32_t));
		return;
	}

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
            auto pageEntryAddress = context.registerGeneral;
            static int physicalAddress = 0;
            mmu->AllocatePage((CentralProcessingUnitCore*)this, pageEntryAddress, physicalAddress);
            physicalAddress += ram->Get_pageSize();
			break;
		}

        case kInteruptCodeTimer:
        {
            context.registerTimer = 100;
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

uint32_t CentralProcessingUnit::ExecuteInstructionPop(uint32_t size)
{
	uint32_t currentStackValue;
	context.registerSC -= size;
	mmu->ReadToRealMemory((CentralProcessingUnitCore*)this, context.registerSC, &currentStackValue);
	return currentStackValue;
}

void CentralProcessingUnit::ExecuteInstructionPush(uint32_t value, uint32_t size)
{
	mmu->WriteFromRealMemory((CentralProcessingUnitCore*)this, &value, context.registerSC);
	context.registerSC += size;
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

	ExecuteInstructionTest();
	return true;
}