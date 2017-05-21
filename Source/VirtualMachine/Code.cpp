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
	if (!CompileDataSegment(source, size))
		return false;

	// Pre-compile labels
	if (!CompileCodeSegment(source, size))
		return false;
	codeSegment.clear();

	if (!CompileCodeSegment(source, size))
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

bool Program::CompileDataSegment(const char* source, size_t size)
{
	const char* sourceEnd = source + size;
	size_t instructionSize = 4;
	while (source <= sourceEnd)
	{
		if (errors.size() != 0)
			return false;

		if (MovePointerIfComment(source))
			continue;

		// Read Heap
		if (MovePointerIfSame(source, ".data"))
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
					source++;

					// Handle special symbols here
					if (value == '\\')
					{
						uint8_t nextValue = *source;
						source++;

						switch (nextValue)
						{
						case 'n':
							value = '\n';
						}
					}

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

bool Program::CompileCodeSegment(const char* source, size_t size)
{
	const char* sourceEnd = source + size;
	size_t instructionSize = 4;
	while (source <= sourceEnd)
	{
		if (errors.size() != 0)
			return false;

		// Skip data segments as its handled in data compilation
		if (MovePointerIfSame(source, ".data"))
		{
			MovePointerToEndOfLine(source);
			continue;
		}

		if (MovePointerIfComment(source))
			continue;

		if (CompileArithmeticInstructions(source))
			continue;

		if (CompileDataManipulationInstructions(source))
			continue;

		if (CompileInteruptInstructions(source))
			continue;

		if (MovePointerIfSame(source, "halt"))
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
	if (MovePointerIfSame(source, "nop"))
	{
		codeSegment.push_back(kInstructionCodeNOP);
		return true;
	}

	if (MovePointerIfSame(source, "add"))
	{
		codeSegment.push_back(kInstructionCodeADD);
		return true;
	}

	if (MovePointerIfSame(source, "inc"))
	{
		codeSegment.push_back(kInstructionCodeINC);
		return true;
	}

	if (MovePointerIfSame(source, "dec"))
	{
		codeSegment.push_back(kInstructionCodeDEC);
		return true;
	}

	if (MovePointerIfSame(source, "sub"))
	{
		codeSegment.push_back(kInstructionCodeSUB);
		return true;
	}

	if (MovePointerIfSame(source, "mul"))
	{
		codeSegment.push_back(kInstructionCodeMUL);
		return true;
	}

	if (MovePointerIfSame(source, "div"))
	{
		codeSegment.push_back(kInstructionCodeDIV);
		return true;
	}

	if (MovePointerIfSame(source, "and"))
	{
		codeSegment.push_back(kInstructionCodeAND);
		return true;
	}

	if (MovePointerIfSame(source, "or"))
	{
		codeSegment.push_back(kInstructionCodeOR);
		return true;
	}

	if (MovePointerIfSame(source, "ceq"))
	{
		codeSegment.push_back(kInstructionCodeCEQ);
		return true;
	}

	if (MovePointerIfSame(source, "cneq"))
	{
		codeSegment.push_back(kInstructionCodeCNEQ);
		return true;
	}

	return false;
}

void Program::ReportError(const char* message)
{
	assert(false);
	ProgramCompileError error;
	error.message = message;
	errors.push_back(error);
}

bool Program::CompileDataManipulationInstructions(const char*& source)
{
	if (MovePointerIfSame(source, "ldarg.", false))
	{
		uint32_t index;
		if (!MovePointerIfReadedUint32(source, index))
		{
			ReportError("Expected local argument index.");
			return false;
		}

		codeSegment.push_back(kInstructionCodeLDARG);
		codeSegment.push_back(index);
		return false;
	}

	if (MovePointerIfSame(source, "starg.", false))
	{
		uint32_t index;
		if (!MovePointerIfReadedUint32(source, index))
		{
			ReportError("Expected local argument index.");
			return false;
		}

		codeSegment.push_back(kInstructionCodeSTARG);
		codeSegment.push_back(index);
		return false;
	}

	if (MovePointerIfSame(source, "ldloc.", false))
	{
		uint32_t index;
		if (!MovePointerIfReadedUint32(source, index))
		{
			ReportError("Expected local argument index.");
			return false;
		}

		codeSegment.push_back(kInstructionCodeLDLOC);
		codeSegment.push_back(index);
		return false;
	}

	if (MovePointerIfSame(source, "stloc.", false))
	{
		uint32_t index;
		if (!MovePointerIfReadedUint32(source, index))
		{
			ReportError("Expected local argument index.");
			return false;
		}

		codeSegment.push_back(kInstructionCodeSTLOC);
		codeSegment.push_back(index);
		return false;
	}

	if (MovePointerIfSame(source, "ldc.i", false))
	{
		uint32_t index;
		if (!MovePointerIfReadedUint32(source, index))
		{
			ReportError("Expected local argument index.");
			return false;
		}

		uint32_t value;
		if (!MovePointerIfReadedUint32OrLabel(source, value))
		{
			ReportError("Expected local argument index.");
			return false;
		}

		codeSegment.push_back(kInstructionCodeLDC);
		codeSegment.push_back(index);
		codeSegment.push_back(value);

		return true;
	}

	if (MovePointerIfSame(source, "lda.i", false))
	{
		uint32_t index;
		if (!MovePointerIfReadedUint32(source, index))
		{
			ReportError("Expected local argument index.");
			return false;
		}

		codeSegment.push_back(kInstructionCodeLDA);
		codeSegment.push_back(index);

		return true;
	}

	if (MovePointerIfSame(source, "sta.i", false))
	{
		uint32_t index;
		if (!MovePointerIfReadedUint32(source, index))
		{
			ReportError("Expected local argument index.");
			return false;
		}

		codeSegment.push_back(kInstructionCodeSTA);
		codeSegment.push_back(index);

		return true;
	}

	return false;
}

bool Program::CompileInteruptInstructions(const char*& source)
{
	if (MovePointerIfSame(source, "func"))
	{
		Procedure procedure;
		if (!MovePointerIfReadedLabel(source, procedure.label))
		{
			ReportError("Expected name of function.");
			return false;
		}
		if (!MovePointerIfReadedUint32(source, procedure.argumentCount))
		{
			ReportError("Expected argument count.");
			return false;
		}
		if (!MovePointerIfReadedUint32(source, procedure.localCount))
		{
			ReportError("Expected local count.");
			return false;
		}
		procedure.address = GetDataSegmentSize() + GetCodeSegmentSize();

		procedures.push_back(procedure);

		return true;
	}

	if (MovePointerIfSame(source, "end"))
	{
		codeSegment.push_back(kInstructionCodeEND);
		return true;
	}

	if (MovePointerIfSame(source, "ret"))
	{
		codeSegment.push_back(kInstructionCodeRET);
		return true;
	}

	if (MovePointerIfSame(source, "call"))
	{
		char label[MAX_LABEL_SIZE];
		if (!MovePointerIfReadedLabel(source, label))
		{
			ReportError("Expected name of function.");
			return false;
		}

		Procedure procedure;
		LabelToProcedure(label, procedure);

		codeSegment.push_back(kInstructionCodeCALL);
		codeSegment.push_back(procedure.address);
		codeSegment.push_back(procedure.argumentCount);
		codeSegment.push_back(procedure.localCount);

		return true;
	}

	if (MovePointerIfSame(source, ".label"))
	{
		char label[MAX_LABEL_SIZE];
		if (!MovePointerIfReadedLabel(source, label))
		{
			ReportError("Expected name of label.");
			return false;
		}

		UpdateLabelAddress(label, GetDataSegmentSize() + GetCodeSegmentSize());

		return true;
	}

	if (MovePointerIfSame(source, "int"))
	{
		uint32_t interuptId;
		if (!MovePointerIfReadedUint32(source, interuptId))
		{
			ReportError("Expected interupt index.");
			return false;
		}

		codeSegment.push_back(kInstructionCodeINT);
		codeSegment.push_back(interuptId);

		return true;
	}

	if (MovePointerIfSame(source, "br"))
	{
		uint32_t address;
		if (!MovePointerIfReadedUint32OrLabel(source, address))
		{
			ReportError("Expected address.");
			return false;
		}

		codeSegment.push_back(kInstructionCodeBR);
		codeSegment.push_back(address);

		return true;
	}

	if (MovePointerIfSame(source, "brtrue"))
	{
		uint32_t address;
		if (!MovePointerIfReadedUint32OrLabel(source, address))
		{
			ReportError("Expected address.");
			return false;
		}

		codeSegment.push_back(kInstructionCodeBRTrue);
		codeSegment.push_back(address);
		return true;
	}

	if (MovePointerIfSame(source, "brfalse"))
	{
		uint32_t address;
		if (!MovePointerIfReadedUint32OrLabel(source, address))
		{
			ReportError("Expected address.");
			return false;
		}

		codeSegment.push_back(kInstructionCodeBRFalse);
		codeSegment.push_back(address);
		return true;
	}

	if (MovePointerIfSame(source, "break"))
	{
		codeSegment.push_back(kInstructionCodeBREAK);
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

bool Program::LabelToProcedure(char* label, Procedure& procedure)
{
	for (auto itr = procedures.begin(); itr < procedures.end(); itr++)
	{
		if (strcmp(itr->label, label) == 0)
		{
			procedure = *itr;
			return true;
		}
	}
	return false;
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

		if (*source == '\n' || *source == '\r')
			return false;
		source++;
	}
}

void Program::MovePointerToEndOfLine(const char*& source)
{
	while (*source != '\n' && *source != '\r')
	{
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
	while (*currentSourcePointer == ' ' || *currentSourcePointer == '\n' || *currentSourcePointer == '\r')
	{
		if (*currentSourcePointer == '\n' || *currentSourcePointer == '\r')
			return false;
		currentSourcePointer++;
	}
	auto labelStartPointer = currentSourcePointer;

	while (*currentSourcePointer != ' ' && *currentSourcePointer != '\n' && *currentSourcePointer != '\r')
	{
		currentSourcePointer++;
	}
	auto labelEndPointer = currentSourcePointer;
	auto labelSize = labelEndPointer - labelStartPointer;

	if (labelSize + 1 > MAX_LABEL_SIZE)
	{
		ReportError("Exceeds label size limit.");
		return false;
	}

	source = labelEndPointer;
	memcpy(label, labelStartPointer, labelSize);
	label[labelSize] = 0;

	return true;
}

bool Program::MovePointerIfComment(const char*& source)
{
	auto currentSourcePointer = source;
	while (*currentSourcePointer == ' ' || *currentSourcePointer == '	' || *currentSourcePointer == '\n' || *currentSourcePointer == '\r')
	{
		currentSourcePointer++;
	}

	if (*currentSourcePointer != '/' || *(++currentSourcePointer) != '/')
		return false;

	while (*currentSourcePointer != '\n' && *currentSourcePointer != '\r')
	{
		currentSourcePointer++;
	}

	source = currentSourcePointer;
	return true;
}

bool Program::MovePointerIfSame(const char*& source, const char* text, bool asSeperateWord)
{
	auto currentSourcePointer = source;
	while (*currentSourcePointer == ' ' || *currentSourcePointer == '	' || *currentSourcePointer == '\n' || *currentSourcePointer == '\r')
	{
		currentSourcePointer++;
	}

	auto currentTextPointer = text;
	while (true)
	{
		if (*currentTextPointer == '\0' && 
			(!asSeperateWord || *currentSourcePointer == ' ' || *currentSourcePointer == '\n' || *currentSourcePointer == '\r' || *currentSourcePointer == 0))
			break;
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
	while (*currentSourcePointer == ' ' || *currentSourcePointer == '\n' || *currentSourcePointer == '\r')
	{
		currentSourcePointer++;
	}

	if (sscanf_s(currentSourcePointer, "%d", &number) != 1)
	{
		return false;
	}

	while (*currentSourcePointer != ',' && *currentSourcePointer != ' ' && *currentSourcePointer != '\n' && *currentSourcePointer != '\r')
	{
		currentSourcePointer++;
	}

	source = currentSourcePointer;
	return true;
}

bool Program::MovePointerIfReadedUint32OrLabel(const char*& source, uint32_t& number)
{
	auto currentSourcePointer = source;
	while (*currentSourcePointer == ' ' || *currentSourcePointer == '\n' || *currentSourcePointer == '\r')
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

	while (*currentSourcePointer != ',' && *currentSourcePointer != ' ' && *currentSourcePointer != '\n' && *currentSourcePointer != '\r')
	{
		currentSourcePointer++;
	}

	source = currentSourcePointer;
	return true;
}