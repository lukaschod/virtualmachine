call Main
halt

func Main 0 0
	.data helloWorld "Hello World!\n"
	ldc.i4 helloWorld
	call PrintLine
	
	.label Main_Repeat
		call HandleCommand
	br Main_Repeat

	end
	
func HandleCommand 0 3
	// ReadLine(message);
	.data message "000000000000000000000000000000000000000000000"
	ldc.i4 message
	call ReadLine
	
	// if (Strcmp("compile", message));
	.data compileCommand "compile"
	ldc.i4 compileCommand
	ldc.i4 message
	call StrCmp
	brfalse HandleCommand_NotCompile
		.data messageStartingCompilation "Starting compilation ...!\n"
		ldc.i4 messageStartingCompilation
		call PrintLine
	
		// loc0 = message
		ldc.i4 message
		stloc.0
		
		// loc1 = StrFindNext(loc0, ' ');
		ldloc.0
		ldc.i4 10
		call StrFindNext
		inc
		stloc.1
		
		// loc2 = StrFindNext(loc1, ' ');
		ldloc.1
		ldc.i4 10
		call StrFindNext
		inc
		stloc.2
		
		// CompileSource(loc1, loc2);
		ldloc.1
		ldloc.2
		call CompileSource
	.label HandleCommand_NotCompile
	
	// if (Strcmp("run", message));
	.data runCommand "run"
	ldc.i4 runCommand
	ldc.i4 message
	call StrCmp
	brfalse HandleCommand_NotRun
		// loc0 = message
		ldc.i4 message
		stloc.0
		
		// loc1 = StrFindNext(loc0, ' ');
		ldloc.0
		ldc.i4 10
		call StrFindNext
		inc
		stloc.1
		
		// CompileSource(loc1, runCommand);
		ldloc.1
		ldc.i4 runCommand
		call CreateProcess
	.label HandleCommand_NotRun
	
	end
	
func CompileSource 2 1  // (char*, char*)
	ldarg.0
	int 15
	stloc.0
	
	ldarg.1
	ldloc.0
	int 17
	end
	
func CreateProcess 2 0  // (char*, char*)
	ldarg.0
	ldarg.1
	int 13
	end
	
func PrintLine 1 0 // (char*)
	ldarg.0
	int 1
	end
	
func ReadLine 1 0 // (char*)
	ldarg.0
	int 0
	end
	
func StrFindNext 2 0 // (char*, char) -> char*
	.label StrFindNext_Repeat
		break
		
		ldarg.0
		lda.i1
		ldarg.1
		ceq
		brfalse StrFindNext_NotZero
			ldarg.0
			ret
		.label StrFindNext_NotZero
		
		ldarg.0
		inc
		starg.0
		br StrFindNext_Repeat
		

	ldc.i4 0
	ret
	
func StrCmp 2 2 // (char*, char*) -> bool
	// Store arguments to local
	ldarg.0
	stloc.0
	ldarg.1
	stloc.1
	
	.label StrCmp_Repeat
		// Check if zero
		ldloc.0
		lda.i1
		ldc.i4 0
		ceq
		brfalse StrCmp_NotZero
			ldc.i4 1
			ret
		.label StrCmp_NotZero
		
		// Check if space
		ldloc.0
		lda.i1
		ldc.i4 10
		ceq
		brfalse StrCmp_NotZero
			ldc.i4 1
			ret
		.label StrCmp_NotZero
		
		// Check if equal
		ldloc.0
		lda.i1
		ldloc.1
		lda.i1
		ceq
		brfalse StrCmp_NotZero
			ldloc.0
			inc
			stloc.0
			ldloc.1
			inc
			stloc.1
			br StrCmp_Repeat
		.label StrCmp_NotZero

	ldc.i4 0
	ret