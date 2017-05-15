#pragma once

#include <stdint.h>
#include <vector>
#include <stdio.h>

class CentralProcessingUnitCore;
class MemoryManagmentUnit;
struct Registers;

#define MAX_FILEPATH_SIZE 128

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
	ExternalMemory(const char* globalPath, uint32_t maximumFileCount);

	uint32_t Open(CentralProcessingUnitCore* core, uint32_t filePathAddress, FileAccessFlag accessFlag);
	void Close(CentralProcessingUnitCore* core, uint32_t fileHandle);
	size_t Read(CentralProcessingUnitCore* core, uint32_t fileHandle, uint32_t address, size_t size = sizeof(uint32_t));
	size_t Write(CentralProcessingUnitCore* core, uint32_t fileHandle, uint32_t address, size_t size = sizeof(uint32_t));

private:
	FileHeader* TryResolveFileHandle(uint32_t fileHandle);
	FileHeader* TryOpenFirstAvailableFileHandle(const char* path, FileAccessFlag accessFlag);

private:
	std::vector<FileHeader> files;
	char globalFilePath[MAX_FILEPATH_SIZE];
	size_t globalPathSize;
};
