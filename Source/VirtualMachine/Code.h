#pragma once

#include "Helper.h"
#include <vector>

class RealMachine;
class CentralProcessingUnitCore;

enum InstructionCode
{
	ADD,
	SUB,
	MUL,
	DIV,
	AND,
	OR,
	CMP,

	LDC,
	LDI,
	STI,

	INT,
	HALT,
	JMP,
	JMPE,
	JMPL,
	JMPEL
};

enum InteruptCode
{
	kInteruptCodeInputReadUntilEnter,
	kInteruptCodeOutputPrintToScreen,
	kInteruptCodeExternalMemoryOpenFile,
	kInteruptCodeExternalMemoryCloseFile,
	kInteruptCodeExternalMemoryReadFile,
	kInteruptCodeExternalMemoryWriteFile,
	kInteruptCodeTimer,
	kInteruptCodeHalt,
	kInteruptExecuteKernelInstructions,

	kInteruptCodeFailureMemoryException,
	kInteruptCodeFailureGeneral,
	kInteruptCodeFailurePage,
};

#define MAX_LABEL_SIZE 30

struct ProgramHeader
{
	uint32_t codeSegmentSize;
	uint32_t dataSegmentSize;
	uint32_t stackSegmentSize;
};

class Program
{
public:
	Program();

	bool CreateFromText(const char* source);
	bool CreateFromMemory(CentralProcessingUnitCore* core, uint32_t address, uint32_t size);
	bool SaveToMemory(CentralProcessingUnitCore* core, uint32_t address);

private:
	bool CompileInternal(const char* source);
	bool CompileArithmeticInstructions(const char*& source);
	bool CompileDataManipulationInstructions(const char*& source);
	bool CompileInteruptInstructions(const char*& source);

	bool MovePointerIfSame(const char*& source, const char* text);
	bool MovePointerIfReadedUint32(const char*& source, uint32_t& number);
	bool MovePointerIfReadedStringSymbol(const char*& source);
	bool MovePointerIfReadedLabel(const char*& source, char* label);
	bool IsNextSeperator(const char*& source);
	void UpdateLabelAddress(char* label, uint32_t address);
	bool LabelToAddress(char* label, uint32_t& address);

public:
	inline void* GetDataSegment() { return dataSegment.data(); }
	inline size_t GetDataSegmentSize() { return sizeof(uint8_t) * dataSegment.size(); }

	inline size_t GetStackSegmentSize() { return header.stackSegmentSize; }

	inline void* GetCodeSegment() { return (void*)codeSegment.data(); }
	inline size_t GetCodeSegmentSize() { return sizeof(uint32_t) * codeSegment.size(); }

	inline size_t GetTotalSize() { return sizeof(ProgramHeader) + GetCodeSegmentSize() + GetDataSegmentSize(); }

private:
	ProgramHeader header;
	std::vector<uint32_t> codeSegment;
	std::vector<uint8_t> dataSegment;
	std::vector<std::pair<char[MAX_LABEL_SIZE], uint32_t>> labelToAddress;
};