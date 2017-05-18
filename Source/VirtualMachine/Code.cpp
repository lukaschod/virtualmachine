#include "Code.h"
#include <VirtualMachine\CentralProcessingUnit.h>
#include <string.h>

Program::Program()
{

}

bool Program::CreateFromMemory(CentralProcessingUnitCore* core, uint32_t address, uint32_t size)
{
	auto memory = core->Get_memory();

	memory->ReadToRealMemory(core, address, &header, sizeof(ProgramHeader));
	address += sizeof(ProgramHeader);

	if (header.codeSegmentSize != 0)
	{
		codeSegment.resize(header.codeSegmentSize / sizeof(uint32_t));
		memory->ReadToRealMemory(core, address, codeSegment.data(), header.codeSegmentSize);
		address += header.codeSegmentSize;
	}

	if (header.dataSegmentSize != 0)
	{
		dataSegment.resize(header.dataSegmentSize);
		memory->ReadToRealMemory(core, address, dataSegment.data(), header.dataSegmentSize);
		address += header.dataSegmentSize;
	}

	// TODO: Add validation

	return true;
}

bool Program::SaveToMemory(CentralProcessingUnitCore* core, uint32_t address)
{
	auto memory = core->Get_memory();

	memory->WriteFromRealMemory(core, &header, address, sizeof(ProgramHeader));
	address += sizeof(ProgramHeader);

	if (header.codeSegmentSize != 0)
	{
		memory->WriteFromRealMemory(core, codeSegment.data(), address, header.codeSegmentSize);
		address += header.codeSegmentSize;
	}
	
	if (header.dataSegmentSize != 0)
	{
		memory->WriteFromRealMemory(core, dataSegment.data(), address, header.dataSegmentSize);
		address += header.dataSegmentSize;
	}

	// TODO: Add validation

	return true;
}

bool Program::CreateFromText(const char* source, size_t size)
{
	if (!CompileData(source, size))
		return false;

	// Prebakes the labels
	if (!CompileInternal(source, size)) // TODO: optimize
		return false;
	codeSegment.clear();

	if (!CompileInternal(source, size))
		return false;

	header.codeSegmentSize = sizeof(uint32_t) * codeSegment.size();
	header.dataSegmentSize = sizeof(uint8_t) * dataSegment.size();
	header.stackSegmentSize = sizeof(uint32_t) * 30;

	return true;
}

bool Program::CompileFromMemoryAndCreate(CentralProcessingUnitCore* core, uint32_t address, size_t size)
{
	auto memory = core->Get_memory();
	auto range = memory->AddressToPointerRange(core, address, size);
	return CreateFromText((const char*)range.pointer, range.size);
}

bool Program::CompileData(const char* source, size_t size)
{
	const char* sourceEnd = source + size;
	size_t instructionSize = 4;
	while (source != sourceEnd)
	{
		// Read Heap
		if (MovePointerIfSame(source, "DATA"))
		{
			char label[MAX_LABEL_SIZE];
			if (!MovePointerIfReadedLabel(source, label))
				return false;

			UpdateLabelAddress(label, GetDataSegmentSize());

			// Read as uint8_t char array
			if (MovePointerIfReadedStringSymbol(source))
			{
				while (true)
				{
					if (MovePointerIfReadedStringSymbol(source))
						break;

					uint8_t value = *source;
					//if (sscanf_s(source, "%c", &value) != 1)
					//	return false;
					source++;

					dataSegment.push_back(value);
				}

				dataSegment.push_back(0);
				continue;
			}

			// Read as uint32_t array
			while (true)
			{
				uint32_t value;
				if (!MovePointerIfReadedUint32OrLabel(source, value))
					return false;

				dataSegment.push_back(value);
				dataSegment.push_back(value << 8);
				dataSegment.push_back(value << 16);
				dataSegment.push_back(value << 24);

				if (!IsNextSeperator(source))
					break;
			}

			continue;
		}

		source++;
		continue;
	}

	return true;
}

