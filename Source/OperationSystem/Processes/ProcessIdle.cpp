#include <OperationSystem\Processes\ProcessIdle.h>
#include <OperationSystem\OperationSystem.h>
#include <OperationSystem\Resource.h>
#include <OperationSystem\Processes\ProcessStartStop.h>
#include <OperationSystem\Processes\ProcessManager.h>
#include <OperationSystem\Processes\ProcessProgram.h>
#include <VirtualMachine\CentralProcessingUnit.h>
#include <VirtualMachine\ExternalMemory.h>

ProcessIdle::ProcessIdle(ProcessStartStop* parent, OperationSystem* operationSystem) :
	ProcessSystem("ProcessIdle", parent, kProcessPriorityLow, operationSystem) { }

void ProcessIdle::Execute(CentralProcessingUnitCore* core)
{
	static int counter = 0;
	counter++;

	static uint32_t pathToFileSourceAddress;
	static uint32_t pathToFileAssemblyAddress;

	auto startStop = operationSystem->Get_startStopProcess();
	auto processManager = startStop->Get_processManager();
	auto programManager = startStop->Get_processProgramManager();

	if (counter == 5)
	{
		ResourceRequest memoryRequest;
		memoryRequest.count = 1;
		memoryRequest.requester = this;
		resourcePlanner->RequestResourceElement(operationSystem->Get_startStopProcess()->Get_resourceMemory(), memoryRequest);

		ExecuteWhenRunning([this](CentralProcessingUnitCore* core)
		{
			pathToFileSourceAddress = GetRequestedResourceElementReturn();
			const char* pathToSource = "console.asm";
			auto size = strlen(pathToSource) + 1;
			core->Get_ram()->WriteFromRealMemory(core, (void*) pathToSource, pathToFileSourceAddress, size);

			pathToFileAssemblyAddress = pathToFileSourceAddress + size;
			const char* pathToAssembly = "console.exe";
			core->Get_ram()->WriteFromRealMemory(core, (void*) pathToAssembly, pathToFileAssemblyAddress, size);
		});
	}

	if (counter == 20)
	{
		/*const char* source =
			// Create new program
			"DATA CreateProgramFromPath &myprogram.txt&\n"
			"LDC CreateProgramFromPath\n"
			"INT 13\n"

			"DATA FileHandle 0\n"
			"DATA FilePath &random.txt&\n"
			"DATA DataToWrite &Writing some stuff for fun...&\n"

			// Open file
			"LDC FilePath\n"
			"LDC 2\n"
			"INT 2\n"
			"STI FileHandle\n"

			// Write to file
			"LDI FileHandle\n"
			"LDC DataToWrite\n"
			"LDC 8\n"
			"INT 5\n"

			// Close file
			"LDI FileHandle\n"
			"INT 3\n"

			"HALT";

		auto program = new Program();
		assert(program->CreateFromText(source));

		auto programHandle = programManager->ProgramToHandle(program);
		*/
		programManager->CreateProgramFromSource(core, pathToFileSourceAddress, [this, programManager](CentralProcessingUnitCore* core)
		{
			auto error = GetRequestedResourceElementError();
			assert(error == kResourceRespondSuccess);

			auto programHanlde = GetRequestedResourceElementReturn();
			programManager->SaveProgramToFile(core, pathToFileAssemblyAddress, programHanlde, [this](CentralProcessingUnitCore* core)
			{
				auto error = GetRequestedResourceElementError();
				assert(error == kResourceRespondSuccess);
			});
		});
	}

	if (counter == 30)
	{
		processManager->CreateProcessUser(core, pathToFileAssemblyAddress, [this, processManager](CentralProcessingUnitCore* core)
		{
			auto error = GetRequestedResourceElementError();
			assert(error == kResourceRespondSuccess);
		});
	}

	if (counter == 60)
	{
		operationSystem->Get_startStopProcess()->StopOperationSystem(core, nullptr);
	}
}