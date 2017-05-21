#pragma once

#include "Helper.h"
#include <vector>

class RealMachine;
class CentralProcessingUnitCore;

enum InstructionCode
{
	kInstructionCodeNOP,
	kInstructionCodeADD,
	kInstructionCodeSUB,
	kInstructionCodeMUL,
	kInstructionCodeDIV,
	kInstructionCodeAND,
	kInstructionCodeOR,
	kInstructionCodeCEQ,
	kInstructionCodeCNEQ,
	kInstructionCodeINC,
	kInstructionCodeDEC,

	kInstructionCodeLDC,

	kInstructionCodeLDA,
	kInstructionCodeSTA,

	kInstructionCodeINT,
	kInstructionCodeHALT,
	kInstructionCodeBR,
	kInstructionCodeBRTrue,
	kInstructionCodeBRFalse,
	kInstructionCodeEND,
	kInstructionCodeRET,
	kInstructionCodeCALL,
	kInstructionCodeSTACK,

	kInstructionCodeLDLOC,
	kInstructionCodeSTLOC,
	kInstructionCodeLDARG,
	kInstructionCodeSTARG,

	kInstructionCodeBREAK,
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

	kInteruptCodeAbort,

	kInteruptCodeCreateProcess,

	kInteruptCodeOSStart,

	kInteruptCreateProgramFromSource,
	kInteruptLoadProgramFromFile,
	kInteruptSaveProgramToFile,
	kInteruptDestroyProgram,

	kInteruptCodeOSEnd,

	kInteruptCodeReporRam,
};

#define MAX_LABEL_SIZE 30

struct ProgramHeader
{
	uint32_t codeSegmentSize;
	uint32_t dataSegmentSize;
	uint32_t stackSegmentSize;
};

struct Procedure
{
	char label[MAX_LABEL_SIZE];
	uint32_t localCount;
	uint32_t argumentCount;
	uint32_t address;
};

struct ProgramCompileError
{
	const char* message;
};

class Program
{
public:
	Program();

	bool CreateFromText(const char* source, size_t size);
	bool CompileFromMemoryAndCreate(CentralProcessingUnitCore* core, uint32_t address, uint32_t size);
	bool CreateFromMemory(CentralProcessingUnitCore* core, uint32_t address, uint32_t size);
	bool SaveToMemory(CentralProcessingUnitCore* core, uint32_t address);

private:
	bool CompileDataSegment(const char* source, size_t size);
	bool CompileCodeSegment(const char* source, size_t size);
	bool CompileArithmeticInstructions(const char*& source);
	bool CompileDataManipulationInstructions(const char*& source);
	bool CompileInteruptInstructions(const char*& source);

	bool MovePointerIfSame(const char*& source, const char* text, bool asSeperateWord = true);
	bool MovePointerIfReadedUint32(const char*& source, uint32_t& number);
	bool MovePointerIfReadedUint32OrLabel(const char*& source, uint32_t& number);
	bool MovePointerIfReadedStringSymbol(const char*& source);
	bool MovePointerIfReadedLabel(const char*& source, char* label);
	bool MovePointerIfComment(const char*& source);
	void MovePointerToEndOfLine(const char*& source);
	bool IsNextSeperator(const char*& source);
	void UpdateLabelAddress(char* label, uint32_t address);
	bool LabelToAddress(char* label, uint32_t& address);
	bool LabelToProcedure(char* label, Procedure& procedure);

	void ReportError(const char* error);

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
	std::vector<Procedure> procedures;
	std::vector<ProgramCompileError> errors;
};

class Compiler
{
public:
	Compiler();

	Program* CreateFromSource(CentralProcessingUnitCore* core, uint32_t address, uint32_t size);
};