bool Program::CompileInternal(const char* source, size_t size)
{
	const char* sourceEnd = source + size;
	size_t instructionSize = 4;
	while (source != sourceEnd)
	{
		if (MovePointerIfSame(source, "PROC"))
		{
			char label[MAX_LABEL_SIZE];
			if (!MovePointerIfReadedLabel(source, label))
				return false;

			UpdateLabelAddress(label, GetDataSegmentSize() + GetCodeSegmentSize());

			continue;
		}

		if (MovePointerIfSame(source, "LDARG."))
		{
			uint32_t index;
			if (!MovePointerIfReadedUint32(source, index))
				return false;

			codeSegment.push_back(kInstructionCodeLDARG);
			codeSegment.push_back(index);
			continue;
		}

		if (MovePointerIfSame(source, "STARG."))
		{
			uint32_t index;
			if (!MovePointerIfReadedUint32(source, index))
				return false;

			codeSegment.push_back(kInstructionCodeSTARG);
			codeSegment.push_back(index);
			continue;
		}

		if (MovePointerIfSame(source, "LDLOC."))
		{
			uint32_t index;
			if (!MovePointerIfReadedUint32(source, index))
				return false;

			codeSegment.push_back(kInstructionCodeLDLOC);
			codeSegment.push_back(index);
			continue;
		}

		if (MovePointerIfSame(source, "STLOC."))
		{
			uint32_t index;
			if (!MovePointerIfReadedUint32(source, index))
				return false;

			codeSegment.push_back(kInstructionCodeSTLOC);
			codeSegment.push_back(index);
			continue;
		}

		if (MovePointerIfSame(source, "RET"))
		{
			codeSegment.push_back(kInstructionCodeRET);
			continue;
		}

		if (MovePointerIfSame(source, "CALL"))
		{
			char label[MAX_LABEL_SIZE];
			if (!MovePointerIfReadedLabel(source, label))
				break;

			uint32_t address;
			LabelToAddress(label, address);

			uint32_t argumentCount;
			if (!MovePointerIfReadedUint32(source, argumentCount))
				return false;

			uint32_t localCount;
			if (!MovePointerIfReadedUint32(source, localCount))
				return false;

			codeSegment.push_back(kInstructionCodeCALL);
			codeSegment.push_back(address);
			codeSegment.push_back(argumentCount);
			codeSegment.push_back(localCount);

			continue;
		}

		if (MovePointerIfSame(source, "#LABEL"))
		{
			char label[MAX_LABEL_SIZE];
			if (!MovePointerIfReadedLabel(source, label))
				return false;

			UpdateLabelAddress(label, GetDataSegmentSize() + GetCodeSegmentSize());

			continue;
		}

		if (CompileArithmeticInstructions(source))
			continue;

		if (CompileDataManipulationInstructions(source))
			continue;

		if (CompileInteruptInstructions(source))
			continue;

		if (MovePointerIfSame(source, "HALT"))
		{
			codeSegment.push_back(kInstructionCodeHALT);
			continue;
		}

		source++;
		continue;
	}

	return true;
}

bool Program::CompileArithmeticInstructions(const char*& source)
{
	if (MovePointerIfSame(source, "ADD"))
	{
		codeSegment.push_back(kInstructionCodeADD);
		return true;
	}

	if (MovePointerIfSame(source, "SUB"))
	{
		codeSegment.push_back(kInstructionCodeSUB);
		return true;
	}

	if (MovePointerIfSame(source, "MUL"))
	{
		codeSegment.push_back(kInstructionCodeMUL);
		return true;
	}

	if (MovePointerIfSame(source, "DIV"))
	{
		codeSegment.push_back(kInstructionCodeDIV);
		return true;
	}

	if (MovePointerIfSame(source, "AND"))
	{
		codeSegment.push_back(kInstructionCodeAND);
		return true;
	}

	if (MovePointerIfSame(source, "OR"))
	{
		codeSegment.push_back(kInstructionCodeOR);
		return true;
	}

	if (MovePointerIfSame(source, "CMP"))
	{
		codeSegment.push_back(kInstructionCodeCMP);
		return true;
	}

	return false;
}

bool Program::CompileDataManipulationInstructions(const char*& source)
{
	if (MovePointerIfSame(source, "LDC"))
	{
		uint32_t value;
		if (MovePointerIfReadedUint32OrLabel(source, value))
		{
			codeSegment.push_back(kInstructionCodeLDC);
			codeSegment.push_back(value);
			return true;
		}

		return false;
	}

	if (MovePointerIfSame(source, "LDI"))
	{
		uint32_t value;
		if (MovePointerIfReadedUint32OrLabel(source, value))
		{
			codeSegment.push_back(kInstructionCodeLDI);
			codeSegment.push_back(value);
			return true;
		}

		return false;
	}

	if (MovePointerIfSame(source, "STI"))
	{
		uint32_t value;
		if (MovePointerIfReadedUint32OrLabel(source, value))
		{
			codeSegment.push_back(kInstructionCodeSTI);
			codeSegment.push_back(value);
			return true;
		}

		return false;
	}

	return false;
}

