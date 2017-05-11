#include "ExternalMemory.h"
#include "MemoryManagmentUnit.h"
#include "CentralProcessingUnit.h"

ExternalMemory::ExternalMemory(uint32_t maximumFileCount)
{
	files.resize(maximumFileCount);
}

uint32_t ExternalMemory::Open(CentralProcessingUnit* core, uint32_t filePathAddress, FileAccessFlag accessFlag)
{
	auto memory = core->Get_memory();
	auto path = (char*) memory->AddressToPointerRange(core, filePathAddress, 128).pointer;
	if (path == nullptr || accessFlag == FileAccessFlag::kFileAccessNone)
		return 0;

	auto fileHeader = TryOpenFirstAvailableFileHandle(path, accessFlag);

	if (fileHeader != nullptr)
		return fileHeader->fileHandle;

	return 0;
}

FileHeader* ExternalMemory::TryOpenFirstAvailableFileHandle(const char* path, FileAccessFlag accessFlag)
{
	for (unsigned int i = 1; i < files.size(); i++)
	{
		auto header = files.data() + i;
		if (!header->isUsed)
		{
			header->isUsed = true;
			header->accessFlag = accessFlag;
			header->fileHandle = i;

			const char* mode = "";
			if (accessFlag == kFileAccessReadBit)
				mode = "r";
			if (accessFlag == kFileAccessWriteBit)
				mode = "w";
			if (fopen_s(&header->file, path, mode) != 0)
				return nullptr;

			return header;
		}
	}

	return nullptr;
}

void ExternalMemory::Close(CentralProcessingUnit* core, uint32_t fileHandle)
{
	auto fileHeader = TryResolveFileHandle(fileHandle);
	if (fileHeader == nullptr)
		return;

	fileHeader->isUsed = false;
	fclose(fileHeader->file);
}

size_t ExternalMemory::Read(CentralProcessingUnit* core, uint32_t fileHandle, uint32_t address, size_t size)
{
	auto memory = core->Get_memory();
	auto fileHeader = TryResolveFileHandle(fileHandle);
	if (fileHeader == nullptr || (fileHeader->accessFlag != kFileAccessReadBit))
		return 0;

	auto startSize = size;
	assert(size != 0);
	while (size != 0)
	{
		assert(size > 0);
		auto range = memory->AddressToPointerRange(core, address, size);

		auto copied = fread_s(range.pointer, range.size, sizeof(uint8_t), range.size, fileHeader->file);
		if (copied == 0)
			break;
		address += copied;
		size -= copied;
	}
	return startSize - size;
}

size_t ExternalMemory::Write(CentralProcessingUnit* core, uint32_t fileHandle, uint32_t address, size_t size)
{
	auto memory = core->Get_memory();
	auto fileHeader = TryResolveFileHandle(fileHandle);
	if (fileHeader == nullptr || (fileHeader->accessFlag != FileAccessFlag::kFileAccessWriteBit))
		return 0;

	auto startSize = size;
	assert(size != 0);
	while (size != 0)
	{
		assert(size > 0);
		auto range = memory->AddressToPointerRange(core, address, size);

		auto copied = fwrite(range.pointer, sizeof(uint8_t), range.size, fileHeader->file);
		if (copied == 0)
			break;
		address += copied;
		size -= copied;
	}
	return startSize - size;
}

FileHeader* ExternalMemory::TryResolveFileHandle(uint32_t fileHandle)
{
	if (files.size() <= fileHandle)
		return nullptr;

	auto header = files.data() + fileHandle;
	assert(header->file != nullptr);

	if (!header->isUsed)
		return nullptr;

	return header;
}