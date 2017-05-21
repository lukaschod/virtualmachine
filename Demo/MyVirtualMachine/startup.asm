call Main
halt

func Main 0 0
	.data pathToConsoleSource "console.asm"
	.data pathToConsoleAssembly "console.exe"
	
	ldc.i4 pathToConsoleSource
	ldc.i4 pathToConsoleAssembly
	call CompileSource
	
	.data consoleName "Console"
	ldc.i4 pathToConsoleAssembly
	ldc.i4 consoleName
	call CreateProcess
	end
	
func CompileSource 2 1
	ldarg.0
	int 15
	stloc.0
	
	ldarg.1
	ldloc.0
	int 17
	end

func CreateProcess 2 0
	ldarg.0
	ldarg.1
	int 13
	end
