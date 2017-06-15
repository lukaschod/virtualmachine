.label jumpHere
	.data helloMessage "Hello I'm process test3\n"
	ldc.i4 helloMessage
	call PrintLine
	//ldc.i4 helloMessage
	//int 1
	br jumpHere

halt

func ReadLine 1 0 // (char*)
	ldarg.0
	int 0
	end
	
func PrintLine 1 0 // (char*)
	ldarg.0
	int 1
	end