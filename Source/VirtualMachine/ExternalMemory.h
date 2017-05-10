#pragma once

#include <stdint.h>
#include <vector>
#include <stdio.h>

class CentralProcessingUnit;
class MemoryManagmentUnit;
struct Registers;

enum FileAccessFlag
{
	kFileAccessNone = 0,
	kFileAccessReadBit = 1,
	kFileAccessWriteBit = 2,
};

struct FileHeader
{
	uint32_t fileHandle;
	bool isUsed;
	FileAccessFlag accessFlag;
	FILE* file;
};

class ExternalMemory
{
public:
	ExternalMemory(uint32_t maximumFileCount);

	uint32_t Open(CentralProcessingUnit* core, uint32_t filePathAddress, FileAccessFlag accessFlag);
	void Close(CentralProcessingUnit* core, uint32_t fileHandle);
	size_t Read(CentralProcessingUnit* core, uint32_t fileHandle, uint32_t address, size_t size = sizeof(uint32_t));
	size_t Write(CentralProcessingUnit* core, uint32_t fileHandle, uint32_t address, size_t size = sizeof(uint32_t));

private:
	FileHeader* TryResolveFileHandle(uint32_t fileHandle);
	FileHeader* TryOpenFirstAvailableFileHandle(const char* path, FileAccessFlag accessFlag);

private:
	std::vector<FileHeader> files;
};
