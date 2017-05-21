DATA PathToRandomSource &myRandomSource.asm&
DATA PathToRandomAssembly &myRandomSource.exe&
DATA RandomSource &HALT&
DATA RandomSourceHandle 0

LDC PathToRandomSource
LDC 2
INT 2
STI RandomSourceHandle

LDI RandomSourceHandle
LDC RandomSource
LDC 4
INT 5

LDI RandomSourceHandle
INT 3

DATA ProgramHandle 0
LDC PathToRandomSource
INT 15
STI ProgramHandle

LDC PathToRandomAssembly
LDI ProgramHandle
INT 17

// Create new program
LDC PathToRandomAssembly
INT 13

DATA FileHandle 0
DATA FilePath &random.txt&
DATA DataToWrite &Writing some stuff for fun...&

// Open file
LDC FilePath
LDC 2
INT 2
STI FileHandle

// Write to file
LDI FileHandle
LDC DataToWrite
LDC 8
INT 5

// Close file
LDI FileHandle
INT 3

HALT