bool Program::CompileInteruptInstructions(const char*& source)
{
	if (MovePointerIfSame(source, "INT"))
	{
		uint32_t interuptId;
		if (!MovePointerIfReadedUint32OrLabel(source, interuptId))
			return false;

		codeSegment.push_back(kInstructionCodeINT);
		codeSegment.push_back(interuptId);

		return true;
	}

	if (MovePointerIfSame(source, "JMP"))
	{
		codeSegment.push_back(kInstructionCodeJMP);
		return true;
	}

	if (MovePointerIfSame(source, "JMPE"))
	{
		codeSegment.push_back(kInstructionCodeJMPE);
		return true;
	}

	if (MovePointerIfSame(source, "JMPL"))
	{
		codeSegment.push_back(kInstructionCodeJMPL);
		return true;
	}

	if (MovePointerIfSame(source, "JMPEL"))
	{
		codeSegment.push_back(kInstructionCodeJMPEL);
		return true;
	}

	return false;
}

void Program::UpdateLabelAddress(char* label, uint32_t address)
{
	for (auto itr = labelToAddress.begin(); itr < labelToAddress.end(); itr++)
	{
		if (strcmp(itr->first, label) == 0)
		{
			itr->second = address;
			return;
		}
	}

	std::pair<char[MAX_LABEL_SIZE], uint32_t> labelToAddress;
	memcpy(labelToAddress.first, label, MAX_LABEL_SIZE);
	labelToAddress.second = address;
	this->labelToAddress.push_back(labelToAddress);
}

bool Program::LabelToAddress(char* label, uint32_t& address)
{
	for (auto itr = labelToAddress.begin(); itr < labelToAddress.end(); itr++)
	{
		if (strcmp(itr->first, label) == 0)
		{
			address = itr->second;
			return true;
		}
	}

	return false;
}

bool Program::IsNextSeperator(const char*& source)
{
	while (true)
	{
		if (*source == ',')
		{
			source++;
			return true;
		}

		if (*source == '\n')
			return false;
		source++;
	}
}

bool Program::MovePointerIfReadedStringSymbol(const char*& source)
{
	auto currentSourcePointer = source;
	while (true)
	{
		if (*currentSourcePointer == '"')
		{
			source = currentSourcePointer + 1;
			return true;
		}

		if (*currentSourcePointer != ' ')
			break;

		currentSourcePointer++;
	}
	return false;
}

bool Program::MovePointerIfReadedLabel(const char*& source, char* label)
{
	auto currentSourcePointer = source;
	while (*currentSourcePointer == ' ' || *currentSourcePointer == '\n')
	{
		if (*currentSourcePointer == '\n')
			return false;
		currentSourcePointer++;
	}
	auto labelStartPointer = currentSourcePointer;

	while (*currentSourcePointer != ' ' && *currentSourcePointer != '\n')
	{
		currentSourcePointer++;
	}
	auto labelEndPointer = currentSourcePointer;
	auto labelSize = labelEndPointer - labelStartPointer;

	if (labelSize + 1 > MAX_LABEL_SIZE)
	{
		// TODO: EXCEPTION
		return false;
	}

	source = labelEndPointer;
	memcpy(label, labelStartPointer, labelSize);
	label[labelSize] = 0;

	return true;
}

bool Program::MovePointerIfSame(const char*& source, const char* text)
{
	auto currentSourcePointer = source;
	while (*currentSourcePointer == ' ' || *currentSourcePointer == '	' || *currentSourcePointer == '\n')
	{
		currentSourcePointer++;
	}

	auto currentTextPointer = text;
	while (*currentTextPointer != '\0')
	{
		if (*currentSourcePointer != *currentTextPointer)
			return false;
		currentSourcePointer++;
		currentTextPointer++;
	}

	source = currentSourcePointer;
	return true;
}

bool Program::MovePointerIfReadedUint32(const char*& source, uint32_t& number)
{
	auto currentSourcePointer = source;
	while (*currentSourcePointer == ' ' || *currentSourcePointer == '\n')
	{
		currentSourcePointer++;
	}

	if (sscanf_s(currentSourcePointer, "%d", &number) != 1)
	{
		return false;
	}

	while (*currentSourcePointer != ',' && *currentSourcePointer != ' ' && *currentSourcePointer != '\n')
	{
		currentSourcePointer++;
	}

	source = currentSourcePointer;
	return true;
}

bool Program::MovePointerIfReadedUint32OrLabel(const char*& source, uint32_t& number)
{
	auto currentSourcePointer = source;
	while (*currentSourcePointer == ' ' || *currentSourcePointer == '\n')
	{
		currentSourcePointer++;
	}

	if (sscanf_s(currentSourcePointer, "%d", &number) != 1)
	{
		// Lets check maybe next number is actually a label
		char label[MAX_LABEL_SIZE];
		if (MovePointerIfReadedLabel(source, label))
		{
			uint32_t address;
			if (!LabelToAddress(label, address))
				return true;

			number = address;
			return true;
		}
	}

	while (*currentSourcePointer != ',' && *currentSourcePointer != ' ' && *currentSourcePointer != '\n')
	{
		currentSourcePointer++;
	}

	source = currentSourcePointer;
	return true;
}